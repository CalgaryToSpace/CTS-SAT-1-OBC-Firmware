// debug_uart.h
// Functions for debug printing to the Debug UART interface.

#ifndef __INCLUDE_GUARD__DEBUG_UART_H__
#define __INCLUDE_GUARD__DEBUG_UART_H__

#include "main.h"
#include <stdint.h>

void DEBUG_uart_print_str(const char *str);
void DEBUG_uart_print_uint32(uint32_t value);
void DEBUG_uart_print_int32(int32_t value);
void DEBUG_uart_print_array_hex(uint8_t *array, uint32_t array_len);

#endif // __INCLUDE_GUARD__DEBUG_UART_H__
