
#include "main.h"
#include "debug_tools/debug_i2c.h"
#include "debug_tools/debug_uart.h"
#include "transforms/arrays.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>  // For isprint()


void DEBUG_uart_print_str(const char *str) {
    HAL_UART_Transmit(&hlpuart1, (uint8_t *)str, strlen(str), 300);
}

void DEBUG_uart_print_uint32(uint32_t value) {
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%lu", value);
    DEBUG_uart_print_str(buffer);
}

void DEBUG_uart_print_int32(int32_t value) {
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%ld", value);
    DEBUG_uart_print_str(buffer);
}

void DEBUG_uart_print_uint64(uint64_t value) {
    char buffer[32];
    
    // Can't use snprintf for uint64
    GEN_uint64_to_str(value, buffer);
    DEBUG_uart_print_str(buffer);
}

/// @brief Print an array of bytes in hex format, like `00 01 02 ...`
/// @param array A pointer to the array of bytes.
/// @param array_len The length of the array.
void DEBUG_uart_print_array_hex(const uint8_t *array, uint32_t array_len) {
    for (uint32_t i = 0; i < array_len; i++) {
        char buffer[4];
        snprintf(buffer, 4, "%02X ", array[i]);
        DEBUG_uart_print_str(buffer);
    }
}

void DEBUG_uart_write_bytes(const uint8_t *data, uint32_t data_len) {
    HAL_UART_Transmit(&hlpuart1, (uint8_t *)data, data_len, 300);
}

/// @brief Print array in hex + ASCII format for UART debugging, aligned.
/// @param array Pointer to the byte array.
/// @param array_len Length of the array.
/// @param prefix Prefix string to prepend to the hex line (e.g., "AX100 Down")
void DEBUG_uart_print_mixed_array(const uint8_t *array, uint16_t array_len, const char *prefix) {
    char line[64];

    // Print hex prefix
    snprintf(line, sizeof(line), "%s (%dB, hex): ", prefix, array_len);
    DEBUG_uart_print_str(line);

    // Print hex values directly
    for (uint32_t i = 0; i < array_len; i++) {
        snprintf(line, sizeof(line), "%02X ", array[i]);
        DEBUG_uart_print_str(line);
    }
    DEBUG_uart_print_str("\n");

    // Print ASCII prefix
    snprintf(line, sizeof(line), "%s (%dB, raw): ", prefix, array_len);
    DEBUG_uart_print_str(line);

    // Print ASCII representation directly
    for (uint32_t i = 0; i < array_len; i++) {
        char ch = (isprint(array[i]) && array[i] != '\n') ? array[i] : ' ';
        snprintf(line, sizeof(line), " %c ", ch);
        DEBUG_uart_print_str(line);
    }
    DEBUG_uart_print_str("\n");
}
