#include "log/log.h"
#include "log/log_sinks.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"
#include "comms_drivers/comms_tx.h"
#include "timekeeping/timekeeping.h"
#include <stdio.h>

#define LOG_FILE_SYNC_INTERVAL 60000 // 60,000 is 1 minute
#define LOG_WRITE_BUFFER_MAX_SIZE 1024 * 2

//FIXME: filesystem is bricking.

extern UART_HandleTypeDef hlpuart1;
extern uint8_t LFS_is_lfs_mounted;

//FIXME: make sure the lfs buffers are actually big enough to write this many strings


// Cannot use LOG_message here, since it would overwrite the caller's message
// due to statically allocated log message buffers
// static int16_t append_to_current_log_file(char* log_write_buffer, uint16_t log_write_buffer_idx) {

//         LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_DEBUG, LOG_SINK_UMBILICAL_UART, //FIXME: remove all `LOG_message` calls after debuging.
//             "---------------------------- \nWriting to system log file: \n----------------------------");

//         //FIXME: I have no clue what the comment below means.
//         // We cannot use LFS_append_file due to recursion of the logging system
//         // lfs_file_t file;
//         // const int8_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, log_filename,
//         //     LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND, &LFS_file_cfg);
//         // if (open_result < 0){
//         //     // This error cannot be logged, except via UART or during an overpass of the ground station.
//         //     LOG_to_umbilical_uart("\nError opening system log file\n"); // FIXME(Issue #398): log to memory buffer
//         //     // LOG_to_uhf_radio("\nError opening system log file\n");
//         //     return -2;
//         // }
        

//         // Close the File, the storage is not updated until the file is closed successfully.
//         const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
//         if (close_result < 0) {
//             //FIXME: REMOVE LOG_message.
//             LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_SINK_UMBILICAL_UART, 
//                 "Error closing log file: %d", close_result); // FIXME(Issue #398): log to memory buffer
//             LOG_to_uhf_radio("\nError closing system log file\n");
//             return -4;
//         }
//         return 0;
// }

lfs_file_t current_log_file;
int64_t last_log_file_change_timestamp = 0;
extern int64_t last_log_file_sync_timestamp;

/// @param msg The message to be logged
void LOG_to_file(const char msg[])
{
    if (!LFS_is_lfs_mounted) {
        LOG_to_umbilical_uart("\nError writing to system log file: LFS not mounted\n"); // FIXME(Issue #398): log to memory buffer
        LOG_to_uhf_radio("\nError writing to system log file: LFS not mounted\n");
        return;
    }

    uint32_t is_time_to_change_log_file = (TIME_get_current_system_uptime_ms() - last_log_file_change_timestamp) > 60000; // 60,000 is 1 minute.
    if (is_time_to_change_log_file || last_log_file_change_timestamp == 0) {
        // Only close the file if it is opend.
        if (last_log_file_change_timestamp != 0) {
            lfs_file_close(&LFS_filesystem, &current_log_file);
        }

        last_log_file_change_timestamp = TIME_get_current_system_uptime_ms();
        last_log_file_sync_timestamp = TIME_get_current_system_uptime_ms();

        char log_filepath[30];
        char timestamp_str[16];
        TIME_get_current_utc_datetime_str(timestamp_str, sizeof(timestamp_str)); 
        snprintf(log_filepath, sizeof(log_filepath), "logs/%s.log", timestamp_str);

        const int16_t open_result = lfs_file_opencfg(&LFS_filesystem, &current_log_file, log_filepath, 
            LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND, &LFS_file_cfg);
        if (open_result < 0){
            LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                "Failed to open log file: %s", log_filepath);
        }
    }

    const lfs_ssize_t bytes_written = lfs_file_write(&LFS_filesystem, &current_log_file, msg, strlen(msg));
    if (bytes_written < 0) {
        LOG_to_umbilical_uart("\nError writing to system log file\n");// FIXME(Issue #398): log to memory buffer 
        LOG_to_uhf_radio("\nError writing to system log file\n");
    }
}

/// @brief Sends a log message to the umbilical UART
/// @param msg The message to be logged
/// @return void
void LOG_to_umbilical_uart(const char msg[]) 
{
    HAL_UART_Transmit(&hlpuart1, (uint8_t *)msg, strlen(msg),
                      LOG_UART_TRANSMIT_TIMEOUT);
    return;
}

/// @brief Sends a log message to the UHF radio
/// @param msg The message to be logged
/// @return void
void LOG_to_uhf_radio(const char msg[])
{
    const uint8_t result = COMMS_downlink_log_message(msg);
    if (result != 0) {
        LOG_to_umbilical_uart("\nError sending log message to UHF radio\n");
        // FIXME(Issue #398): log to memory buffer
    }
    return;
}

