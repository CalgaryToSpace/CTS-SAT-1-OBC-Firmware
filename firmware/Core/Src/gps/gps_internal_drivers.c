#include "main.h"

#include "debug_tools/debug_uart.h"
#include "debug_tools/debug_i2c.h"
#include "gps/gps_internal_drivers.h"
#include "gps/gps_args_helpers.h"
#include "uart_handler/uart_handler.h"
#include "stm_drivers/timing_helpers.h"
#include "log/log.h"

#include <stdint.h>
#include <string.h>

extern UART_HandleTypeDef *UART_gps_port_handle;

// TODO: please verify these numbers in the Software ICD, and remove this comment when satisfied.
const uint32_t EPS_RX_TIMEOUT_BEFORE_FIRST_BYTE_MS = 50;
const uint32_t EPS_RX_TIMEOUT_BETWEEN_BYTES_MS = 25;

/// @brief Sends a command to the GPS, and receives the response.
/// @param cmd_buf Array of bytes to send to the EPS, including the command code, STID, IVID, etc.
/// @param cmd_buf_len Exact length of the command buffer.
/// @param rx_buf Buffer to store the response. Is filled with the response, without tags.
/// @param rx_buf_len Length of the response buffer. Must be the command length.
/// @return 0 on success, >0 if error.
uint8_t GPS_send_cmd_get_response(const char cmd_buf, uint8_t cmd_buf_len,
        uint8_t rx_buf[], uint16_t rx_buf_len){

	// ASSERT: rx_buf_len must be >= 5 for all commands. Raise error if it's less.
	if (rx_buf_len < GPS_DEFAULT_RX_LEN_MIN) return 1;

    // cmd_buf should contain the string of the log command to be sent.

    // Error checking to ensure that the log command starts with the appropriate log command prefix
    const uint8_t gps_log_command_prefix_check = GPS_check_starts_with_log_prefix(cmd_buf);

    if(gps_log_command_prefix_check > 0){
        // Error: Invalid log command prefix
        return 2;
    }


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

	if (EPS_ENABLE_DEBUG_PRINT) {
		DEBUG_uart_print_str("OBC->EPS DATA (no tags): ");
		DEBUG_uart_print_array_hex(cmd_buf, cmd_buf_len);
		DEBUG_uart_print_str("\nOBC->EPS DATA (with tags): ");
		DEBUG_uart_print_array_hex(cmd_buf_with_tags, cmd_buf_with_tags_len);
		DEBUG_uart_print_str("\n");
	}

	// TX TO GPS
	const HAL_StatusTypeDef tx_status = HAL_UART_Transmit(
		UART_gps_port_handle,
		(uint8_t*) cmd_buf_with_tags, cmd_buf_with_tags_len, 1000); // FIXME: update the timeout
	if (tx_status != HAL_OK) {
		if (EPS_ENABLE_DEBUG_PRINT) {
			char msg[200];
			sprintf(msg, "OBC->EPS ERROR: tx_status != HAL_OK (%d)\n", tx_status);
			DEBUG_uart_print_str(msg);
		}
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
				if (EPS_ENABLE_DEBUG_PRINT) {
					DEBUG_uart_print_str("EPS->OBC ERROR: timeout before first byte received\n");
				}
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
				if (EPS_ENABLE_DEBUG_PRINT) {
					LOG_message(
						LOG_SYSTEM_EPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
						"EPS->OBC WARNING: timeout between bytes. UART_eps_last_write_time_ms=%lu, cur_time=%lu",
						UART_eps_last_write_time_ms,
						cur_time
					);
				}
				// Non-fatal error; try to parse what we received
				break;
			}
		}

		// TODO: also end when we receive the end tag
	}

	// End Receiving
	UART_eps_is_expecting_data = 0; // We are no longer expecting a response

	// For now, log the received bytes
	DEBUG_uart_print_str("EPS->OBC DATA (with tags): ");
	DEBUG_uart_print_array_hex((uint8_t*)UART_eps_buffer, UART_eps_buffer_write_idx);
	DEBUG_uart_print_str("\n");

	// Check that we've received what we're expecting
	// TODO: if the following cases happen ever during testing, consider allowing them and treating them as WARNINGs
	if (UART_eps_buffer_write_idx == 0) {
		if (EPS_ENABLE_DEBUG_PRINT) {
			DEBUG_uart_print_str("EPS->OBC ERROR: UART_eps_buffer_write_idx == 0\n");
		}
		return 12;
	}
	else if (UART_eps_buffer_write_idx < begin_tag_len + GPS_DEFAULT_RX_LEN_MIN + end_tag_len) {
		if (EPS_ENABLE_DEBUG_PRINT) {
			char msg[200];
			snprintf(
				msg, sizeof(msg),
				"EPS->OBC ERROR: UART_eps_buffer_write_idx < begin_tag_len + EPS_DEFAULT_RX_LEN_MIN + end_tag_len (%d < %d)\n",
				UART_eps_buffer_write_idx, begin_tag_len+GPS_DEFAULT_RX_LEN_MIN+end_tag_len);
			DEBUG_uart_print_str(msg);
		}
		return 13;
	}
	else if (UART_eps_buffer_write_idx < rx_len_with_tags) {
		if (EPS_ENABLE_DEBUG_PRINT) {
			char msg[200];
			snprintf(
				msg, sizeof(msg),
				"EPS->OBC ERROR: UART_eps_buffer_write_idx < rx_len_with_tags (%d < %d)\n",
				UART_eps_buffer_write_idx, rx_len_with_tags);
			DEBUG_uart_print_str(msg);
		}
		return 14;
	}
	else if (UART_eps_buffer_write_idx > (rx_len_with_tags+2)) {
		// The +2 is for a "\r\n" that might be appended to the end of the response
		if (EPS_ENABLE_DEBUG_PRINT) {
			char msg[200];
			snprintf(
				msg, sizeof(msg),
				"EPS->OBC WARNING: UART_eps_buffer_write_idx > rx_len_with_tags+2 (%d > %d+2)\n",
				UART_eps_buffer_write_idx, rx_len_with_tags);
			DEBUG_uart_print_str(msg);
		}
		return 15;
	}

	// FIXME: pack the rx_buf less-naively (check/confirm the start tag location)

	// Copy the received bytes into the rx_buf.
	// Can't use memcpy because UART_eps_buffer is volatile.
	for (uint16_t i = 0; i < rx_buf_len; i++) {
		rx_buf[i] = UART_eps_buffer[begin_tag_len + i];
	}

	if (EPS_ENABLE_DEBUG_PRINT) {
		DEBUG_uart_print_str("EPS->OBC DATA (no tags): ");
		DEBUG_uart_print_array_hex(rx_buf, rx_buf_len);
		DEBUG_uart_print_str("\n");
	}

	// Check STAT field (Table 3-11) - 0x00 and 0x80 mean success
	// TODO: consider doing this check in the next level up
	const uint8_t eps_stat_field = rx_buf[4];
	if ((eps_stat_field != 0x00) && (eps_stat_field != 0x80)) {
		if (EPS_ENABLE_DEBUG_PRINT) {
			char msg[100];
			snprintf(
				msg, sizeof(msg),
				"EPS returned an error in the STAT field: 0x%02x (see ESP_SICD Table 3-11)\n",
				eps_stat_field);
			DEBUG_uart_print_str(msg);
		}
	}

	return 0;
}