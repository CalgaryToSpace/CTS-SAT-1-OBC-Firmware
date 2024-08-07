
#ifndef __INCLUDE_GUARD__UART_HANDLER_H__
#define __INCLUDE_GUARD__UART_HANDLER_H__

#include <stdint.h>

extern const uint16_t UART_telecommand_buffer_len; // Length of the UART telecommand buffer
extern volatile uint8_t UART_telecommand_buffer[]; // Buffer for UART telecommands
extern volatile uint16_t UART_telecommand_buffer_write_idx; // Write index for UART telecommand buffer
extern volatile uint32_t UART_telecommand_last_write_time_ms; // Last write time in milliseconds for UART telecommand

extern const uint16_t UART_eps_buffer_len;
extern volatile uint8_t UART_eps_buffer[];
extern volatile uint16_t UART_eps_buffer_write_idx;
extern volatile uint32_t UART_eps_last_write_time_ms;
extern volatile uint8_t UART_eps_is_expecting_data; // Set to 1 when a command is sent, and we're awaiting a response

extern const uint16_t UART_gps_buffer_len; // Length of the UART telecommand buffer
extern volatile uint8_t UART_gps_buffer[];          // Buffer for UART telecommands
extern volatile uint16_t UART_gps_buffer_write_idx; // Write index for UART telecommand buffer
extern volatile uint32_t UART_gps_last_write_time_ms; // Last write time in milliseconds for UART telecommand
extern volatile uint8_t UART_gps_buffer_last_rx_byte; // Last received byte for UART telecommand


void UART_init_uart_handlers(void);


#endif // __INCLUDE_GUARD__UART_HANDLER_H__
