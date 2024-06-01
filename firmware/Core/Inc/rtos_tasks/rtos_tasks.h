
#ifndef __INCLUDE_GUARD__RTOS_TASKS_H__
#define __INCLUDE_GUARD__RTOS_TASKS_H__

#include "main.h"
#include "rtos_tasks/rtos_task_helpers.h"

// TODO: how large should this be?
#define TASK_MAX_ARG_LIST_SIZE 512
#define TASK_OSDELAY_TICKS 40

void TASK_DEBUG_print_heartbeat(void *argument);

void TASK_handle_uart_telecommands(void *argument);


#endif // __INCLUDE_GUARD__RTOS_TASKS_H__
