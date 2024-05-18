
#ifndef __INCLUDE_GUARD__UART_HANDLER_H__
#define __INCLUDE_GUARD__UART_HANDLER_H__

#include "main.h"

extern const uint16_t UART_telecommand_buffer_len; // Length of the UART telecommand buffer
extern volatile uint8_t UART_telecommand_buffer[];          // Buffer for UART telecommands
extern volatile uint16_t UART_telecommand_buffer_write_idx; // Write index for UART telecommand buffer
extern volatile uint32_t UART_telecommand_last_write_time_ms; // Last write time in milliseconds for UART telecommand
extern volatile uint8_t UART_telecommand_buffer_last_rx_byte; // Last received byte for UART telecommand


void UART_init_uart_handlers(void);


#endif // __INCLUDE_GUARD__UART_HANDLER_H__
