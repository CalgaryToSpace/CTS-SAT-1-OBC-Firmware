
#include "main.h"
#include "debug_tools/debug_i2c.h"
#include "debug_tools/debug_uart.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

void DEBUG_uart_print_str(const char *str) {
    HAL_UART_Transmit(&hlpuart1, (uint8_t *)str, strlen(str), 100);
}

void DEBUG_uart_print_uint32(uint32_t value) {
    char buffer[16];
    snprintf(buffer, 16, "%lu", value);
    DEBUG_uart_print_str(buffer);
}

void DEBUG_uart_print_int32(int32_t value) {
    char buffer[16];
    snprintf(buffer, 16, "%ld", value);
    DEBUG_uart_print_str(buffer);
}


void DEBUG_uart_print_array_hex(const uint8_t* arr, uint16_t len, const char* end_str) {
	char msg[10];
	for (uint16_t i = 0; i < len; i++) {
		sprintf(msg, "0x%02x ", arr[i]);
		DEBUG_uart_print_str(msg);
	}

	DEBUG_uart_print_str(end_str);
}
