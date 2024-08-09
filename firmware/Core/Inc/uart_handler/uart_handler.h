
#ifndef __INCLUDE_GUARD__UART_HANDLER_H__
#define __INCLUDE_GUARD__UART_HANDLER_H__

#include "main.h"

extern const uint16_t UART_telecommand_buffer_len; // Length of the UART telecommand buffer
extern volatile uint8_t UART_telecommand_buffer[]; // Buffer for UART telecommands
extern volatile uint16_t UART_telecommand_buffer_write_idx; // Write index for UART telecommand buffer
extern volatile uint32_t UART_telecommand_last_write_time_ms; // Last write time in milliseconds for UART telecommand
extern volatile uint8_t UART_telecommand_buffer_last_rx_byte; // Last received byte for UART telecommand

// UART MPI cmd response buffer
extern const uint16_t UART_mpi_rx_buffer_len; // Length of the UART MPI response buffer
extern volatile uint8_t UART_mpi_rx_buffer[]; // Buffer for UART MPI response
extern volatile uint8_t UART_mpi_rx_last_byte; // Last received byte from the MPI response
extern volatile uint32_t UART_mpi_rx_last_byte_write_time_ms; // Last write time in milliseconds for MPI response
extern volatile uint16_t UART_mpi_rx_buffer_write_idx; // Write index for MPI response

void UART_init_uart_handlers(void);


#endif // __INCLUDE_GUARD__UART_HANDLER_H__
