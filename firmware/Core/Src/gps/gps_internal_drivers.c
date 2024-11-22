#include "main.h"

#include "debug_tools/debug_uart.h"
#include "debug_tools/debug_i2c.h"
#include "gps/gps_internal_drivers.h"
#include "gps/gps_args_helpers.h"
#include "uart_handler/uart_handler.h"
#include "stm_drivers/timing_helpers.h"
#include "uart_handler/uart_handler.h"
#include "log/log.h"

#include <stdint.h>
#include <string.h>

extern UART_HandleTypeDef *UART_gps_port_handle;

const uint32_t GPS_RX_TIMEOUT = 50;

/// @brief Sends a log command to the GPS, and receives the response.
/// @param cmd_buf log command string to send to the GPS.
/// @param cmd_buf_len Exact length of the log command string.
/// @param rx_buf Buffer to store the response.
/// @param rx_buf_len Length of the response buffer.
/// @param rx_buf_max_size Maximum length of the response buffer.
/// @return 0 on success, >0 if error.
uint8_t GPS_send_cmd_get_response(const char *cmd_buf, uint8_t cmd_buf_len, uint8_t rx_buf[], uint16_t rx_buf_len, const uint16_t rx_buf_max_size)
{

	// TX TO GPS
	const HAL_StatusTypeDef tx_status = HAL_UART_Transmit(
		UART_gps_port_handle,
		(uint8_t *)cmd_buf,
		cmd_buf_len,
		GPS_RX_TIMEOUT); // FIXME: update the timeout

	if (tx_status != HAL_OK)
	{
		LOG_message(
			LOG_SYSTEM_GPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
			"GPS ERROR: tx_status != HAL_OK (%d)\n",
			tx_status);

		return 1;
	}

	// Reset the GPS UART interrupt variables
	// UART_gps_uart_interrupt_enabled = 0; // Lock writing to the UART_gps_buffer while we memset it
	// for (uint16_t i = 0; i < UART_gps_buffer_len; i++)
	// {
	// 	// Clear the buffer
	// 	// Can't use memset because UART_gps_buffer is volatile
	// 	UART_gps_buffer[i] = 0;
	// }
	// UART_gps_buffer_write_idx = 0;		 // Make it start writing from the start
	UART_gps_uart_interrupt_enabled = 1; // We are now expecting a response

	// RX FROM GPS, into UART_gps_buffer
	const uint32_t start_rx_time = HAL_GetTick();

	// Checking if GPS data is being populated into the UART_gps_buffer
	if ((UART_gps_buffer_write_idx == 0))
	{
		// Check if we've timed out (before the first byte)
		if ((HAL_GetTick() - start_rx_time) > GPS_RX_TIMEOUT)
		{
			LOG_message(
				LOG_SYSTEM_GPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
				"GPS ERROR: Timeout Error\n");

			// fatal error; return
			return 2;
		}
	}
	else
	{ // thus, UART_gps_buffer_write_idx > 0
		// Check if we've timed out (between bytes)
		const uint32_t cur_time = HAL_GetTick();
		// Note: Sometimes, because ISRs and C are fun, the UART_gps_last_write_time_ms is
		// greater than `cur_time`. Thus, we must do a safety check that the time difference
		// is positive.
		if (
			(cur_time > UART_gps_last_write_time_ms) // Important seemingly-obvious safety check.
			&& ((cur_time - UART_gps_last_write_time_ms) > GPS_RX_TIMEOUT))
		{
			LOG_message(
				LOG_SYSTEM_GPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
				"GPS WARNING: Timeout during receiving of response. UART_gps_last_write_time_ms=%lu, cur_time=%lu",
				UART_gps_last_write_time_ms,
				cur_time);
			// Non-fatal error; try to parse what we received
		}
	}

	// End Receiving
	UART_gps_uart_interrupt_enabled = 0; // We are no longer expecting a response

	// Check that we've received what we're expecting
	// TODO: if the following cases happen ever during testing, consider allowing them and treating them as WARNINGs
	if (UART_gps_buffer_write_idx == 0)
	{
		LOG_message(
			LOG_SYSTEM_GPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
			"GPS ERROR: UART_gps_buffer_write_idx == 0");

		return 3;
	}
	else if (UART_gps_buffer_write_idx > rx_buf_max_size)
	{
		LOG_message(
			LOG_SYSTEM_GPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
			"GPS ERROR: UART_gps_buffer overflow");
		return 4;
	}

	return 0;
}

/// @brief Validates the response of a log command sent to the GPS, based of the reponse tag.
/// Refer to page 1159, table 267 of the OEM7 Commands Logs Manual
/// @param gps_response_buf log command string to send to the GPS.
/// @return 0 on success, > 0 if error.
/// TODO: Implement this function with other functions
uint8_t GPS_validate_log_response(const char *gps_response_buf)
{
	if (gps_response_buf == NULL)
	{
		LOG_message(
			LOG_SYSTEM_GPS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
			"GPS ERROR: GPS response buffer is empty/Null");

		// Error: Empty gps response buffer
		return 1;
	}

	// Find the start and end of the log command response, which is < and [ resepectively
	const char *sync_char = strchr(gps_response_buf, '<');
	const char *delimiter_char = strchr(gps_response_buf, '[');

	if (!sync_char || !delimiter_char)
	{
		// Invalid data: No response tag in gps response
		return 2;
	}

	// Calculate the length of the response tag
	const int response_tag_length = delimiter_char - sync_char + 1;
	if (response_tag_length < 0)
	{
		// Sync character occurs after delimiter character
		return 3;
	}

	char response_tag_buffer[response_tag_length];

	// Copy response tag string into a buffer
	strncpy(response_tag_buffer, sync_char, response_tag_length);
	response_tag_buffer[response_tag_length] = '\0'; // Null-terminate the substring

	if (strcmp(response_tag_buffer, "OK") != 0)
	{
		LOG_message(
			LOG_SYSTEM_GPS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
			"GPS VALIDATION ERROR: GPS response is: %s",
			response_tag_buffer);
		// Error status: Error with the log command/gnss receiver
		return 4;
	}
	else
	{
		LOG_message(
			LOG_SYSTEM_GPS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
			"GPS VALIDATION SUCCESS: GPS response is: %s",
			response_tag_buffer);
	}

	return 0;
}