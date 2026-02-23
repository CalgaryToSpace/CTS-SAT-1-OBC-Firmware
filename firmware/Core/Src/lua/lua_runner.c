#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "FreeRTOS.h"
#include "log/log.h"
#include <string.h>
#include <stdlib.h>

#include "lua/lua_bindings.h"

/*
// Simple implementation of lua_alloc function.
// Reference: 'https://www.lua.org/manual/5.1/manual.html' search for 'lua_Alloc'
// Does not track memory usage.
static void *l_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
    if (nsize == 0) {
        free(ptr);
        return NULL;
    }
    else {
        return realloc(ptr, nsize);
    }
}
*/

/*
container_of - Cast a member of a structure out to the containing structure.
This is how it is done in the linux kernel.
see: https://github.com/torvalds/linux/blob/master/include/linux/container_of.h
*/
#define container_of(ptr, type, member) ({                      \
    const __typeof__(((type *)0)->member) *__mptr = (ptr);      \
    (type *)((char *)__mptr - offsetof(type, member));          \
})

// Struct representing a allocated block of memory.
typedef struct {
    size_t size;
    uint8_t data[];
} block_t;

// Used to track lua memory usage.
typedef struct { 
    int32_t in_use;
    int32_t limit;
    int32_t min;
    int32_t max; 
    int32_t blocks_currently_allocated;
    int32_t total_blocks_allocated;
} lua_mem_usage_t;
// An implementation of lua_Alloc that tracks memory usage by keeping a header in front of the allocated buffer.
// This is required since osize is not always reliable and thus we cannot use it to track memory usage.
// verified empirically.
static void *l_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
    lua_mem_usage_t *usage = (lua_mem_usage_t*)ud;

    // if freeing.
    if (nsize == 0) {
        if (ptr) {
            // block_t *block = (block_t *)ptr - 1; 
            block_t *block = container_of(ptr, block_t, data);
            usage->in_use -= block->size;
            usage->blocks_currently_allocated--;
            free(block);
        }

        if (usage->in_use < usage->min) {
            usage->min = usage->in_use;
        }

        return NULL;
    }

    // if a new allocation.
    if (ptr == NULL) {
        block_t *block = malloc(sizeof(block_t) + nsize);
        if (block == NULL) return NULL; // Failed to allocate.

        block->size = nsize;

        usage->in_use += nsize;
        usage->blocks_currently_allocated++;
        usage->total_blocks_allocated++;

        return block->data;
    } 
    // else if realloc.
    else {
        // block_t *h = (block_t *)ptr - 1;
        block_t *block = container_of(ptr, block_t, data);
        size_t old_size = block->size;

        block_t *new_block = realloc(block, sizeof(block_t) + nsize);
        if (new_block == NULL) return NULL;

        new_block->size = nsize;

        usage->in_use += nsize - old_size;
        return new_block->data;
    }
}

/// @brief Initialize a lua state.
/// @param usage a pointer to a struct that tracks lua memory usage, and limits it.
/// @return An initialized lua state.
lua_State* LUA_init(lua_mem_usage_t *usage) {
    lua_State *L = lua_newstate(l_alloc, usage);

    // TODO: Include what we need/want.
    // Open standard libraries
    luaL_requiref(L, "string", luaopen_string, 1);
    lua_pop(L, 1); // String library is in global table, so remove it from the stack.

    // Register our custom functions
    // print(str:str)
    lua_register(L, "print", l_print);
    // lfs_read_file(path:str, offset:int, len:int)
    lua_register(L, "lfs_read_file", l_LFS_read_file);
    // lfs_write_file(path:str, str:str) 
    lua_register(L, "lfs_write_file", l_LFS_write_file);
    return L;
}

void LUA_run_script(const char *script) {
    lua_mem_usage_t usage = {.in_use=0, .limit=10000, .min=0, .max = 0};
    lua_State *L = LUA_init(&usage);

    if (luaL_dostring(L, script) != LUA_OK){
        // If an error occurs, pop the error message off the stack and log.
        const char *msg = lua_tostring(L, -1);
        LOG_message(
            LOG_SYSTEM_OBC, LOG_SEVERITY_ERROR,
            LOG_all_sinks_except(LOG_SINK_FILE),
            "Error occurred while running lua script: %s", msg
        );
        
    }

    lua_close(L);

    LOG_message(
        LOG_SYSTEM_OBC, LOG_SEVERITY_DEBUG,
        LOG_all_sinks_except(LOG_SINK_FILE),
        "lua script ran successfully! in_use: %ld, min: %ld, max: %ld, blocks_currently_allocated: %ld, total_blocks_allocated: %ld", 
        usage.in_use, usage.min, usage.max, usage.blocks_currently_allocated, usage.total_blocks_allocated
    );
}


void LUA_test() {
    static const char *script = 
        "print('testing lfs bindings')\n"
        "lfs_write_file('./hello.txt', 'hello world from lua file!')\n"
        "text = lfs_read_file('./hello.txt', 0, 100)\n"
        "print(text)\n";

    LUA_run_script(script);
}

/// @brief attempts to write a string to a file, then attempts to read it back.
void LUA_test_lfs_read_write_bindings() {
    static const char *script = 
        "print('testing lfs bindings')\n"
        "lfs_write_file('./hello.txt', 'hello world from lua file!')\n"
        "text = lfs_read_file('./hello.txt', 0, 100)\n"
        "print(text)\n";

    LUA_run_script(script);
}

/// @brief attempts to overflow the heap via repeated string allocations.
void LUA_test_out_of_memory() {
    static const char *lua_oom_test_script =
        "print(\"OOM test starting...\")\n"
        "\n"
        "local big = {}\n"
        "local i = 1\n"
        "\n"
        "while true do\n"
        "    big[i] = string.rep(\"X\", 1024 * 1024) -- 1MB chunks\n"
        "    i = i + 1\n"
        "end\n";

    LUA_run_script(lua_oom_test_script);
}

