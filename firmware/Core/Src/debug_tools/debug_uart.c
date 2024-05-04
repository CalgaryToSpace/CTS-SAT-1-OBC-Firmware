
#include "main.h"
#include "debug_tools/debug_i2c.h"
#include "debug_tools/debug_uart.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

void debug_uart_print_str(const char *str) {
    HAL_UART_Transmit(&hlpuart1, (uint8_t *)str, strlen(str), 100);
}

void debug_uart_print_uint32(uint32_t value) {
    char buffer[16];
    snprintf(buffer, 16, "%lu", value);
    debug_uart_print_str(buffer);
}
