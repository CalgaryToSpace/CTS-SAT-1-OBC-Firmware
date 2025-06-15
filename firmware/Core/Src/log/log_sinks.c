#include "log/log.h"
#include "log/log_sinks.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"
#include "comms_drivers/comms_tx.h"
#include "timekeeping/timekeeping.h"
#include <stdio.h>

#define LOG_FILE_SYNC_INTERVAL 60000 // 1 minute
#define LOG_STRING_MAX_LENGTH 250
#define LOG_BUFFER_MAX_STRINGS 10


extern UART_HandleTypeDef hlpuart1;
extern uint8_t LFS_is_lfs_mounted;

char log_write_buffer[LOG_BUFFER_MAX_STRINGS][LOG_STRING_MAX_LENGTH];
uint16_t log_write_buffer_idx = 0;
uint64_t timestamp_of_last_sync;



// Cannot use LOG_message here, since it would overwrite the caller's message
// due to statically allocated log message buffers

/// @brief Sends a log message to a log file
/// @param filename full path of the log file
/// @param msg The message to be logged
void LOG_to_file(const char filename[], const char msg[])
{

    // add to buffer
    snprintf(log_write_buffer[log_write_buffer_idx], LOG_STRING_MAX_LENGTH, "%s", msg);
    log_write_buffer_idx++;

    uint64_t time_since_last_sync = TIME_get_current_unix_epoch_time_ms() - timestamp_of_last_sync;
    if (time_since_last_sync < LOG_FILE_SYNC_INTERVAL && log_write_buffer_idx < LOG_BUFFER_MAX_STRINGS - 1) {
        LOG_to_umbilical_uart("");
        return;
    }
    log_write_buffer_idx = 0;
    timestamp_of_last_sync = TIME_get_current_unix_epoch_time_ms();

   // LFS_ensure_mounted();
    if (!LFS_is_lfs_mounted) {
        LOG_to_umbilical_uart("\nError writing to system log file: LFS not mounted\n");
        // LOG_to_uhf_radio("\nError writing to system log file: LFS not mounted\n");
        // FIXME(Issue #398): log to memory buffer
        return;
    }

    
    char log_filename[30];
    char timestamp_str[14];
    TIME_get_current_utc_datetime_str(timestamp_str, 14);
    snprintf(log_filename, 30, "logs/%s.log", timestamp_str);

    // We cannot use LFS_append_file due to recursion of the logging system
    lfs_file_t file;
    const int8_t open_result = lfs_file_opencfg(
        &LFS_filesystem, &file, log_filename,
        LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND,
        &LFS_file_cfg
    );
    // If the logs directory doesn't exist, create it.
    // if(open_result < LFS_ERR_NOENT) {
    //     int16_t mkdir_result = lfs_mkdir(&LFS_filesystem, "logs");
    //     if (mkdir_result != 0 && mkdir_result != LFS_ERR_EXIST) {
    //         LOG_message(
    //             LOG_SYSTEM_LOG, 
    //             LOG_SEVERITY_ERROR, 
    //             LOG_SINK_UMBILICAL_UART, 
    //             "Error making log directory: %d", 
    //             mkdir_result);
    //         return;
    //     }
    // }
	if (open_result < 0)
	{

        // This error cannot be logged, except via UART or during an overpass 
        // of the ground station
        LOG_to_umbilical_uart("\nError opening system log file\n");
        // LOG_message(
        //     LOG_SYSTEM_LOG, 
        //     LOG_SEVERITY_ERROR, 
        //     LOG_SINK_UMBILICAL_UART, 
        //     "Error opening system log file: %d", 
        //     open_result);
        
        // LOG_to_uhf_radio("\nError opening system log file\n");
        // FIXME(Issue #398): log to memory buffer
		return;
	}
        // const lfs_soff_t offset = lfs_file_seek(&LFS_filesystem, &file, 0, LFS_SEEK_END);
        // if (offset < 0) {
        //     LOG_to_umbilical_uart("\nError seeking to end of system log file\n");
        //     LOG_to_uhf_radio("\nError seeking to end of system log file\n");
        //     // FIXME(Issue #398): log to memory buffer
        //     return;
        // }
    for (int i = 0; i < log_write_buffer_idx; i++) {
        
        const lfs_ssize_t bytes_written = lfs_file_write(&LFS_filesystem, &file, msg, strlen(msg));
        if (bytes_written < 0) {
            // LOG_to_umbilical_uart("\nError writing to system log file\n");
            // LOG_to_uhf_radio("\nError writing to system log file\n");
            // FIXME(Issue #398): log to memory buffer
            // LOG_message(
            //     LOG_SYSTEM_LOG, 
            //     LOG_SEVERITY_ERROR, 
            //     LOG_SINK_UMBILICAL_UART, 
            //     "Error writing to system log file: %d", 
            //     bytes_written
            // );
            return;
        }
    }
	// Close the File, the storage is not updated until the file is closed successfully
	const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
	// if (close_result < 0) {
    //     LOG_to_umbilical_uart("\nError closing system log file\n");
    //     // LOG_to_uhf_radio("\nError closing system log file\n");
    //     // FIXME(Issue #398): log to memory buffer
	// 	return;
	// }
	
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

