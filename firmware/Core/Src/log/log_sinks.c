#include "log/log.h"
#include "log/log_sinks.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"
#include "comms_drivers/comms_tx.h"
// #include "timekeeping/timekeeping.h"
#include <stdio.h>
#include "main.h"

extern UART_HandleTypeDef hlpuart1;

// A context for the current log file. Externed in log.c and rtos_background_upkeep.c
LOG_File_Context_t current_log_file_ctx;

//FIXME: need to do a soak test te ensure this doesn't break anything.
/// @brief Write a log message to an lfs log file. 
/// @param msg The message to be logged
/// @note lfs files are not updated util the file is closed or synced. syncing is done periodically in the rtos_background_upkeep task.
void LOG_to_file(const char msg[])
{
    LFS_ensure_mounted();

    LOG_ensure_current_log_file_is_open();

    LOG_write_to_current_log_file(msg, strlen(msg));
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

