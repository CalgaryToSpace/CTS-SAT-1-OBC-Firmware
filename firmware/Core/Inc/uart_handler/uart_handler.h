
#ifndef INCLUDE_GUARD__UART_HANDLER_H__
#define INCLUDE_GUARD__UART_HANDLER_H__

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

extern const uint16_t UART_mpi_rx_buffer_len; // Length of the UART MPI response buffer (for telecommand responses only & NOT SCIENCE DATA)
extern volatile uint8_t UART_mpi_rx_buffer[]; // Buffer for UART MPI response
extern volatile uint8_t UART_mpi_rx_last_byte; // Last received byte from the MPI response
extern volatile uint32_t UART_mpi_rx_last_byte_write_time_ms; // Last write time in milliseconds for MPI response
extern volatile uint16_t UART_mpi_rx_buffer_write_idx; // Write index for MPI response buffer

extern const uint16_t UART_gps_buffer_len; 
extern volatile uint8_t UART_gps_buffer[];          
extern volatile uint16_t UART_gps_buffer_write_idx; 
extern volatile uint32_t UART_gps_last_write_time_ms; 
extern volatile uint8_t UART_gps_buffer_last_rx_byte;  
extern volatile uint8_t UART_gps_uart_interrupt_enabled; // Flag to enable or disable the UART GPS ISR


extern const uint16_t UART_camera_buffer_len;
extern volatile uint8_t UART_camera_buffer[];
extern uint8_t camera_rx_buf[];
extern volatile uint8_t camera_write_file;
extern volatile uint16_t UART_camera_buffer_write_idx;
extern volatile uint32_t UART_camera_last_write_time_ms;
extern volatile uint8_t UART_camera_is_expecting_data;
extern volatile uint8_t UART_camera_buffer_last_rx_byte;


void UART_init_uart_handlers(void);
void GPS_set_uart_interrupt_state(uint8_t new_enabled) ;
uint8_t CAMERA_set_expecting_data(uint8_t new_enabled) ;


#endif // INCLUDE_GUARD__UART_HANDLER_H__
