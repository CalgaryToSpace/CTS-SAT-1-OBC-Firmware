
#ifndef INCLUDE_GUARD__UART_HANDLER_H__
#define INCLUDE_GUARD__UART_HANDLER_H__

#include "stm32l4xx_hal.h"
#include <stdint.h>

typedef enum {
    CAMERA_UART_WRITE_STATE_IDLE,
    CAMERA_UART_WRITE_STATE_HALF_FILLING,
    CAMERA_UART_WRITE_STATE_HALF_FILLED_WAITING_FS_WRITE,
    CAMERA_UART_WRITE_STATE_HALF_WRITTEN_TO_FS
} CAMERA_uart_write_state_enum_t;

extern volatile CAMERA_uart_write_state_enum_t CAMERA_uart_half_1_state;
extern volatile CAMERA_uart_write_state_enum_t CAMERA_uart_half_2_state;

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

extern const uint16_t UART_gps_buffer_len;                      // Length of the GPS response buffer
extern volatile uint8_t UART_gps_buffer[];                      // Buffer for GPS response
extern volatile uint16_t UART_gps_buffer_write_idx;             // Write index for GPS response buffer
extern volatile uint32_t UART_gps_last_write_time_ms;           // Last write time in milliseconds for GPS response

extern const uint16_t UART_camera_dma_buffer_len;                   // Length of the CAMERA response buffer
extern const uint16_t UART_camera_dma_buffer_len_half;              // Half length of the CAMERA response buffer
extern volatile uint8_t UART_camera_dma_buffer[];               // Buffer for CAMERA response
extern volatile uint8_t UART_camera_pending_fs_write_half_1_buf[];
extern volatile uint8_t UART_camera_pending_fs_write_half_2_buf[];
extern volatile uint32_t UART_camera_last_write_time_ms;        // Last write time in milliseconds for CAMERA response
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
uint8_t CAMERA_set_expecting_data(uint8_t new_enabled) ;

#endif // INCLUDE_GUARD__UART_HANDLER_H__
