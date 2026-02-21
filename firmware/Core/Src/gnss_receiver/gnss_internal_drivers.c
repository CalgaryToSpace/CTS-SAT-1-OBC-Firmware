#include "main.h"

#include "debug_tools/debug_uart.h"
#include "debug_tools/debug_i2c.h"
#include "gnss_receiver/gnss_internal_drivers.h"
#include "uart_handler/uart_handler.h"
#include "stm32/stm32_timing_helpers.h"
#include "uart_handler/uart_handler.h"
#include "log/log.h"

#include <stdint.h>
#include <string.h>

extern UART_HandleTypeDef *UART_gnss_port_handle;

const uint32_t GNSS_RX_TIMEOUT_BEFORE_FIRST_BYTE_MS = 800;
const uint32_t GNSS_RX_TIMEOUT_BETWEEN_BYTES_MS = 2500; // Lots of commands pause in the middle (e.g., BESTXYZA) as it contemplates its position in the universe.

/// @brief Sends a log command to the GNSS, and receives the response.
/// @param cmd_buf Log command string to send to the GNSS, without EOL characters.
/// @param cmd_buf_len Exact length of the log command string.
/// @param rx_buf Buffer to store the response (not necessarily null terminated).
/// @param rx_buf_max_size Size of the response buffer.
/// @param rx_buf_len_dest Pointer to place to store the length of the response buffer (not necessarily null terminated).
/// @return 0 on success, >0 if error.
/// @note This function is intended for "once" log commands and control commands.
/// @note This function does not validate the response, as related to the request.
uint8_t GNSS_send_cmd_get_response(
    const char *cmd_buf, uint8_t cmd_buf_len,
    uint8_t rx_buf[],
    const uint16_t rx_buf_max_size,
    uint16_t* rx_buf_len_dest
) {
    // Reset the GNSS UART interrupt variables
    GNSS_set_uart_interrupt_state(0); // Lock writing to the UART_gnss_buffer while we memset it
    for (uint16_t i = 0; i < UART_gnss_buffer_len; i++)
    {
        // Clear the buffer
        // Can't use memset because UART_gnss_buffer is volatile
        UART_gnss_buffer[i] = 0;
    }
    
    // Make it start writing to the start of the buffer.
    UART_gnss_buffer_write_idx = 0;

    // TX TO GNSS
    const HAL_StatusTypeDef tx_status_1 = HAL_UART_Transmit(
        UART_gnss_port_handle,
        (uint8_t *)cmd_buf,
        cmd_buf_len,
        100
    );
    const HAL_StatusTypeDef tx_status_2 = HAL_UART_Transmit(
        UART_gnss_port_handle,
        (uint8_t *)"\r\n",
        3,
        100
    );

    if (tx_status_1 != HAL_OK || tx_status_2 != HAL_OK) {
        LOG_message(
            LOG_SYSTEM_GNSS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "GNSS ERROR: tx_status != HAL_OK (%d, %d)",
            tx_status_1, tx_status_2
        );
        return 1;
    }

    GNSS_set_uart_interrupt_state(1);	// We are now expecting a response

    // FIXME: Update the timeouts with the actual times, it currently works with 800 ms and 500ms does not work
    // GNSS takes time to respond, first section of log response ie <OK\n [COM1] is quick but the rest of the data response takes a while
    

    // RX FROM GNSS, into UART_gnss_buffer
    const uint32_t start_rx_time = HAL_GetTick();
    while (1) {
        if ((UART_gnss_buffer_write_idx == 0)) {
            // Check if we've timed out (before the first byte)
            if ((HAL_GetTick() - start_rx_time) > GNSS_RX_TIMEOUT_BEFORE_FIRST_BYTE_MS) {
                LOG_message(
                    LOG_SYSTEM_GNSS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
                    "GNSS ERROR: Timeout before receiving any data"
                );

                // Disable the UART gnss channel
                GNSS_set_uart_interrupt_state(0);

                *rx_buf_len_dest = 0;
            
                return 2; // Error: Timeout before receiving any data.
            }
        }
        else { // thus, UART_eps_buffer_write_idx > 0
            // Check if we've timed out (between bytes)
            const uint32_t cur_time = HAL_GetTick();
            // Note: Sometimes, because ISRs and C are fun, the UART_eps_last_write_time_ms is
            // greater than `cur_time`. Thus, we must do a safety check that the time difference
            // is positive.
            if (
                (cur_time > UART_gnss_last_write_time_ms) // Important seemingly-obvious safety check.
                && ((cur_time - UART_gnss_last_write_time_ms) > GNSS_RX_TIMEOUT_BETWEEN_BYTES_MS)
            ) {
                // Non-fatal error. Parse what we've received.
                break;
            }

            // TODO: Add an "end of message" check here, probably.
            
            // Exit if we've received all the buffer can hold.
            if (UART_gnss_buffer_write_idx >= rx_buf_max_size) {
                break;
            }
        }
    }

    // End Receiving
    GNSS_set_uart_interrupt_state(0); // We are no longer expecting a response

    UART_gnss_buffer[UART_gnss_buffer_write_idx] = '\0'; // Null-terminate the string

    // Check that we've received what we're expecting
    if (UART_gnss_buffer_write_idx >= rx_buf_max_size) {
        LOG_message(
            LOG_SYSTEM_GNSS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "GNSS: Received more data (>=%d bytes) than rx_buf_max_size (%d bytes)",
            UART_gnss_buffer_write_idx,
            rx_buf_max_size
        );
        
        *rx_buf_len_dest = rx_buf_max_size - 1;
        // No need to return here. We can still pass back the data we have.
    }
    else {
        *rx_buf_len_dest = UART_gnss_buffer_write_idx;
    }

    // Copy the log response from the UART gnss buffer to the rx_buf[] and clear the buffer.
    for (uint16_t i = 0; i < (*rx_buf_len_dest); i++) {
        rx_buf[i] = UART_gnss_buffer[i];
    }

    // Ensure the final output buffer (rx_buf) is null-terminated, no matter what.
    rx_buf[rx_buf_max_size - 1] = '\0';
    rx_buf[*rx_buf_len_dest] = '\0';

    return 0;
}
