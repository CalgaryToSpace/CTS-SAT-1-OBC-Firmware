
#ifndef __INCLUDE_GUARD__RTOS_TASKS_H__
#define __INCLUDE_GUARD__RTOS_TASKS_H__

#include "main.h"
#include "rtos_tasks/rtos_task_helpers.h"

void TASK_debug_print_heartbeat(void *argument);

void TASK_handle_uart_telecommands(void *argument);


#endif // __INCLUDE_GUARD__RTOS_TASKS_H__
