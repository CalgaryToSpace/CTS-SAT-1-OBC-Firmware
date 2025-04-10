
#ifndef INCLUDE_GUARD__UART_HANDLER_H__
#define INCLUDE_GUARD__UART_HANDLER_H__

#include "stm32l4xx_hal.h"
#include <stdint.h>

// Name the UART interfaces
extern UART_HandleTypeDef *UART_telecommand_port_handle;  
extern UART_HandleTypeDef *UART_mpi_port_handle;                
extern UART_HandleTypeDef *UART_gps_port_handle;
extern UART_HandleTypeDef *UART_camera_port_handle;
extern UART_HandleTypeDef *UART_eps_port_handle;

extern const uint16_t UART_telecommand_buffer_len;              // Length of the UART telecommand buffer
extern volatile uint8_t UART_telecommand_buffer[];              // Buffer for UART telecommands
extern volatile uint16_t UART_telecommand_buffer_write_idx;     // Write index for UART telecommand buffer
extern volatile uint32_t UART_telecommand_last_write_time_ms;   // Last write time in milliseconds for UART telecommand

extern const uint16_t UART_mpi_buffer_len;                      // Length of the MPI response buffer (for telecommand responses only & NOT SCIENCE DATA)
extern volatile uint8_t UART_mpi_buffer[];                      // Buffer for MPI response
extern volatile uint16_t UART_mpi_buffer_write_idx;             // Write index for MPI response buffer
extern volatile uint32_t UART_mpi_last_write_time_ms;           // Last write time in milliseconds for MPI response
extern volatile uint8_t UART_mpi_last_rx_byte;                  // Last received byte from the MPI response

extern const uint16_t UART_lora_buffer_len;                     // Length of the LORA response buffer
extern volatile uint8_t UART_lora_buffer[];                     // Buffer for LORA response
extern volatile uint16_t UART_lora_buffer_write_idx;            // Write index for LORA response buffer
extern volatile uint32_t UART_lora_last_write_time_ms;          // Last write time in milliseconds for LORA response
extern volatile uint8_t UART_lora_is_expecting_data;            // Set to 1 when a data is sent, and we're awaiting a response
extern volatile uint8_t UART_lora_buffer_last_rx_byte;          // Last received byte for LORA response

extern const uint16_t UART_gps_buffer_len;                      // Length of the GPS response buffer
extern volatile uint8_t UART_gps_buffer[];                      // Buffer for GPS response
extern volatile uint16_t UART_gps_buffer_write_idx;             // Write index for GPS response buffer
extern volatile uint32_t UART_gps_last_write_time_ms;           // Last write time in milliseconds for GPS response

extern const uint16_t UART_camera_buffer_len;                   // Length of the CAMERA response buffer
extern volatile uint8_t UART_camera_buffer[];                   // Buffer for CAMERA response
extern volatile uint16_t UART_camera_buffer_write_idx;          // Write index for CAMERA response buffer
extern volatile uint32_t UART_camera_last_write_time_ms;        // Last write time in milliseconds for CAMERA response
extern volatile uint8_t UART_camera_is_expecting_data;          // Set to 1 when a data is sent, and we're awaiting a response
extern volatile uint8_t UART_camera_buffer_last_rx_byte;        // Last received byte for CAMERA response

extern const uint16_t UART_eps_buffer_len;                      // Length of the EPS response buffer
extern volatile uint8_t UART_eps_buffer[];                      // Buffer for EPS response
extern volatile uint16_t UART_eps_buffer_write_idx;             // Write index for EPS response buffer
extern volatile uint32_t UART_eps_last_write_time_ms;           // Last write time in milliseconds for EPS response
extern volatile uint8_t UART_eps_is_expecting_data;             // Set to 1 when a command is sent, and we're awaiting a response

extern const uint16_t UART_gps_buffer_len; 
extern volatile uint8_t UART_gps_buffer[];          
extern volatile uint16_t UART_gps_buffer_write_idx; 
extern volatile uint32_t UART_gps_last_write_time_ms; 
extern volatile uint8_t UART_gps_buffer_last_rx_byte;  
extern volatile uint8_t UART_gps_uart_interrupt_enabled; // Flag to enable or disable the UART GPS ISR

void UART_init_uart_handlers(void);
void GPS_set_uart_interrupt_state(uint8_t new_enabled) ;


#endif // INCLUDE_GUARD__UART_HANDLER_H__
