#include <stdint.h>
#include <stdio.h>

#include "log/lazy_file_log_sink.h"
#include "log/log.h"
#include "timekeeping/timekeeping.h"

#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"

#include "main.h"


typedef struct {
    lfs_file_t file;
    uint64_t timestamp_of_last_sync;
    uint64_t timestamp_of_last_close;
    uint8_t is_open;
} LOG_file_context_struct_t;


// A context for the current log file.
static LOG_file_context_struct_t LOG_current_log_file_ctx;


/// @brief  Closed the current log file.
static int8_t LOG_close_current_log_file(void) {

    extern LOG_file_context_struct_t LOG_current_log_file_ctx; // from log_sinks.c

    if(!LOG_current_log_file_ctx.is_open) {
        return 0;
    }

    LFS_ensure_mounted();

    LOG_current_log_file_ctx.is_open = 0;
    LOG_current_log_file_ctx.timestamp_of_last_close = TIME_get_current_unix_epoch_time_ms();
    return lfs_file_close(&LFS_filesystem, &LOG_current_log_file_ctx.file);
}


/// @brief Opens a new timestamped log file, and sets it as the current log file.
static int8_t LOG_open_new_log_file_and_set_as_current(void) {

    extern LOG_file_context_struct_t LOG_current_log_file_ctx; // from log_sinks.c

    LFS_ensure_mounted();

    // Close the current log file if open.
    if (LOG_current_log_file_ctx.is_open) {
        int8_t result = LOG_close_current_log_file();
        if (result != 0) {return result;}
    }


    // Generate a timestamped filename.
    char timestamp_str[20];
    TIME_get_current_utc_datetime_str(timestamp_str, sizeof(timestamp_str)); 
    char filename[32];
    snprintf(filename, sizeof(filename), "logs/%s.log", timestamp_str);

    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_DEBUG, LOG_all_sinks_except(LOG_SINK_FILE), 
        " opening new log file: %s", filename
    );

    // Open the new log file.
    const int16_t result = lfs_file_opencfg(
        &LFS_filesystem, &LOG_current_log_file_ctx.file, filename, 
        LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND, &LFS_file_cfg
    );
    if (result != 0) {
        LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_FILE), 
            "failed to open new log file: %d", result);
        return result;
    }

    // Set metadata for the new log file.
    LOG_current_log_file_ctx.timestamp_of_last_sync = TIME_get_current_unix_epoch_time_ms();
    LOG_current_log_file_ctx.is_open = 1;
    return 0;
}



static int8_t LOG_ensure_current_log_file_is_open() {

    extern LOG_file_context_struct_t LOG_current_log_file_ctx; // from log_sinks.c

    if (!LOG_current_log_file_ctx.is_open) {
        return LOG_open_new_log_file_and_set_as_current();
    }
    return 0;
}


/// @brief Writes to the current log file.
static int8_t LOG_write_to_current_log_file(const char *msg, uint16_t msg_length) {
    LFS_ensure_mounted();
    LOG_ensure_current_log_file_is_open();

    return lfs_file_write(&LFS_filesystem, &LOG_current_log_file_ctx.file, msg, msg_length);
}



/// @brief Write a log message to an LFS log file. 
/// @param msg The message to be logged
/// @note LFS files are not updated until the file is closed or synced.
///     Syncing is done periodically in the rtos_background_upkeep task.
void LOG_to_file_lazy(const char filename[], const char msg[]) {
    LFS_ensure_mounted();

    LOG_ensure_current_log_file_is_open();

    LOG_write_to_current_log_file(msg, strlen(msg));
}



/// @brief Syncs the current log file.
int8_t LOG_sync_current_log_file(void) {

    extern LOG_file_context_struct_t LOG_current_log_file_ctx; // from log_sinks.c

    LFS_ensure_mounted();
    LOG_ensure_current_log_file_is_open();

    LOG_current_log_file_ctx.timestamp_of_last_sync = TIME_get_current_unix_epoch_time_ms();
    return lfs_file_sync(&LFS_filesystem, &LOG_current_log_file_ctx.file);
}


/// @brief Flush the current LittleFS log file(s), forcing a write to flash.
/// @note This function is meant to be called periodically in an RTOS task.
/// @note This function can be called very frequently. It tracks its own internal configuration.
void LOG_subtask_handle_sync_and_close_of_current_log_file() {
    // TODO: Make these global configs.
    const uint32_t LOG_FILE_SYNC_INTERVAL_MS = 15000; // 60,000 is 1 minute.
    const uint32_t LOG_FILE_CLOSE_INTERVAL_MS = 30000; // 300,0000 is 5 minutes

    LOG_ensure_current_log_file_is_open();

    const int64_t sync_interval_has_elapsed = (
        (TIME_get_current_unix_epoch_time_ms() - LOG_current_log_file_ctx.timestamp_of_last_sync) > LOG_FILE_SYNC_INTERVAL_MS
    );
    if (sync_interval_has_elapsed) {
        // TODO: benchmarking stuff to delete here.
        
        uint32_t start_time = HAL_GetTick();
        
        LOG_sync_current_log_file();

        LOG_message(LOG_SYSTEM_EPS, LOG_SEVERITY_DEBUG, LOG_all_sinks_except(LOG_SINK_FILE),
            "Log file synced in %ld ms", (HAL_GetTick() - start_time));
    }


    const int64_t close_interval_has_elapsed = (
        (TIME_get_current_unix_epoch_time_ms() - LOG_current_log_file_ctx.timestamp_of_last_close) > LOG_FILE_CLOSE_INTERVAL_MS
    );
    if (close_interval_has_elapsed) {
        // TODO: benchmarking stuff to delete here.

        uint32_t start_time = HAL_GetTick();

        LOG_close_current_log_file();

        LOG_message(LOG_SYSTEM_EPS, LOG_SEVERITY_DEBUG, LOG_all_sinks_except(LOG_SINK_FILE),
            "Log file closed in %ld ms", (HAL_GetTick() - start_time));

        LOG_open_new_log_file_and_set_as_current();
    }
}
