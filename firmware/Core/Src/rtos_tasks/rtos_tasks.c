
#include "main.h"
#include "rtos_tasks/rtos_tasks.h"
#include "debug_tools/debug_uart.h"
#include "cmsis_os.h"

void TASK_debug_print_heartbeat(void *argument) {
	while (1) {
		debug_uart_print_str("TASK_debug_print_heartbeat()\n");
		HAL_GPIO_TogglePin(PIN_DEVKIT_LD2_GPIO_Port, PIN_DEVKIT_LD2_Pin);
		osDelay(1000);
	}
}
