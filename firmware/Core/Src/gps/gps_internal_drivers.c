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
/// TODO: Update docString
uint8_t GPS_send_cmd_get_response(const char cmd_buf, uint8_t cmd_buf_len,
        uint8_t rx_buf[], const uint8_t rx_buf_max_size, uint16_t *rx_buf_len){

	// ASSERT: rx_buf_max_size must be >= 9 for all commands. Raise error if it's less.
	// TODO: Determine if this is needed, this condition will always be false
	if (rx_buf_max_size < GPS_DEFAULT_RX_LEN_MIN){
        return 1;
    } 


    // TODO: Determine if error checking of cmd_buf to ensure that the log command starts with the appropriate log command prefix is necessary
	// cmd_buf contains the string of the log command to be sent.

	// TX TO GPS
	const HAL_StatusTypeDef tx_status = HAL_UART_Transmit(
		UART_gps_port_handle,
		(uint8_t*) cmd_buf, 
		cmd_buf_len, 
		HAL_MAX_DELAY); // FIXME: update the timeout

	if (tx_status != HAL_OK) {
		if (GPS_ENABLE_DEBUG_PRINT) {
			LOG_message(
				LOG_SYSTEM_GPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
				"OBC->GPS ERROR: tx_status != HAL_OK (%d)\n",
				tx_status
			);
		}
		return 2;
	}

	// Reset the GPS UART interrupt variables
	UART_gps_is_expecting_data = 0; // Lock writing to the UART_eps_buffer while we memset it
	for (uint16_t i = 0; i < UART_gps_buffer_len; i++) {
		// Clear the buffer
		// Can't use memset because UART_eps_buffer is volatile
		UART_gps_buffer[i] = 0;
	}
	UART_gps_buffer_write_idx = 0; // Make it start writing from the start
	UART_gps_is_expecting_data = 1; // We are now expecting a response

	// RX FROM GPS, into UART_gps_buffer
	// End when we timeout, when we receive the expected number of bytes.
	const uint32_t start_rx_time = HAL_GetTick();

	if (UART_gps_buffer_write_idx >= rx_buf_len) {
			// Best-case: we've received the expected number of bytes, and can be done!
			LOG_message(
				LOG_SYSTEM_GPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
				"OBC->GPS ERROR: Write index wrong with rx buffer lengthS\n"
			);
			return 3;
	}

	if ((UART_eps_buffer_write_idx == 0)) {
			// Check if we've timed out (before the first byte)
			if ((HAL_GetTick() - start_rx_time) > EPS_RX_TIMEOUT_BEFORE_FIRST_BYTE_MS) {
				if (GPS_ENABLE_DEBUG_PRINT) {
					LOG_message(
						LOG_SYSTEM_GPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
						"GPS->OBC ERROR: timeout before first byte received\n"
					);
				}
				// fatal error; return
				return 3;
			}
	}

	while (1) {
		// Check if we've received the expected number of bytes
		if (UART_gps_buffer_write_idx >= rx_buf_len) {
			// Best-case: we've received the expected number of bytes, and can be done!
			break;
		}

		if ((UART_eps_buffer_write_idx == 0)) {
			// Check if we've timed out (before the first byte)
			if ((HAL_GetTick() - start_rx_time) > EPS_RX_TIMEOUT_BEFORE_FIRST_BYTE_MS) {
				if (GPS_ENABLE_DEBUG_PRINT) {
					LOG_message(
						LOG_SYSTEM_GPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
						"GPS->OBC ERROR: timeout before first byte received\n"
					);
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
				(cur_time > UART_gps_last_write_time_ms) // Important seemingly-obvious safety check.
				&& ((cur_time - UART_gps_last_write_time_ms) > EPS_RX_TIMEOUT_BETWEEN_BYTES_MS)
			) {
				if (GPS_ENABLE_DEBUG_PRINT) {
					LOG_message(
						LOG_SYSTEM_GPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
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
	UART_gps_is_expecting_data = 0; // We are no longer expecting a response

	// For now, log the received bytes
	DEBUG_uart_print_str("EPS->OBC DATA (with tags): ");
	DEBUG_uart_print_array_hex((uint8_t*)UART_eps_buffer, UART_eps_buffer_write_idx);
	DEBUG_uart_print_str("\n");

	// Check that we've received what we're expecting
	// TODO: if the following cases happen ever during testing, consider allowing them and treating them as WARNINGs
	if (UART_eps_buffer_write_idx == 0) {
		if (GPS_ENABLE_DEBUG_PRINT) {
			DEBUG_uart_print_str("EPS->OBC ERROR: UART_eps_buffer_write_idx == 0\n");
		}
		return 12;
	}
	else if (UART_gps_buffer_write_idx < begin_tag_len + GPS_DEFAULT_RX_LEN_MIN + end_tag_len) {
		if (GPS_ENABLE_DEBUG_PRINT) {
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
		if (GPS_ENABLE_DEBUG_PRINT) {
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
		if (GPS_ENABLE_DEBUG_PRINT) {
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

	if (GPS_ENABLE_DEBUG_PRINT) {
		DEBUG_uart_print_str("EPS->OBC DATA (no tags): ");
		DEBUG_uart_print_array_hex(rx_buf, rx_buf_len);
		DEBUG_uart_print_str("\n");
	}

	// Check STAT field (Table 3-11) - 0x00 and 0x80 mean success
	// TODO: consider doing this check in the next level up
	const uint8_t eps_stat_field = rx_buf[4];
	if ((eps_stat_field != 0x00) && (eps_stat_field != 0x80)) {
		if (GPS_ENABLE_DEBUG_PRINT) {
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