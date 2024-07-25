
#ifndef __INCLUDE_GUARD__RTOS_TASKS_H__
#define __INCLUDE_GUARD__RTOS_TASKS_H__

#include "main.h"
#include "rtos_tasks/rtos_task_helpers.h"

#define DEBUG_HEARTBEAT_TASK_SIZE 256
#define DEFAULT_TASK_SIZE (128 * 4)
#define HANDLE_UART_TASK_SIZE 8192
#define EXECUTE_TELECOMMANDS_TASK_SIZE 8192

void TASK_DEBUG_print_heartbeat(void *argument);

void TASK_handle_uart_telecommands(void *argument);

void TASK_execute_telecommands(void *argument);


#endif // __INCLUDE_GUARD__RTOS_TASKS_H__
