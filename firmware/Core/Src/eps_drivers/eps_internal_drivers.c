
#include "main.h"

#include "debug_tools/debug_uart.h"
#include "debug_tools/debug_i2c.h"
#include "eps_drivers/eps_types.h"
#include "eps_drivers/eps_internal_drivers.h"
#include "uart_handler/uart_handler.h"
#include "stm_drivers/timing_helpers.h"

#include <stdint.h>
#include <string.h>

extern UART_HandleTypeDef *UART_eps_port_handle;

const uint32_t EPS_RX_TIMEOUT_MS = 200; // TODO: decrease for flight

/// @brief Sends a command to the EPS, and receives the response.
/// @param cmd_buf Array of bytes to send to the EPS, including the command code, STID, IVID, etc.
/// @param cmd_buf_len Exact length of the command buffer.
/// @param rx_buf Buffer to store the response. Is filled with the response, without tags.
/// @param rx_buf_len Length of the response buffer. Must be the command length.
/// @return 0 on success, >0 if error.
uint8_t eps_send_cmd_get_response(
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

	if (EPS_ENABLE_DEBUG_PRINT) {
		DEBUG_uart_print_str("OBC->EPS (no tags): ");
		DEBUG_uart_print_array_hex(cmd_buf, cmd_buf_len, "\n");
		DEBUG_uart_print_str("OBC->EPS (with tags): ");
		DEBUG_uart_print_array_hex(cmd_buf_with_tags, cmd_buf_with_tags_len, "\n");
	}

	// TX TO EPS
	HAL_StatusTypeDef tx_status = HAL_UART_Transmit(
		UART_eps_port_handle,
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
	uint32_t start_rx_time = HAL_GetTick();
	while (1) {
		// Check if we've received the expected number of bytes
		if (UART_eps_buffer_write_idx >= rx_buf_len) {
			// Best-case: we've received the expected number of bytes, and can be done!
			break;
		}

		// Check if we've timed out
		if ((HAL_GetTick() - start_rx_time) > EPS_RX_TIMEOUT_MS) {
			if (EPS_ENABLE_DEBUG_PRINT) {
				char msg[200];
				snprintf(
					msg, sizeof(msg),
					"OBC -> EPS TIMEOUT ERROR: HAL_GetTick() - start_rx_time > EPS_RX_TIMEOUT_MS\n");
				DEBUG_uart_print_str(msg);
			}
			break;
		}

		// TODO: Consider a short timeout after we've received the first byte, but before we've received the last expected byte, between received bytes (not that valuable)

		// TODO: also end when we receive the end tag
	}

	// End Receiving
	UART_eps_is_expecting_data = 0; // We are no longer expecting a response

	// For now, log the received bytes
	DEBUG_uart_print_str("EPS->OBC (with tags): ");
	DEBUG_uart_print_array_hex((uint8_t*)UART_eps_buffer, UART_eps_buffer_write_idx, "");
	DEBUG_uart_print_str("\n");

	// Check that we've received what we're expecting
	// TODO: if the following cases happen ever during testing, consider allowing them and treating them as WARNINGs
	if (UART_eps_buffer_write_idx == 0) {
		if (EPS_ENABLE_DEBUG_PRINT) {
			DEBUG_uart_print_str("OBC -> EPS ERROR: UART_eps_buffer_write_idx == 0\n");
		}
		return 3;
	}
	else if (UART_eps_buffer_write_idx < begin_tag_len + EPS_DEFAULT_RX_LEN_MIN + end_tag_len) {
		if (EPS_ENABLE_DEBUG_PRINT) {
			DEBUG_uart_print_str("OBC -> EPS ERROR: UART_eps_buffer_write_idx < expected shortest RX\n");
		}
		return 4;
	}
	else if (UART_eps_buffer_write_idx < rx_len_with_tags) {
		if (EPS_ENABLE_DEBUG_PRINT) {
			DEBUG_uart_print_str("OBC -> EPS ERROR: UART_eps_buffer_write_idx < rx_len_with_tags\n");
		}
		return 5;
	}
	else if (UART_eps_buffer_write_idx > rx_len_with_tags) {
		if (EPS_ENABLE_DEBUG_PRINT) {
			DEBUG_uart_print_str("OBC -> EPS ERROR: UART_eps_buffer_write_idx > rx_len_with_tags\n");
		}
		return 6;
	}

	// FIXME: pack the rx_buf less-naively (check/confirm the start tag location)

	// Copy the received bytes into the rx_buf.
	// Can't use memcpy because UART_eps_buffer is volatile.
	for (uint16_t i = 0; i < rx_buf_len; i++) {
		rx_buf[i] = UART_eps_buffer[begin_tag_len + i];
	}

	if (EPS_ENABLE_DEBUG_PRINT) {
		DEBUG_uart_print_str("EPS->OBC (no tags): ");
		DEBUG_uart_print_array_hex(rx_buf, rx_buf_len, "\n");
	}

	// Check STAT field (Table 3-11) - 0x00 and 0x80 mean success
	// TODO: consider doing this check in the next level up
	int8_t eps_stat_field = rx_buf[4];
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

	const uint8_t comms_err = eps_send_cmd_get_response(cmd_buf, cmd_len, rx_buf, rx_len);
	return comms_err;
}
