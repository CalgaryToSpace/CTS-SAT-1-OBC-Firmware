
#include "main.h"
#include "debug_tools/debug_i2c.h"
#include "debug_tools/debug_uart.h"

#include <stdint.h>
#include <string.h>

void debug_uart_print_str(const char *str) {
    HAL_UART_Transmit(&hlpuart1, (uint8_t *)str, strlen(str), 100);
}

