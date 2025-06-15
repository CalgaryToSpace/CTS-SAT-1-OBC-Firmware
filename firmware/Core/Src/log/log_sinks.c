#include "log/log.h"
#include "log/log_sinks.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"
#include "comms_drivers/comms_tx.h"
#include "timekeeping/timekeeping.h"
#include <stdio.h>

#define LOG_FILE_SYNC_INTERVAL 60000 // 60,000 is 1 minute
#define LOG_WRITE_BUFFER_MAX_SIZE 2000


extern UART_HandleTypeDef hlpuart1;
extern uint8_t LFS_is_lfs_mounted;

//FIXME: make sure the lfs buffers are actually big enough to write this many strings
char log_write_buffer[LOG_WRITE_BUFFER_MAX_SIZE];
uint16_t log_write_buffer_idx = 0;
uint64_t timestamp_of_last_sync;


// Cannot use LOG_message here, since it would overwrite the caller's message
// due to statically allocated log message buffers
static int16_t append_to_current_log_file(char* log_write_buffer, uint16_t log_write_buffer_idx) {
        if (!LFS_is_lfs_mounted) {
        LOG_to_umbilical_uart("\nError writing to system log file: LFS not mounted\n"); // FIXME(Issue #398): log to memory buffer
        LOG_to_uhf_radio("\nError writing to system log file: LFS not mounted\n");
        return -1;
        }

        char log_filename[30];
        char timestamp_str[17];
        TIME_get_current_utc_datetime_str(timestamp_str, sizeof(timestamp_str)); //FIXME: this is always returns the same string.
        snprintf(log_filename, sizeof(log_filename), "logs/%s.log", timestamp_str);
        LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_NORMAL, LOG_SINK_UMBILICAL_UART, //FIXME: remove all `LOG_message` calls after debuging.
            "---------------------------- \nWriting to system log file: %s \n----------------------------", log_filename);

        //FIXME: I have no clue what the comment below means.
        // We cannot use LFS_append_file due to recursion of the logging system
        lfs_file_t file;
        const int8_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, log_filename,
            LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND, &LFS_file_cfg);
        if (open_result < 0){
            // This error cannot be logged, except via UART or during an overpass of the ground station.
            LOG_to_umbilical_uart("\nError opening system log file\n"); // FIXME(Issue #398): log to memory buffer
            // LOG_to_uhf_radio("\nError opening system log file\n");
            return -2;
        }
        
        const lfs_ssize_t bytes_written = lfs_file_write(&LFS_filesystem, &file, log_write_buffer, log_write_buffer_idx);
        if (bytes_written < 0) {
            LOG_to_umbilical_uart("\nError writing to system log file\n");// FIXME(Issue #398): log to memory buffer 
            LOG_to_uhf_radio("\nError writing to system log file\n");
            return -3;
        }

        // Close the File, the storage is not updated until the file is closed successfully.
        const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
        if (close_result < 0) {
            //FIXME: REMOVE LOG_message.
            LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_SINK_UMBILICAL_UART, 
                "Error closing log file: %d", close_result); // FIXME(Issue #398): log to memory buffer
            LOG_to_uhf_radio("\nError closing system log file\n");
            return -4;
        }
        return 0;
}

/// @brief Write the message to a staging buffer. Once the buffer is full or the sync interval has elapsed, write the buffer to a timestamped log file.
/// @param msg The message to be logged
void LOG_to_file(const char msg[])
{
    int8_t sync_interval_has_elapsed = (TIME_get_current_unix_epoch_time_ms() - timestamp_of_last_sync) > LOG_FILE_SYNC_INTERVAL;
    int8_t log_write_buffer_is_full = (log_write_buffer_idx + strlen(msg) + 1) >= LOG_WRITE_BUFFER_MAX_SIZE; 

    if (sync_interval_has_elapsed || log_write_buffer_is_full) {
        append_to_current_log_file(log_write_buffer, log_write_buffer_idx);
        log_write_buffer_idx = 0;
    }

    /*add message to buffer. log_write_buffer is a effectively a multiline string. Which we conntiuously append to.
     This allows us to write all messages with a single write call, while still tightly packing messages in the staging buffer.*/
    snprintf(&log_write_buffer[log_write_buffer_idx], LOG_WRITE_BUFFER_MAX_SIZE - log_write_buffer_idx, "%s", msg);
    log_write_buffer_idx += strlen(msg);

    timestamp_of_last_sync = TIME_get_current_unix_epoch_time_ms();
	return;
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

