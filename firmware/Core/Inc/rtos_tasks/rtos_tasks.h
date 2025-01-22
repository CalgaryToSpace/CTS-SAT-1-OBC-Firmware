
#ifndef INCLUDE_GUARD__RTOS_TASKS_H__
#define INCLUDE_GUARD__RTOS_TASKS_H__

#include "main.h"
#include "rtos_tasks/rtos_task_helpers.h"

#define TCMD_MAX_RESPONSE_BUFFER_LENGTH 2048

void TASK_DEBUG_print_heartbeat(void *argument);

void TASK_handle_uart_telecommands(void *argument);

void TASK_execute_telecommands(void *argument);

void TASK_monitor_freertos_memory(void *argument);

#endif // INCLUDE_GUARD__RTOS_TASKS_H__
