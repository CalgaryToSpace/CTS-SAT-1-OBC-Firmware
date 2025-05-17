#ifndef INCLUDE_GUARD__RTOS_TCMD_RECEIVE_DEBUG_UART_TASK_H
#define INCLUDE_GUARD__RTOS_TCMD_RECEIVE_DEBUG_UART_TASK_H

#include <stdint.h>

extern uint32_t AX100_uptime_at_last_received_kiss_tcmd_ms;

void TASK_handle_uart_telecommands(void *argument);

void TASK_handle_ax100_kiss_telecommands(void *argument);


#endif // INCLUDE_GUARD__RTOS_TCMD_RECEIVE_DEBUG_UART_TASK_H
