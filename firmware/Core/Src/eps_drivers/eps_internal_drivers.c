
#include "main.h"

#include "debug_tools/debug_uart.h"
#include "debug_tools/debug_i2c.h"
#include "eps_drivers/eps_types.h"
#include "eps_drivers/eps_internal_drivers.h"
#include "uart_handler/uart_handler.h"
#include "stm32/stm32_timing_helpers.h"
#include "log/log.h"

#include <stdint.h>
#include <string.h>

extern UART_HandleTypeDef *UART_eps_port_handle;

// TODO: please verify these numbers in the Software ICD, and remove this comment when satisfied.
static const uint32_t EPS_TX_TIMEOUT_MS = 1000;
static const uint32_t EPS_RX_TIMEOUT_BEFORE_FIRST_BYTE_MS = 50;
static const uint32_t EPS_RX_TIMEOUT_BETWEEN_BYTES_MS = 25;

/// @brief When enabled, the EPS's raw data is sent to the debug UART.
uint32_t CONFIG_EPS_enable_uart_debug_print = 0;

/// @brief Sends a command to the EPS, and receives the response.
/// @param cmd_buf Array of bytes to send to the EPS, including the command code, STID, IVID, etc.
/// @param cmd_buf_len Exact length of the command buffer.
/// @param rx_buf Buffer to store the response. Is filled with the response, without tags.
/// @param rx_buf_len Length of the response buffer. Must be the command length.
/// @return 0 on success, >0 if error.
uint8_t EPS_send_cmd_get_response(
    const uint8_t cmd_buf[], uint8_t cmd_buf_len,
    uint8_t rx_buf[], uint16_t rx_buf_len
) {
    // ASSERT: rx_buf_len must be >= 5 for all commands. Raise error if it's less.
    if (rx_buf_len < EPS_DEFAULT_RX_LEN_MIN) return 1;

    const uint8_t begin_tag_len = 5; // len of "<cmd>" and "<rsp>", without null terminator
    const uint8_t end_tag_len = 6; // len of "</cmd>" and "</rsp>", without null terminator

    // Create place to store "<cmd>ACTUAL COMMAND BYTES</cmd>" (needs about 15 extra chars for tags),
    // and same for receive buffer.
    const uint16_t cmd_buf_with_tags_len = cmd_buf_len + begin_tag_len + end_tag_len;
    const uint16_t rx_len_with_tags = rx_buf_len + begin_tag_len + end_tag_len;
    uint8_t cmd_buf_with_tags[cmd_buf_with_tags_len];
    memset(cmd_buf_with_tags, 0, cmd_buf_with_tags_len);

    // pack the cmd_buf_with_tags buffer
    strcpy((char*) cmd_buf_with_tags, "<cmd>");
    memcpy(&cmd_buf_with_tags[begin_tag_len], cmd_buf, cmd_buf_len);
    strcpy((char*)&cmd_buf_with_tags[begin_tag_len+cmd_buf_len], "</cmd>");

    if (CONFIG_EPS_enable_uart_debug_print) {
        DEBUG_uart_print_str("OBC->EPS DATA (no tags): ");
        DEBUG_uart_print_array_hex(cmd_buf, cmd_buf_len);
        DEBUG_uart_print_str("\nOBC->EPS DATA (with tags): ");
        DEBUG_uart_print_array_hex(cmd_buf_with_tags, cmd_buf_with_tags_len);
        DEBUG_uart_print_str("\n");
    }

    // TX TO EPS
    const HAL_StatusTypeDef tx_status = HAL_UART_Transmit(
        UART_eps_port_handle,
        cmd_buf_with_tags, cmd_buf_with_tags_len, EPS_TX_TIMEOUT_MS
    );
    if (tx_status != HAL_OK) {
        LOG_message(
            LOG_SYSTEM_EPS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "OBC->EPS: tx_status != HAL_OK (%d)", tx_status
        );
        return 2;
    }

    // Reset the EPS UART interrupt variables
    UART_eps_is_expecting_data = 0; // Lock writing to the UART_eps_buffer while we memset it
    for (uint16_t i = 0; i < UART_eps_buffer_len; i++) {
        // Clear the buffer
        // Can't use memset because UART_eps_buffer is volatile
        UART_eps_buffer[i] = 0;
    }
    UART_eps_buffer_write_idx = 0; // Make it start writing from the start
    UART_eps_is_expecting_data = 1; // We are now expecting a response

    // RX FROM EPS, into UART_eps_buffer
    // End when we timeout, when we receive the expected number of bytes.
    const uint32_t start_rx_time = HAL_GetTick();
    while (1) {
        // Check if we've received the expected number of bytes
        if (UART_eps_buffer_write_idx >= rx_len_with_tags) {
            // Best-case: we've received the expected number of bytes, and can be done!
            break;
        }

        if ((UART_eps_buffer_write_idx == 0)) {
            // Check if we've timed out (before the first byte)
            if ((HAL_GetTick() - start_rx_time) > EPS_RX_TIMEOUT_BEFORE_FIRST_BYTE_MS) {
                LOG_message(
                    LOG_SYSTEM_EPS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "EPS->OBC: timeout before first byte received"
                );
                // fatal error; return
                return 3;
            }
        }
        else { // thus, UART_eps_buffer_write_idx > 0
            // Check if we've timed out (between bytes)
            const uint32_t cur_time = HAL_GetTick();
            // Note: Sometimes, because ISRs and C are fun, the UART_eps_last_write_time_ms is
            // greater than `cur_time`. Thus, we must do a safety check that the time difference
            // is positive.
            if (
                (cur_time > UART_eps_last_write_time_ms) // Important seemingly-obvious safety check.
                && ((cur_time - UART_eps_last_write_time_ms) > EPS_RX_TIMEOUT_BETWEEN_BYTES_MS)
            ) {
                LOG_message(
                    LOG_SYSTEM_EPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
                    "EPS->OBC: timeout between bytes. UART_eps_last_write_time_ms=%lu, cur_time=%lu",
                    UART_eps_last_write_time_ms,
                    cur_time
                );
                // Non-fatal error; try to parse what we received
                break;
            }
        }

        // TODO: also end when we receive the end tag
    }

    // End Receiving
    UART_eps_is_expecting_data = 0; // We are no longer expecting a response

    // Optionally, log the received bytes.
    if (CONFIG_EPS_enable_uart_debug_print) {
        DEBUG_uart_print_str("EPS->OBC DATA (with tags): ");
        DEBUG_uart_print_array_hex((uint8_t*)UART_eps_buffer, UART_eps_buffer_write_idx);
        DEBUG_uart_print_str("\n");
    }

    // Check that we've received what we're expecting
    // TODO: if the following cases happen ever during testing, consider allowing them and treating them as WARNINGs
    if (UART_eps_buffer_write_idx == 0) {
        LOG_message(
            LOG_SYSTEM_EPS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "EPS->OBC: UART_eps_buffer_write_idx == 0"
        );
        
        return 12;
    }
    else if (UART_eps_buffer_write_idx < begin_tag_len + EPS_DEFAULT_RX_LEN_MIN + end_tag_len) {
        LOG_message(
            LOG_SYSTEM_EPS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "EPS->OBC: UART_eps_buffer_write_idx < begin_tag_len + EPS_DEFAULT_RX_LEN_MIN + end_tag_len (%d < %d)",
            UART_eps_buffer_write_idx, begin_tag_len+EPS_DEFAULT_RX_LEN_MIN+end_tag_len
        );
        return 13;
    }
    else if (UART_eps_buffer_write_idx < rx_len_with_tags) {
        LOG_message(
            LOG_SYSTEM_EPS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "EPS->OBC: UART_eps_buffer_write_idx < rx_len_with_tags (%d < %d)",
            UART_eps_buffer_write_idx, rx_len_with_tags
        );
        return 14;
    }
    else if (UART_eps_buffer_write_idx > (rx_len_with_tags+2)) {
        // The +2 is for a "\r\n" that might be appended to the end of the response
        LOG_message(
            LOG_SYSTEM_EPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "EPS->OBC: UART_eps_buffer_write_idx > rx_len_with_tags+2 (%d > %d+2)",
            UART_eps_buffer_write_idx, rx_len_with_tags
        );
        return 15;
    }

    // FIXME: pack the rx_buf less-naively (check/confirm the start tag location)

    // Copy the received bytes into the rx_buf.
    // Can't use memcpy because UART_eps_buffer is volatile.
    for (uint16_t i = 0; i < rx_buf_len; i++) {
        rx_buf[i] = UART_eps_buffer[begin_tag_len + i];
    }

    if (CONFIG_EPS_enable_uart_debug_print) {
        DEBUG_uart_print_str("EPS->OBC DATA (no tags): ");
        DEBUG_uart_print_array_hex(rx_buf, rx_buf_len);
        DEBUG_uart_print_str("\n");
    }

    // Check STAT field (Table 3-11) - 0x00 and 0x80 mean success.
    const uint8_t eps_stat_field = rx_buf[4];
    if ((eps_stat_field != 0x00) && (eps_stat_field != 0x80)) {
        // Only a warning. Non-fatal. Not propagated up.
        LOG_message(
            LOG_SYSTEM_EPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "EPS returned an error in the STAT field: 0x%02x (see EPS_SICD Table 3-11)",
            eps_stat_field
        );
    }

    return 0;
}


/// @brief Sends an argumentless command to the EPS (expecting no data in the response).
/// @param command_code Command code number.
/// @return 0 on success, >0 if error.
uint8_t EPS_run_argumentless_cmd(uint8_t command_code) {
    const uint8_t cmd_len = 4;
    const uint8_t rx_len = EPS_DEFAULT_RX_LEN_MIN;

    uint8_t cmd_buf[cmd_len];
    uint8_t rx_buf[rx_len];
    
    cmd_buf[0] = EPS_COMMAND_STID;
    cmd_buf[1] = EPS_COMMAND_IVID;
    cmd_buf[2] = command_code; // "CC"
    cmd_buf[3] = EPS_COMMAND_BID;

    const uint8_t comms_err = EPS_send_cmd_get_response(cmd_buf, cmd_len, rx_buf, rx_len);
    return comms_err;
}
