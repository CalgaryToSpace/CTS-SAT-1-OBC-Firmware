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
        else { // thus, UART_gps_buffer_write_idx > 0
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
    uint16_t bytes_to_copy = (UART_gps_buffer_write_idx > rx_buf_max_size) ? rx_buf_max_size : UART_gps_buffer_write_idx;
    for (uint16_t i = 0; i < bytes_to_copy; i++) {
        rx_buf[i] = UART_gps_buffer[i];
    }

    LOG_message(
        LOG_SYSTEM_GPS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "rx_buf: %.*s",
        bytes_to_copy,
        rx_buf
    );

    return 0;
}
