// Contract between the task spawner blob (../task_spawner_blob/) and the "task" blobs it starts,
// plus the helpers those blobs use inside their own loop.
//
// Task blobs are built and uplinked with the `.taskblob` extension (normal blobs use `.blob`). The
// spawner refuses to start anything else, and task blobs refuse to run as normal blobs.
//
// The spawner loads a task blob from the filesystem into a heap buffer that is never freed, and
// starts a FreeRTOS task at the blob's entry (offset 0), passing a blob_task_ctx_t. The task blob
// owns its loop: it runs, reports, and decides how long to sleep before the next run.
//
// A task blob's blob_main() tells its two possible callers apart by the magic in its first argument
// (exec_blob_from_fs() passes the args string there; FreeRTOS passes the task parameter), and refuses
// to run when called by exec_blob_from_fs():
//
//     __attribute__((used, section(".text.entry")))
//     uint8_t blob_main(const char *args_str, char *response_buf, unsigned short response_buf_len) {
//         blob_task_ctx_t *ctx = blob_task_ctx_from_arg(args_str);
//         if (ctx != NULL) {
//             // Started by the spawner. response_buf/response_buf_len are garbage here: unused.
//             blob_task_wait_for_predecessor(ctx);
//             for (;;) {
//                 char response[BLOB_TASK_RESPONSE_LEN];
//                 uint32_t next_run_delay_ms = 0;
//                 blob_task_begin_run(ctx);
//                 const uint8_t result = run_once(ctx->args, response, sizeof(response), &next_run_delay_ms);
//                 blob_task_end_run(ctx, result, response);
//                 if ((next_run_delay_ms == 0) || !blob_task_sleep_ms(ctx, next_run_delay_ms)) {
//                     blob_task_park(ctx); // Never returns.
//                 }
//             }
//         }
//         return blob_task_refuse_direct_run(BLOB_NAME, response_buf, response_buf_len);
//     }
//
// Rules for task blobs:
// 1. Never return from the task branch: a FreeRTOS task function must not return. To stop, call
//    blob_task_park(), which sleeps forever.
// 2. Sleep with blob_task_sleep_ms() (not vTaskDelay() directly) between runs, so a stop request
//    from the spawner is noticed within BLOB_TASK_POLL_MS.
// 3. Call blob_task_wait_for_predecessor() before the first run, so the previous task in the same
//    spawner slot has parked before this one touches the blob's persistent state.
//
// Why tasks park instead of exiting: the rc3 firmware doesn't link vTaskDelete() (nor
// osThreadExit()). So every task the spawner starts lives until reboot, and its stack (8 KB) and
// code buffer (~6 KB) are never freed. Each (re)start of a task blob leaks ~15 KB of the 128 KB
// FreeRTOS heap until the next reboot.
//
// Only ever APPEND fields to blob_task_ctx_t: blobs and spawners built at different times must
// agree on its layout.

#ifndef BLOB_LIBS_TASK_SPAWNER_H
#define BLOB_LIBS_TASK_SPAWNER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

extern int memcmp(const void *a, const void *b, size_t len);
extern int strlen(const char *s);
extern int snprintf(char *buf, unsigned int size, const char *fmt, ...);
extern uint64_t TIME_get_current_unix_epoch_time_ms();
extern volatile uint32_t TIME_uptime_ms_from_tim6;
extern uint8_t COMMS_downlink_tcmd_response(
    uint64_t ts_sent, uint8_t response_code, uint16_t duration_ms,
    char *response, uint32_t response_len
);

#define BLOB_TASK_FILE_EXT ".taskblob" // Required extension of task blob files.
#define BLOB_TASK_ERR_RUN_DIRECTLY 249 // A task blob was run with exec_blob_from_fs(); refused.
#define BLOB_TASK_CTX_MAGIC "BLOBTASK" // 8 chars, compared bytewise (args strings aren't aligned).
#define BLOB_TASK_CTX_MAGIC_LEN 8
#define BLOB_TASK_PATH_MAX 64 // Includes the null terminator.
#define BLOB_TASK_ARGS_MAX 200 // Includes the null terminator. Telecommand args are at most 240.
#define BLOB_TASK_RESPONSE_LEN 400 // Suggested per-run response buffer size.
#define BLOB_TASK_POLL_MS 1000u // How often a sleeping task checks for a stop request.

typedef struct blob_task_ctx {
    char magic[BLOB_TASK_CTX_MAGIC_LEN]; // BLOB_TASK_CTX_MAGIC while live (this boot). No terminator.
    struct blob_task_ctx *self; // == this block's own address; a second check against stale data.
    volatile uint8_t stop_requested; // Set by the spawner: park at the next sleep.
    volatile uint8_t parked; // Set by the task once parked (it never touches anything again).
    struct blob_task_ctx *predecessor; // Previous task in the same slot, or NULL.
    uint8_t *code; // The blob's code buffer, which the task executes from.
    TaskHandle_t task;
    uint32_t run_count;
    uint32_t last_run_uptime_ms; // Uptime at the start of the latest run.
    uint64_t last_run_ts_ms; // Unix time at the start of the latest run.
    uint8_t last_result;
    char path[BLOB_TASK_PATH_MAX];
    char args[BLOB_TASK_ARGS_MAX];
} blob_task_ctx_t;

/// @brief The spawner's context, if `arg0` is one; NULL if it's an args string from
///     exec_blob_from_fs() (which is always at least 8 readable bytes: a telecommand args buffer).
static inline blob_task_ctx_t *blob_task_ctx_from_arg(const void *arg0) {
    blob_task_ctx_t *ctx = (blob_task_ctx_t *)arg0;
    if (memcmp(arg0, BLOB_TASK_CTX_MAGIC, BLOB_TASK_CTX_MAGIC_LEN) != 0) {
        return NULL;
    }
    return (ctx->self == ctx) ? ctx : NULL;
}

static inline void blob_task_wait_for_predecessor(blob_task_ctx_t *ctx) {
    while ((ctx->predecessor != NULL) && !ctx->predecessor->parked) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/// @brief Sleep, in slices, until `ms` have passed or the spawner asks this task to stop.
/// @return true if the full time passed; false if a stop was requested (call blob_task_park()).
static inline bool blob_task_sleep_ms(blob_task_ctx_t *ctx, uint32_t ms) {
    while (ms > 0) {
        if (ctx->stop_requested) {
            return false;
        }
        const uint32_t step_ms = (ms < BLOB_TASK_POLL_MS) ? ms : BLOB_TASK_POLL_MS;
        vTaskDelay(pdMS_TO_TICKS(step_ms));
        ms -= step_ms;
    }
    return !ctx->stop_requested;
}

/// @brief Park this task forever. Never returns.
__attribute__((noreturn))
static inline void blob_task_park(blob_task_ctx_t *ctx) {
    ctx->parked = 1;
    for (;;) {
        vTaskDelay(portMAX_DELAY);
    }
}

/// @brief Call right before each run (records the start time for the response and `status`).
static inline void blob_task_begin_run(blob_task_ctx_t *ctx) {
    ctx->last_run_ts_ms = TIME_get_current_unix_epoch_time_ms();
    ctx->last_run_uptime_ms = TIME_uptime_ms_from_tim6;
}

/// @brief Refuse to run a task blob as a normal blob (via exec_blob_from_fs()): its loop would
///     block the telecommand executor forever, and a single run would leave things (e.g., the GNSS
///     power) with nothing managing them afterwards.
/// @return BLOB_TASK_ERR_RUN_DIRECTLY, for blob_main() to return.
static inline uint8_t blob_task_refuse_direct_run(
    const char *blob_name, char *response_buf, unsigned short response_buf_len
) {
    snprintf(
        response_buf, response_buf_len,
        "%s error: this is a task blob (%s); start it with the task spawner blob instead",
        blob_name, BLOB_TASK_FILE_EXT
    );
    return BLOB_TASK_ERR_RUN_DIRECTLY;
}

/// @brief Call right after each run: downlinks the run's response as a telecommand response
///     (exactly like a self-rescheduled blob's response was), and records the result for `status`.
static inline void blob_task_end_run(blob_task_ctx_t *ctx, uint8_t result, char *response) {
    ctx->run_count++;
    ctx->last_result = result;
    COMMS_downlink_tcmd_response(
        ctx->last_run_ts_ms, result, (uint16_t)(TIME_uptime_ms_from_tim6 - ctx->last_run_uptime_ms),
        response, strlen(response) + 1
    );
}

#endif // BLOB_LIBS_TASK_SPAWNER_H
