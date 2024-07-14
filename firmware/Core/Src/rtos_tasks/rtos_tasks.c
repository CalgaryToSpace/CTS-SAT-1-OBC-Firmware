
#include "main.h"
#include "rtos_tasks/rtos_task_helpers.h"
#include "rtos_tasks/rtos_tasks.h"
#include "telecommands/telecommand_parser.h"
#include "telecommands/telecommand_executor.h"
#include "debug_tools/debug_uart.h"
#include "timekeeping/timekeeping.h"
#include "uart_handler/uart_handler.h"
#include "transforms/arrays.h"

#include "cmsis_os.h"

#include <string.h>
#include <stdio.h>
#include <time.h>

volatile uint8_t TASK_heartbeat_is_on = 1;

char TASK_heartbeat_timing_str[64] = {0};

void TASK_DEBUG_print_heartbeat(void *argument) {
	TASK_HELP_start_of_task();

	DEBUG_uart_print_str("TASK_DEBUG_print_heartbeat() -> started (booted)\n");
	osDelay(100);

    uint64_t unix_time_ms = 0;
    time_t seconds = 0;
    uint16_t ms = 0;
    struct tm *time_info;

	while (1) {
        if (TASK_heartbeat_is_on) {
            unix_time_ms = TIM_get_current_unix_epoch_time_ms();
            seconds = (time_t)(unix_time_ms/ 1000U);
            ms = unix_time_ms - 1000U * seconds;
            time_info = gmtime(&seconds);
            snprintf(TASK_heartbeat_timing_str, 64, "FrontierSat time: %d%02d%02dT%02d:%02d:%02d.%03u\n", time_info->tm_year + 1900, time_info->tm_mon + 1, time_info->tm_mday, time_info->tm_hour, time_info->tm_min, time_info->tm_sec, ms);
            DEBUG_uart_print_str(TASK_heartbeat_timing_str);
		}
		HAL_GPIO_TogglePin(PIN_LED_DEVKIT_LD2_GPIO_Port, PIN_LED_DEVKIT_LD2_Pin);
		osDelay(990);
	}
}


void TASK_handle_uart_telecommands(void *argument) {
	TASK_HELP_start_of_task();

	// CONFIGURATION PARAMETER
	uint32_t timeout_duration_ms = 100;

	char latest_tcmd[UART_telecommand_buffer_len];
	uint16_t latest_tcmd_len = 0;

	while (1) {
		// place the main delay at the top to avoid a "continue" statement skipping it
		osDelay(400);

		// DEBUG_uart_print_str("TASK_handle_uart_telecommands -> top of while(1)\n");

		memset(latest_tcmd, 0, UART_telecommand_buffer_len);
		latest_tcmd_len = 0; // 0 means no telecommand available

		// log the status
		// char msg[256];
		// snprintf(msg, sizeof(msg), "UART telecommand buffer: write_index=%d, last_time=%lums\n", UART_telecommand_buffer_write_idx, UART_telecommand_last_write_time_ms);
		// DEBUG_uart_print_str(msg);

		if ((HAL_GetTick() - UART_telecommand_last_write_time_ms > timeout_duration_ms) && (UART_telecommand_buffer_write_idx > 0)) {
			// Copy the buffer to the latest_tcmd buffer.
			latest_tcmd_len = UART_telecommand_buffer_write_idx;
			
			// MEMCPY, but with volatile-compatible casts.
			// Copy the whole buffer to ensure nulls get copied too.
			for (uint16_t i = 0; i < UART_telecommand_buffer_len; i++) {
				latest_tcmd[i] = (char) UART_telecommand_buffer[i];
			}

			// Set the null terminator at the end of the `latest_tcmd` str.
			latest_tcmd[latest_tcmd_len] = '\0';

			// Clear the buffer (memset to 0, but volatile-compatible) and reset the write pointer.
			for (uint16_t i = 0; i < UART_telecommand_buffer_len; i++) {
				UART_telecommand_buffer[i] = 0;
			}
			UART_telecommand_buffer_write_idx = 0;
			// TODO: could do it so that it only clears the part of the buffer which contains a command, to allow multiple commands per buffer
		}

		if (latest_tcmd_len == 0) {
			continue;
		}

		DEBUG_uart_print_str("============= UART Telecommand Received =============\n");
		DEBUG_uart_print_str(latest_tcmd);
		DEBUG_uart_print_str("\n=====================================================\n");

		// TODO: move all the following code into a `TCMD_parse_schedule_execute_telecommand` function
		// TODO: consider moving the "execute now" logic to a separate task, and still queue the telecommands for immediate execution

		// Parse the telecommand
		TCMD_parsed_tcmd_to_execute_t parsed_tcmd;
		uint8_t parse_result = TCMD_parse_full_telecommand(
			latest_tcmd, TCMD_TelecommandChannel_DEBUG_UART, &parsed_tcmd
		);
		if (parse_result != 0) {
			DEBUG_uart_print_str("Error parsing telecommand: ");
			DEBUG_uart_print_uint32(parse_result);
			DEBUG_uart_print_str("\n");
			continue;
		}

		// FIXME: check if the telecommand is in the 'tssent' table, and skip it if it is (to prevent replays).

		// If applicable, schedule the telecommand for later execution.
		if (parsed_tcmd.timestamp_to_execute > TIM_get_current_unix_epoch_time_ms()) {
			// TODO: schedule
			DEBUG_uart_print_str("Telecommand is scheduled for later execution (at: ");
			DEBUG_uart_print_uint64(parsed_tcmd.timestamp_to_execute);
			DEBUG_uart_print_str(" ms)\n");
			continue;
		}

		// Execute the telecommand
		char response_output_buf[512] = {0};
		TCMD_execute_parsed_telecommand_now(
			parsed_tcmd.tcmd_idx,
			parsed_tcmd.args_str_no_parens,
			parsed_tcmd.tcmd_channel,
			response_output_buf,
			sizeof(response_output_buf)
		);
	
	} /* End Task's Main Loop */
}

