
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

void DEBUG_uart_print_array_hex(uint8_t *array, uint32_t array_len) {
    for (uint32_t i = 0; i < array_len; i++) {
        char buffer[4];
        snprintf(buffer, 4, "%02X ", array[i]);
        DEBUG_uart_print_str(buffer);
    }
}
