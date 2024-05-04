
#include "main.h"
#include "rtos_tasks/rtos_task_helpers.h"
#include "rtos_tasks/rtos_tasks.h"
#include "telecommands/telecommand_parser.h"
#include "debug_tools/debug_uart.h"
#include "cmsis_os.h"

void TASK_debug_print_heartbeat(void *argument) {
	TASK_HELP_start_of_task();

	debug_uart_print_str("TASK_debug_print_heartbeat() - started (booted)\n");
	osDelay(100);
	while (1) {
		debug_uart_print_str("TASK_debug_print_heartbeat() - new\n");
		HAL_GPIO_TogglePin(PIN_DEVKIT_LD2_GPIO_Port, PIN_DEVKIT_LD2_Pin);
		osDelay(1000);
	}
}


void TASK_handle_uart_telecommands(void *argument) {
	TASK_HELP_start_of_task();

	uint8_t rx_telecommand_buffer[512];

	while (1) {
		debug_uart_print_str("TASK_handle_uart_telecommands()\n");
		
		// try reading any available data from the UART
		uint32_t bytes_read = 0;
		while (1) {
			uint8_t byte;
			HAL_StatusTypeDef status = HAL_UART_Receive(&hlpuart1, &byte, 1, 0);
			if (status == HAL_OK) {
				rx_telecommand_buffer[bytes_read] = byte;
				bytes_read++;
			} else {
				break;
			}
		}

		if (bytes_read == 0) {
			debug_uart_print_str("No telecommand received.\n");
			continue;
		}

		debug_uart_print_str("Received telecommand. Bytes: ");
		debug_uart_print_uint32(bytes_read);
		debug_uart_print_str(". Mesages: ");
		debug_uart_print_str((char *)rx_telecommand_buffer);
		debug_uart_print_str("\n");
		
		// process the telecommand
		int32_t tcmd_idx = TCMD_parse_telecommand_get_index((char *)rx_telecommand_buffer, bytes_read);
		debug_uart_print_str("Telecommand index: ");
		debug_uart_print_uint32(tcmd_idx);
		debug_uart_print_str("\n");

		// get the telecommand definition
		if (tcmd_idx >= 0) {
			debug_uart_print_str("Telecommand found: ");
			debug_uart_print_str(TCMD_telecommand_definitions[tcmd_idx].tcmd_name);
			debug_uart_print_str("\n");
		} else {
			debug_uart_print_str("Telecommand not found.\n");
		}

		// handle the telecommand by calling the appropriate function
		// null-terminate the args string
		rx_telecommand_buffer[bytes_read] = '\0';
		TCMD_TelecommandDefinition_t tcmd_def = TCMD_telecommand_definitions[tcmd_idx];
		char response_buf[512];
		tcmd_def.tcmd_func(
			&rx_telecommand_buffer[strlen(tcmd_def.tcmd_name)],
			TCMD_TelecommandChannel_DEBUG_UART,
			response_buf,
			sizeof(response_buf));

		// print back the response
		debug_uart_print_str("Response: ");
		debug_uart_print_str(response_buf);
		debug_uart_print_str("\n");

		osDelay(100);
	}
}
