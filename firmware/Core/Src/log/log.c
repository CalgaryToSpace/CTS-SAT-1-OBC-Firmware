#include "log/log.h"
#include "debug_tools/debug_uart.h"
#include "log/ulog.h"
#include "stm32l4xx_hal.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

// currently links GSE computer to dev kit
extern UART_HandleTypeDef hlpuart1;

extern uint8_t LFS_is_lfs_mounted;
extern lfs_t lfs;

void LOG_prep_full_log_message(char *full_log_msg, ulog_level_t severity, char *msg) {
    // TODO add time
    snprintf(full_log_msg, LOG_MAX_LENGTH, "<time> [%s]: %s\n",
             ulog_level_name(severity), msg);
    return;
}

// msg should not have a trailing \n.
void LOG_send_to_system_log_file(ulog_level_t severity, char *msg) {
    char log_msg[LOG_MAX_LENGTH];
    LOG_prep_full_log_message(log_msg, severity, msg);

    // We cannot use LFS_append_file due to recursion of the logging system
    lfs_file_t file;
    const int8_t open_result = lfs_file_open(&lfs, &file, LOG_SYSTEM_FILENAME, LFS_O_RDWR | LFS_O_CREAT | LFS_O_APPEND);
	if (open_result < 0)
	{
        // This error cannot be logged, except via UART or during an overpass 
        // of the ground station
        char *error_msg = "Error opening system log file.\n";
        LOG_send_to_uart(ULOG_CRITICAL_LEVEL, error_msg);
        LOG_send_to_radio(ULOG_CRITICAL_LEVEL, error_msg);
		return;
	}
    const lfs_soff_t offset = lfs_file_seek(&lfs, &file, 0, LFS_SEEK_END);
    if (offset < 0) {
        char *error_msg = "Error seeking to end of system log file.\n";
        LOG_send_to_uart(ULOG_CRITICAL_LEVEL, error_msg);
        LOG_send_to_radio(ULOG_CRITICAL_LEVEL, error_msg);
        return;
    }

	const lfs_ssize_t bytes_written = lfs_file_write(&lfs, &file, log_msg, strlen(log_msg));
	if (bytes_written < 0) {
        char *error_msg = "Error writing to system log file.\n";
        LOG_send_to_uart(ULOG_CRITICAL_LEVEL, error_msg);
        LOG_send_to_radio(ULOG_CRITICAL_LEVEL, error_msg);
		return;
	}
	
	// Close the File, the storage is not updated until the file is closed successfully
	const int8_t close_result = lfs_file_close(&lfs, &file);
	if (close_result < 0) {
        char *error_msg = "Error closing system log file.\n";
        LOG_send_to_uart(ULOG_CRITICAL_LEVEL, error_msg);
        LOG_send_to_radio(ULOG_CRITICAL_LEVEL, error_msg);
		return;
	}
	
	return;
}

// msg should not have a trailing \n.
void LOG_send_to_uart(ulog_level_t severity, char *msg) {
    // TODO: rename LOG_send_to_umbilical?
    char log_msg[LOG_MAX_LENGTH];
    LOG_prep_full_log_message(log_msg, severity, msg);
    HAL_UART_Transmit(&hlpuart1, (uint8_t *)log_msg, strlen(log_msg),
                      LOG_UART_TRANSMIT_TIMEOUT);
    return;
}

// msg should not have a trailing \n.
void LOG_send_to_radio(ulog_level_t severity, char *msg) {
    char log_msg[LOG_MAX_LENGTH];
    LOG_prep_full_log_message(log_msg, severity, msg);
    // TODO Replace with a radio command 
    DEBUG_uart_print_str("TODO: replace with a RADIO transmission\n");
    DEBUG_uart_print_str(log_msg);
    return;
}

void LOG_init(void) {
    ULOG_INIT();
    ULOG_SUBSCRIBE(LOG_send_to_uart, ULOG_TRACE_LEVEL);
    ULOG_SUBSCRIBE(LOG_send_to_radio, ULOG_TRACE_LEVEL);
    ULOG_SUBSCRIBE(LOG_send_to_system_log_file, ULOG_TRACE_LEVEL);
    ULOG_INFO("Initialized log system");
  
    return;
}

