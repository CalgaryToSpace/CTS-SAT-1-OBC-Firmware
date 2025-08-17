#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "FreeRTOS.h"
#include "log/log.h"
#include <string.h>

#include "lua/lua_bindings.h"



// TODO: not used yet. Some more thought is needed with respect to
//       memory management.
// TODO: implement a memory analyzer to evaluate memory usage.
typedef struct { size_t in_use, limit; } lua_mem_limit_t;

static void *rtos_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
  (void)osize;
  lua_mem_limit_t *lim = (lua_mem_limit_t*)ud;
  if (nsize == 0) { vPortFree(ptr); return NULL; }

  void *np = pvPortMalloc(nsize);
  if (!np) return NULL;

  if (ptr) {
    // best-effort realloc
    size_t copy = nsize; // unknown old size; copy min if you track it elsewhere
    memcpy(np, ptr, copy);
    vPortFree(ptr);
  }
  if (lim) {
    lim->in_use += nsize;
    if (lim->limit && lim->in_use > lim->limit) { vPortFree(np); return NULL; }
  }
  return np;
}

lua_State* LUA_init() {
    // Create the lua state
    lua_State *L = lua_newstate(rtos_alloc, NULL);
    // Open standard libraries
    //TODO: only include what we need/want.
    // luaL_openlibs(L);

    // Register our custom functions
    lua_register(L, "print", l_print);
    return L;
}

void LUA_test() {
    lua_State *L = LUA_init();

    if (luaL_dostring(L, "print(1+1.5)") != LUA_OK){
        LOG_message(
            LOG_SYSTEM_OBC, LOG_SEVERITY_ERROR,
            LOG_all_sinks_except(LOG_SINK_FILE),
            "lua error" 
        );
    }
    LOG_message(
        LOG_SYSTEM_OBC, LOG_SEVERITY_DEBUG,
        LOG_all_sinks_except(LOG_SINK_FILE),
        "lua Success!" 
    );
    lua_close(L);
}

