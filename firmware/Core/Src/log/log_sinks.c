#include "log/log.h"
#include "log/log_sinks.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"
#include "comms_drivers/comms_tx.h"
#include "timekeeping/timekeeping.h"
#include <stdio.h>


extern UART_HandleTypeDef hlpuart1;
extern uint8_t LFS_is_lfs_mounted;


lfs_file_t current_log_file;
int8_t log_file_is_open = 0;

/* Some notes: 
It seems that having a lot of files in in the file system causes the watchdog to trigger, below details why I think this.
In commit (cd5e9651d79e4070c82aa8417f694b804855bf7a) we attempt to write to a new file after a period of time.
If you format the the storage and flash this commit, it works fine for a bit but at some point will get stuck in a loop of triggering the watchdog.
However when we write to a single file it works fine for as long as i have tested it. Though we still need a proper soak test.
*/
//FIXME: need to do a soak test te ensure this doesn't break anything.
/// @brief Write a log message to an lfs log file. 
/// @param msg The message to be logged
/// @note lfs files are not updated util the file is closed or synced. syncing is done periodically in the rtos_background_upkeep task.
void LOG_to_file(const char msg[])
{
    if (!LFS_is_lfs_mounted) {
            LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_FILE),
                "Error logging to file: LFS not mounted");// FIXME(Issue #398): log to memory buffer
        return;
    }

    if (!log_file_is_open) {
        const int16_t open_result = lfs_file_opencfg(&LFS_filesystem, &current_log_file, "logs/log.txt",
            LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND, &LFS_file_cfg);
        if (open_result < 0){
            LOG_message(LOG_SYSTEM_LOG, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_FILE),
                "Failed to open log file: %d", open_result);
                return;
        }
        log_file_is_open = 1;
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

