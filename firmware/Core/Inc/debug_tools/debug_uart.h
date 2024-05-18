// debug_uart.h
// Functions for debug printing to the Debug UART interface.

#ifndef __INCLUDE_GUARD__DEBUG_UART_H__
#define __INCLUDE_GUARD__DEBUG_UART_H__

#include "main.h"
#include <stdint.h>

void debug_uart_print_str(const char *str);
void debug_uart_print_uint32(uint32_t value);
void debug_uart_print_int32(int32_t value);


#endif // __INCLUDE_GUARD__DEBUG_UART_H__
