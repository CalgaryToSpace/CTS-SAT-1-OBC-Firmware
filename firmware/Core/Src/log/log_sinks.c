#include "log/log.h"
#include "log/log_sinks.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"
#include "comms_drivers/comms_tx.h"
#include "main.h"

extern UART_HandleTypeDef hlpuart1;
extern uint8_t LFS_is_lfs_mounted;

// Cannot use LOG_message here, since it would overwrite the caller's message
// due to statically allocated log message buffers

/// @brief Sends a log message to a log file
/// @param filename full path of the system-specific log file - currently not used
/// @param msg The message to be logged
void LOG_to_file_eager(const char filename[], const char msg[]) {
    if (!LFS_is_lfs_mounted) {
        LOG_to_umbilical_uart("\nError writing to system log file: LFS not mounted\n");
        LOG_to_uhf_radio("\nError writing to system log file: LFS not mounted\n");
        // FIXME(Issue #398): log to memory buffer
        return;
    }

    // We cannot use LFS_append_file due to recursion of the logging system
    lfs_file_t file;
    const int8_t open_result = lfs_file_opencfg(
        &LFS_filesystem, &file, filename,
        LFS_O_RDWR | LFS_O_CREAT | LFS_O_APPEND,
        &LFS_file_cfg
    );
	if (open_result < 0)
	{
        // This error cannot be logged, except via UART or during an overpass 
        // of the ground station
        LOG_to_umbilical_uart("\nError opening system log file\n");
        LOG_to_uhf_radio("\nError opening system log file\n");
        // FIXME(Issue #398): log to memory buffer
		return;
	}
    const lfs_soff_t offset = lfs_file_seek(&LFS_filesystem, &file, 0, LFS_SEEK_END);
    if (offset < 0) {
        LOG_to_umbilical_uart("\nError seeking to end of system log file\n");
        LOG_to_uhf_radio("\nError seeking to end of system log file\n");
        // FIXME(Issue #398): log to memory buffer
        return;
    }

	const lfs_ssize_t bytes_written = lfs_file_write(&LFS_filesystem, &file, msg, strlen(msg));
	if (bytes_written < 0) {
        LOG_to_umbilical_uart("\nError writing to system log file\n");
        LOG_to_uhf_radio("\nError writing to system log file\n");
        // FIXME(Issue #398): log to memory buffer
		return;
	}
	
	// Close the File, the storage is not updated until the file is closed successfully
	const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
	if (close_result < 0) {
        LOG_to_umbilical_uart("\nError closing system log file\n");
        LOG_to_uhf_radio("\nError closing system log file\n");
        // FIXME(Issue #398): log to memory buffer
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
    const uint8_t result = COMMS_downlink_log_message(msg);
    if (result != 0) {
        LOG_to_umbilical_uart("\nError sending log message to UHF radio\n");
        // FIXME(Issue #398): log to memory buffer
    }
    return;
}

