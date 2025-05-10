
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

void DEBUG_uart_print_str_max_len(const char* str, size_t n) {
    char buf[n+1];
    strncpy(buf, str, n);
    buf[n] = '\0'; // Null-terminate the string
    DEBUG_uart_print_str(buf);
    DEBUG_uart_print_str("\n");
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
    char hex_line[3 * 256 + 1];     // Each byte: 2 hex digits + 1 space
    char ascii_line[3 * 256 + 1];   // Aligned: 2 spaces + 1 ASCII char per byte
    uint32_t hex_index = 0;
    int32_t ascii_index = 0;

    for (uint32_t i = 0; i < array_len; i++) {
        // Write hex part
        snprintf(&hex_line[hex_index], 4, "%02X ", array[i]);
        hex_index += 3;

        // Write aligned ASCII part: 2 spaces + character/dot
        ascii_line[ascii_index++] = ' ';
        ascii_line[ascii_index++] = (isprint(array[i]) && (array[i] != '\n')) ? array[i] : ' ';
        ascii_line[ascii_index++] = ' ';
    }

    // Null-terminate both strings
    hex_line[hex_index] = '\0';
    ascii_line[ascii_index] = '\0';

    // Add prefix for hex:
    char prefixed_hex_line[100];
    snprintf(prefixed_hex_line, sizeof(prefixed_hex_line), "%s (%dB, hex): ", prefix, array_len);
    DEBUG_uart_print_str(prefixed_hex_line);

    // Output over UART
    DEBUG_uart_print_str(hex_line);
    DEBUG_uart_print_str("\n");


    snprintf(prefixed_hex_line, sizeof(prefixed_hex_line), "%s (%dB, raw): ", prefix, array_len);
    DEBUG_uart_print_str(prefixed_hex_line);
    DEBUG_uart_print_str(ascii_line);
    DEBUG_uart_print_str("\n");
}
