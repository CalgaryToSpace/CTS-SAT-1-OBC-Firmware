
#include "main.h"
#include "rtos_tasks/rtos_task_helpers.h"
#include "rtos_tasks/rtos_tasks.h"
#include "telecommands/telecommand_parser.h"
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

	uint8_t latest_tcmd[UART_telecommand_buffer_len];
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
			// DEBUG_uart_print_str("UART telecommand received (timeout occurred, buffer has data)\n");
			
			// copy the buffer to the latest_tcmd buffer
			latest_tcmd_len = UART_telecommand_buffer_write_idx;
			// FIXME: rewrite the following memcpy/memset without the casts (which disregards the volatile qualifier)
			memcpy(latest_tcmd, (uint8_t*) UART_telecommand_buffer, UART_telecommand_buffer_len); // copy the whole buffer to ensure nulls get copied too

			// clear the buffer and reset the write pointer
			memset((uint8_t*) UART_telecommand_buffer, 0, UART_telecommand_buffer_len);
			UART_telecommand_buffer_write_idx = 0;
			// TODO: could do it so that it only clears the part of the buffer which contains a command, to allow multiple commands per buffer
		}

		// FIXME: replace this if(1) with a check to see if a full telecommand has been received yet
		// TODO: lots of this should be factored out when we get the comms module going (inside this block, basically)
		if (1) {

			// optionally, echo back the command
			if (latest_tcmd_len > 0) {
				DEBUG_uart_print_str("Received telecommand (len=");
				DEBUG_uart_print_uint32(latest_tcmd_len);
				DEBUG_uart_print_str("): '");
				DEBUG_uart_print_str((char *)latest_tcmd);
				DEBUG_uart_print_str("'\n");
			}
			else {
				// DEBUG_uart_print_str("No telecommand received.\n");
				continue;
			}

			// check that the telecommand starts with the correct prefix
			if (!TCMD_check_starts_with_device_id((char *)latest_tcmd, latest_tcmd_len)) {
				DEBUG_uart_print_str("Telecommand does not start with the correct prefix.\n");
				continue;
			}

			// execute/queue the command
			// process the telecommand name
			int32_t tcmd_idx = TCMD_parse_telecommand_get_index((char *)latest_tcmd, latest_tcmd_len);
			if (tcmd_idx < 0) {
				DEBUG_uart_print_str("Telecommand not found in the list.\n");
				continue;
			}

			// get the telecommand definition
			TCMD_TelecommandDefinition_t tcmd_def = TCMD_telecommand_definitions[tcmd_idx];
			DEBUG_uart_print_str("======= Telecommand ======\n");
			DEBUG_uart_print_str("Received telecommand '");
			DEBUG_uart_print_str(tcmd_def.tcmd_name);
			DEBUG_uart_print_str("'\n");

			// validate the args

			// opening parenthesis index
			int32_t start_of_args_idx = TCMD_PREFIX_STR_LEN + strlen(tcmd_def.tcmd_name);
			if (latest_tcmd_len < start_of_args_idx + 1) {
				DEBUG_uart_print_str("ERROR: You must have parenthesis for the args.\n");
				continue;
			}
			if (latest_tcmd[start_of_args_idx] != '(') {
				DEBUG_uart_print_str("ERROR: You must have parenthesis for the args. You need an opening paren.\n");
				continue;
			}
			
			// closing parenthesis index
			int32_t end_of_args_idx = GEN_get_index_of_substring_in_array((char*)latest_tcmd, latest_tcmd_len, ")");
			if (end_of_args_idx < 0) {
				DEBUG_uart_print_str("ERROR: You must have parenthesis for the args. No closing paren found.\n");
				continue;
			}

			// TODO: maybe log/print args too
			
			const uint16_t arg_len = end_of_args_idx - start_of_args_idx - 1;
			char args_str_no_parens[arg_len + 1];
			memcpy(args_str_no_parens, &latest_tcmd[start_of_args_idx + 1], arg_len);
			args_str_no_parens[arg_len] = '\0';

			// TODO: parse out the @tssent=xx (timestamp sent) and ensure it's not a duplicate
			// TODO: parse out the @tsexec=xx (timestamp to execute at) and see if it should be run immediately or queued (and add a queue system)
			// TODO: add a queue system for telecommands, maybe to flash
			// TODO: read out the @sha256=xx hash and validate it


			// Handle the telecommand by calling the appropriate function.
			// Null-terminate the args string.
			latest_tcmd[latest_tcmd_len] = '\0';
			char response_buf[512];
			memset(response_buf, 0, sizeof(response_buf));
			const uint8_t tcmd_result = tcmd_def.tcmd_func(
				args_str_no_parens,
				TCMD_TelecommandChannel_DEBUG_UART,
				response_buf,
				sizeof(response_buf));

			// print back the response
			DEBUG_uart_print_str("======== Response (err=");
			DEBUG_uart_print_uint32(tcmd_result);
			if (tcmd_result != 0) {
				DEBUG_uart_print_str(" !!!!!! ERROR !!!!!!");
			}
			DEBUG_uart_print_str(") ========\n");
			DEBUG_uart_print_str(response_buf);
			DEBUG_uart_print_str("\n==========================\n");
			
			// TODO: in the future, if the buffer content was longer than the telecommand, we
			//   _could_ shift the remaining bytes to the front of the buffer
		}
		else {
			DEBUG_uart_print_str("Only partial telecommand received. Buffer length: ");
			DEBUG_uart_print_uint32(latest_tcmd_len);
			DEBUG_uart_print_str(" bytes filled.\n");
		}
	} /* End Task's Main Loop */
}

