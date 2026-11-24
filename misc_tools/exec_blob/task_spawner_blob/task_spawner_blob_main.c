// This is a blob (executable) that starts "task blobs" as their own long-running FreeRTOS tasks,
// instead of each blob re-scheduling itself through the telecommand agenda. Each task blob owns
// its own loop and timing (see ../blob_libs/task_spawner.h for the contract).
//
// Args Format (one of):
//   <slot>;<task_blob_path>[;<task_blob_args>]
//       Load the task blob (must end in `.taskblob`) from the filesystem and start it as a task in slot <slot> (0..7).
//       Everything after the path's ';' is passed to the blob verbatim as its args (may contain ';').
//       Whatever task was in the slot is asked to stop first; the new task waits for it to park.
//   <slot>;stop      Ask the slot's task to stop (it parks at its next sleep).
//   stop_all         Same, for every slot.
//   status           Report every slot: blob, args, run count, last run age, last result.
//
// Usage Example:
//   CTS1+exec_blob_from_fs(blobs/task_spawner_v1.blob,0,0;blobs/gnss_bestxyzb_ring_v3.taskblob;9000;5)!
//   CTS1+exec_blob_from_fs(blobs/task_spawner_v1.blob,0,1;blobs/extended_beacon_v6.taskblob;9000;5)!
//   CTS1+exec_blob_from_fs(blobs/task_spawner_v1.blob,0,status)!
//   CTS1+exec_blob_from_fs(blobs/task_spawner_v1.blob,0,1;stop)!
//
// Notes:
//  1. Always use "0" as the second argument (i.e., always run with malloc).
//  2. EVERY start leaks ~15 KB of the 128 KB FreeRTOS heap until reboot (the firmware can't delete
//     tasks, so a stopped task parks forever, keeping its stack and code). Don't re-send start
//     commands every pass; check `status` first.
//  3. Agenda commands (agenda_delete_all, etc.) do NOT stop a task. Use `stop_all`, or reboot.
//  4. A reboot stops everything; nothing is restored after a reboot.
//  5. The same blob path can't run in two slots at once, since blobs keep persistent state at
//     fixed addresses.
//  6. The firmware's scheduler is cooperative: a task blob's run blocks every other task until it
//     yields, exactly as it blocked the telecommand executor before. Keep runs well under the 16s
//     STM32 watchdog.
//
// ------ Implementation Notes ----------------
//
// The task blob is loaded into a heap buffer (the same way exec_blob_from_fs() does) that is never
// freed, and xTaskCreate() starts it at offset 0 (blob_main()), with a blob_task_ctx_t as the
// parameter. blob_main() recognises the context by its magic and runs its loop.
//
// The pointer to each slot's context lives in fixed, persistent SRAM (BLOB_TASKS in
// ../blobs_persistent_sram_regions.ld). After a reboot, it still points into the FreeRTOS heap
// (`ucHeap`, in .bss, so zeroed on boot), where the context's magic no longer reads back, so the
// slot is treated as empty -- same idea as the task blobs' heap canaries.

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../blob_libs/task_spawner.h"

extern void *memset(void *dest, int value, size_t len);
extern void *memcpy(void *dest, const void *src, size_t len);
extern int snprintf(char *buf, unsigned int size, const char *fmt, ...);
extern int strcmp(const char *s1, const char *s2);
extern int strncmp(const char *s1, const char *s2, size_t n);
extern void *pvPortMalloc(size_t xWantedSize);
extern void vPortFree(void *pv);
extern int32_t LFS_file_size(const char file_name[], uint8_t enable_log_messages);
extern int32_t LFS_read_file(const char file_name[], int32_t offset, uint8_t *read_buffer, uint32_t read_buffer_size);

// Defined by blob.ld.
extern uint8_t blob_task_slots_base[];

static const char BLOB_NAME[] = "task_spawner_v1";

#define SPAWNER_SLOT_COUNT 8 // BLOB_TASKS's LENGTH (0x40) / sizeof(spawner_slot_t).
#define SPAWNER_SLOT_MAGIC 0xB10B5107u
#define SPAWNER_STACK_BYTES 8192 // Same as TASK_execute_telecommands, which used to run these blobs.
#define SPAWNER_TASK_PRIORITY 36 // osPriorityAboveNormal4, same as TASK_execute_telecommands.

typedef struct {
    uint32_t magic; // SPAWNER_SLOT_MAGIC once written; anything else means cold/garbage SRAM.
    blob_task_ctx_t *ctx;
} spawner_slot_t;

_Static_assert(sizeof(spawner_slot_t) * SPAWNER_SLOT_COUNT == 0x40, "BLOB_TASKS region size");

#define g_slots ((spawner_slot_t *)blob_task_slots_base)


/// @brief The slot's live context, or NULL if the slot has had no task this boot.
static blob_task_ctx_t *get_live_ctx(uint8_t slot_idx) {
    if (g_slots[slot_idx].magic != SPAWNER_SLOT_MAGIC) {
        return NULL; // Cold/garbage SRAM: don't dereference the pointer.
    }
    blob_task_ctx_t *ctx = g_slots[slot_idx].ctx;
    if ((ctx == NULL) || (blob_task_ctx_from_arg(ctx) == NULL)) {
        return NULL; // Rebooted since (the heap was wiped).
    }
    return ctx;
}

/// @brief Is the context's task running (or about to), i.e., not stopping or parked?
static bool is_active(const blob_task_ctx_t *ctx) {
    return (ctx != NULL) && !ctx->stop_requested && !ctx->parked;
}

static void stop_slot(uint8_t slot_idx) {
    blob_task_ctx_t *ctx = get_live_ctx(slot_idx);
    if (ctx != NULL) {
        ctx->stop_requested = 1;
    }
}

/// @brief Load a task blob from the filesystem into a new heap buffer, the same way
///     exec_blob_from_fs() does.
/// @return The buffer, or NULL (with the reason in response_buf).
static uint8_t *load_blob(const char *path, char *response_buf, unsigned short response_buf_len) {
    const int32_t file_size = LFS_file_size(path, 1);
    if (file_size <= 0) {
        snprintf(response_buf, response_buf_len, "%s error: LFS_file_size(%s) -> %ld", BLOB_NAME, path, file_size);
        return NULL;
    }

    const uint32_t buf_size = (uint32_t)file_size + 20; // Same slack as exec_blob_from_fs().
    uint8_t *buf = (uint8_t *)pvPortMalloc(buf_size); // portBYTE_ALIGNMENT=8, fine for Thumb code.
    if (buf == NULL) {
        snprintf(response_buf, response_buf_len, "%s error: pvPortMalloc(%lu) failed", BLOB_NAME, buf_size);
        return NULL;
    }
    memset(buf, 0, buf_size);

    const int32_t read_len = LFS_read_file(path, 0, buf, buf_size);
    if (read_len != file_size) {
        vPortFree(buf);
        snprintf(response_buf, response_buf_len, "%s error: LFS_read_file(%s) -> %ld", BLOB_NAME, path, read_len);
        return NULL;
    }

    __asm volatile ("dsb\n isb" ::: "memory"); // Flush before executing from the new buffer.
    return buf;
}

static uint8_t cmd_start(
    uint8_t slot_idx, const char *path, const char *blob_args,
    char *response_buf, unsigned short response_buf_len
) {
    // Blobs keep persistent state at fixed addresses: two slots must never run the same one.
    for (uint8_t i = 0; i < SPAWNER_SLOT_COUNT; i++) {
        const blob_task_ctx_t *other = get_live_ctx(i);
        if ((i != slot_idx) && is_active(other) && (strcmp(other->path, path) == 0)) {
            snprintf(response_buf, response_buf_len, "%s error: %s already runs in slot %d", BLOB_NAME, path, i);
            return 4;
        }
    }

    uint8_t *code = load_blob(path, response_buf, response_buf_len);
    if (code == NULL) {
        return 5;
    }
    blob_task_ctx_t *ctx = (blob_task_ctx_t *)pvPortMalloc(sizeof(blob_task_ctx_t));
    if (ctx == NULL) {
        vPortFree(code);
        snprintf(response_buf, response_buf_len, "%s error: pvPortMalloc failed (ctx)", BLOB_NAME);
        return 6;
    }

    blob_task_ctx_t *old = get_live_ctx(slot_idx);
    memset(ctx, 0, sizeof(blob_task_ctx_t));
    memcpy(ctx->magic, BLOB_TASK_CTX_MAGIC, BLOB_TASK_CTX_MAGIC_LEN);
    ctx->self = ctx;
    ctx->predecessor = old;
    ctx->code = code;
    snprintf(ctx->path, sizeof(ctx->path), "%s", path);
    snprintf(ctx->args, sizeof(ctx->args), "%s", blob_args);

    // The blob's entry (blob_main) is at offset 0. +1 for Thumb mode.
    const TaskFunction_t entry = (TaskFunction_t)((uintptr_t)code | 1u);
    char task_name[24];
    snprintf(task_name, sizeof(task_name), "blob_slot_%d", slot_idx);
    if (xTaskCreate(
        entry, task_name, SPAWNER_STACK_BYTES / sizeof(StackType_t), ctx,
        SPAWNER_TASK_PRIORITY, &ctx->task
    ) != pdPASS) {
        vPortFree(code);
        vPortFree(ctx);
        snprintf(response_buf, response_buf_len, "%s error: xTaskCreate failed", BLOB_NAME);
        return 7;
    }

    // Only now that the new task exists: the old one keeps running if anything above failed.
    // (The new task can't run before this, as blob_main() doesn't yield.)
    if (old != NULL) {
        old->stop_requested = 1;
    }
    g_slots[slot_idx].ctx = ctx;
    g_slots[slot_idx].magic = SPAWNER_SLOT_MAGIC;

    snprintf(
        response_buf, response_buf_len, "%s: slot %d: started %s(%s)%s",
        BLOB_NAME, slot_idx, path, ctx->args, (old != NULL) ? ", stopping previous task" : ""
    );
    return 0;
}

static uint8_t cmd_status(char *response_buf, unsigned short response_buf_len) {
    uint16_t len = (uint16_t)snprintf(response_buf, response_buf_len, "%s:", BLOB_NAME);
    for (uint8_t i = 0; (i < SPAWNER_SLOT_COUNT) && (len < response_buf_len); i++) {
        const blob_task_ctx_t *ctx = get_live_ctx(i);
        if (ctx == NULL) {
            continue;
        }
        len += (uint16_t)snprintf(
            response_buf + len, response_buf_len - len,
            " [s%d: %s%s(%s), runs=%lu, last=%lus ago, res=%d]",
            i, ctx->parked ? "parked " : (ctx->stop_requested ? "stopping " : ""),
            ctx->path, ctx->args, ctx->run_count,
            (TIME_uptime_ms_from_tim6 - ctx->last_run_uptime_ms) / 1000u, ctx->last_result
        );
    }
    return 0;
}

/// @brief Parse the slot number at the start of args_str, up to ';' or the end.
/// @return Pointer just past the ';' (or at the end), or NULL if invalid.
static const char *parse_slot(const char *s, uint8_t *slot_dest) {
    if ((s[0] < '0') || (s[0] > '9') || ((s[1] != ';') && (s[1] != '\0'))) {
        return NULL; // Exactly one digit.
    }
    *slot_dest = (uint8_t)(s[0] - '0');
    return (s[1] == ';') ? (s + 2) : (s + 1);
}


__attribute__((used, section(".text.entry")))
uint8_t blob_main(
    const char *args_str,
    char *response_buf, unsigned short response_buf_len
) {
    if (strcmp(args_str, "status") == 0) {
        return cmd_status(response_buf, response_buf_len);
    }
    if (strcmp(args_str, "stop_all") == 0) {
        for (uint8_t i = 0; i < SPAWNER_SLOT_COUNT; i++) {
            stop_slot(i);
        }
        snprintf(response_buf, response_buf_len, "%s: all slots stopping", BLOB_NAME);
        return 0;
    }

    uint8_t slot_idx = 0;
    const char *rest = parse_slot(args_str, &slot_idx);
    if ((rest == NULL) || (slot_idx >= SPAWNER_SLOT_COUNT)) {
        snprintf(response_buf, response_buf_len, "%s error: bad slot (0..%d)", BLOB_NAME, SPAWNER_SLOT_COUNT - 1);
        return 1;
    }

    if (strcmp(rest, "stop") == 0) {
        stop_slot(slot_idx);
        snprintf(response_buf, response_buf_len, "%s: slot %d stopping", BLOB_NAME, slot_idx);
        return 0;
    }

    // Path runs up to the next ';' (or the end). Everything after it is the blob's args.
    char path[BLOB_TASK_PATH_MAX];
    uint16_t path_len = 0;
    while ((rest[path_len] != '\0') && (rest[path_len] != ';')) {
        if (path_len >= (sizeof(path) - 1)) {
            snprintf(response_buf, response_buf_len, "%s error: path too long", BLOB_NAME);
            return 3;
        }
        path[path_len] = rest[path_len];
        path_len++;
    }
    path[path_len] = '\0';
    if (path_len == 0) {
        snprintf(response_buf, response_buf_len, "%s error: missing path", BLOB_NAME);
        return 2;
    }

    // Only task blobs: a normal blob started as a task would return from its task function.
    const uint16_t ext_len = (uint16_t)strlen(BLOB_TASK_FILE_EXT);
    if ((path_len <= ext_len) || (strncmp(path + path_len - ext_len, BLOB_TASK_FILE_EXT, ext_len) != 0)) {
        snprintf(response_buf, response_buf_len, "%s error: not a %s file: %s", BLOB_NAME, BLOB_TASK_FILE_EXT, path);
        return 8;
    }
    // Telecommand args (<= 240 chars) minus slot and path always fit in BLOB_TASK_ARGS_MAX.
    const char *blob_args = (rest[path_len] == ';') ? (rest + path_len + 1) : "";

    return cmd_start(slot_idx, path, blob_args, response_buf, response_buf_len);
}
