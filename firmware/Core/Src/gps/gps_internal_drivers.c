#include "main.h"

#include "debug_tools/debug_uart.h"
#include "debug_tools/debug_i2c.h"
#include "gps/gps_internal_drivers.h"
#include "uart_handler/uart_handler.h"
#include "stm_drivers/timing_helpers.h"
#include "uart_handler/uart_handler.h"
#include "log/log.h"

#include <stdint.h>
#include <string.h>

extern UART_HandleTypeDef *UART_gps_port_handle;

const uint32_t GPS_RX_TIMEOUT_BEFORE_FIRST_BYTE_MS = 800;
const uint32_t GPS_RX_TIMEOUT_BETWEEN_BYTES_MS = 800;

const char *GPS_LOW_POWER_COMMANDS[] = { "ANTENNAPOWER" };
const uint8_t GPS_LOW_POWER_COMMANDS_LEN = 1;

/// @brief Sends a log command to the GPS, and receives the response.
/// @param cmd_buf log command string to send to the GPS.
/// @param cmd_buf_len Exact length of the log command string.
/// @param rx_buf Buffer to store the response.
/// @param rx_buf_len Length of the response buffer.
/// @param rx_buf_max_size Maximum length of the response buffer.
/// @return 0 on success, >0 if error.
/// @note This function is intended for "once" log commands
uint8_t GPS_send_cmd_get_response(const char *cmd_buf, uint8_t cmd_buf_len, uint8_t rx_buf[], uint16_t rx_buf_len, const uint16_t rx_buf_max_size)
{
    
    // Reset the GPS UART interrupt variables
    GPS_set_uart_interrupt_state(0); // Lock writing to the UART_gps_buffer while we memset it
    for (uint16_t i = 0; i < UART_gps_buffer_len; i++)
    {
        // Clear the buffer
        // Can't use memset because UART_gps_buffer is volatile
        UART_gps_buffer[i] = 0;
    }
    UART_gps_buffer_write_idx = 0;		// Make it start writing from the start

    // TX TO GPS
    const HAL_StatusTypeDef tx_status = HAL_UART_Transmit(
        UART_gps_port_handle,
        (uint8_t *)cmd_buf,
        cmd_buf_len,
        100);

    if (tx_status != HAL_OK)
    {
        LOG_message(
            LOG_SYSTEM_GPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "GPS ERROR: tx_status != HAL_OK (%d)\n",
            tx_status);

        return 1;
    }

    GPS_set_uart_interrupt_state(1);	// We are now expecting a response

    // FIXME: Update the timeouts with the actual times, it currently works with 800 ms and 500ms does not work
    // GPS takes time to respond, first section of log response ie <OK\n [COM1] is quick but the rest of the data response takes a while
    

    // RX FROM GPS, into UART_gps_buffer
    const uint32_t start_rx_time = HAL_GetTick();
    while (1) {
        if ((UART_gps_buffer_write_idx == 0)) {
            // Check if we've timed out (before the first byte)
            if ((HAL_GetTick() - start_rx_time) > GPS_RX_TIMEOUT_BEFORE_FIRST_BYTE_MS) {
                LOG_message(
                    LOG_SYSTEM_GPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
                    "GPS ERROR: Timeout before receiving any data"
                );

                // Disable the UART gps channel
                GPS_set_uart_interrupt_state(0);
            
                // fatal error; return
                return 2;
            }
        }
        else { // thus, UART_eps_buffer_write_idx > 0
            // Check if we've timed out (between bytes)
            const uint32_t cur_time = HAL_GetTick();
            // Note: Sometimes, because ISRs and C are fun, the UART_eps_last_write_time_ms is
            // greater than `cur_time`. Thus, we must do a safety check that the time difference
            // is positive.
            if (
                (cur_time > UART_gps_last_write_time_ms) // Important seemingly-obvious safety check.
                && ((cur_time - UART_gps_last_write_time_ms) > GPS_RX_TIMEOUT_BETWEEN_BYTES_MS)
            ) {
                LOG_message(
                    LOG_SYSTEM_GPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
                    "GPS WARNING: Timeout during receiving of response. UART_gps_last_write_time_ms=%lu, cur_time=%lu",
                    UART_gps_last_write_time_ms,
                    cur_time
                );
                // Non-fatal error; try to parse what we received
                break;
            }
        }
    }

    // End Receiving
    GPS_set_uart_interrupt_state(0); // We are no longer expecting a response

    // Logging the received response
    LOG_message(
        LOG_SYSTEM_GPS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "GPS Buffer Data: %s",
        UART_gps_buffer
    );

    // Check that we've received what we're expecting
    // TODO: If the following cases happen ever during testing, consider allowing them and treating them as WARNINGs
    if (UART_gps_buffer_write_idx > rx_buf_max_size)
    {
        LOG_message(
            LOG_SYSTEM_GPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "GPS ERROR: UART_gps_buffer overflow");
        return 3;
    }

    // Copy the log response from the UART gps buffer to the rx_buf[] and clear the buffer
    for (uint16_t i = 0; i < UART_gps_buffer_write_idx; i++) {
        rx_buf[i] = UART_gps_buffer[i];
    }

    LOG_message(
        LOG_SYSTEM_GPS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "rx_buf: %s",
        rx_buf
    );

    return 0;
}

/// @brief Sends an enable/disable command to the GPS, and receives the response.
/// @param enable_disable_flag 1 to enable (power on), 0 to disable (power off)
/// @return 0 on success, >0 if error.
uint8_t GPS_set_power_enabled(uint16_t enable_disable_flag)
{
    // This flag is used to indicate that one of the commands had errors, and to return > 0
    uint8_t errorFlag = 0;

    // Loop through all available commands to either enable or disable
    for (uint8_t i = 0; i < GPS_LOW_POWER_COMMANDS_LEN; i++)
    {
        char full_command[128];
        snprintf(full_command, sizeof(full_command), "%s %s\n", GPS_LOW_POWER_COMMANDS[i], enable_disable_flag == 1 ? "ON" : "OFF");
        const uint16_t full_command_len = strlen(full_command);

        // The following buffer is created to satisfy GPS_send_cmd_get_response args, but it's not used further in this function
        const uint16_t GPS_rx_buffer_max_size = 512;
        uint16_t GPS_rx_buffer_len = 0;
        uint8_t GPS_rx_buffer[GPS_rx_buffer_max_size];
        memset(GPS_rx_buffer, 0, GPS_rx_buffer_max_size); 
        
        // Send log command to GPS and receive response
        const uint8_t gps_cmd_response = GPS_send_cmd_get_response(
            full_command, full_command_len, GPS_rx_buffer, GPS_rx_buffer_len, GPS_rx_buffer_max_size);

        if (gps_cmd_response != 0)
        {
            // Change errorFlag to 1 to indicate that we should return > 0 AFTER all commands are attempted
            errorFlag = 1;
        }
        osDelay(1000);
    }

    if (errorFlag != 0)
    {
        return 1;
    }

    return 0;
}