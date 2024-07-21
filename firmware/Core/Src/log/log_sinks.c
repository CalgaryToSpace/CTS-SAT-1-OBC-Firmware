#include "log/log.h"
#include "log/log_sinks.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"

extern UART_HandleTypeDef hlpuart1;
extern uint8_t LFS_is_lfs_mounted;


/// @brief Sends a log message to a log file
/// @param filename full path of the log file
/// @param msg The message to be logged
/// @return void
void LOG_to_file(const char filename[], const char msg[])
{
    if (!LFS_is_lfs_mounted) {
        LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), 
                "Error writing to system log file: LFS not mounted.\n");
        return;
    }

    // We cannot use LFS_append_file due to recursion of the logging system
    lfs_file_t file;
    const int8_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, filename, LFS_O_RDWR | LFS_O_CREAT | LFS_O_APPEND, &LFS_file_cfg);
	if (open_result < 0)
	{
        // This error cannot be logged, except via UART or during an overpass 
        // of the ground station
        LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), 
                "Error opening system log file.\n");
		return;
	}
    const lfs_soff_t offset = lfs_file_seek(&LFS_filesystem, &file, 0, LFS_SEEK_END);
    if (offset < 0) {
        LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), 
                "Error seeking to end of system log file.\n");
        return;
    }

	const lfs_ssize_t bytes_written = lfs_file_write(&LFS_filesystem, &file, msg, strlen(msg));
	if (bytes_written < 0) {
        LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), 
                "Error writing to system log file.\n");
		return;
	}
	
	// Close the File, the storage is not updated until the file is closed successfully
	const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
	if (close_result < 0) {
        LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), 
                "Error closing system log file.\n");
		return;
	}
	
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
    // FIXME: replace with UHF radio transmission
    LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_CRITICAL, LOG_SINK_UMBILICAL_UART, "TODO: replace this with a UHF RADIO transmission");
    return;
}

