#ifndef INCLUDE_GUARD__UART_ERROR_TRACKING_H
#define INCLUDE_GUARD__UART_ERROR_TRACKING_H

#include <stdint.h>
#include "stm32l4xx_hal.h" // Need to include this before including line below
#include "stm32l4xx_hal_uart.h"

#define UART_ERROR_TRACKING_JSON_BUFFER_LEN 768 // 512 + 256

/// @brief These will be an error defined in the stm32l4xx_hal_uart.h file.
/// @note UART_Error_Definition: stm32l4xx_hal_uart.h
/// @note Not accounting for: HAL_UART_ERROR_NONE as there is no point
typedef struct
{
    uint16_t parity_error_count; // HAL_UART_ERROR_PE
    uint16_t noise_error_count; // HAL_UART_ERROR_NE
    uint16_t frame_error_count; // HAL_UART_ERROR_FE
    uint16_t overrun_error_count; // HAL_UART_ERROR_ORE
    uint16_t dma_transfer_error_count; // HAL_UART_ERROR_DMA
    uint16_t receiver_timeout_error_count; // HAL_UART_ERROR_RTO
    uint16_t handler_buffer_full_error_count; // Custom field to track if buffer in ISR handler becomes full
} UART_error_counts_single_subsystem_struct_t;

extern UART_error_counts_single_subsystem_struct_t UART_error_mpi_error_info;
extern UART_error_counts_single_subsystem_struct_t UART_error_ax100_error_info;

extern UART_error_counts_single_subsystem_struct_t UART_error_gnss_error_info;

extern UART_error_counts_single_subsystem_struct_t UART_error_camera_error_info;

extern UART_error_counts_single_subsystem_struct_t UART_error_eps_error_info;

extern UART_error_counts_single_subsystem_struct_t UART_error_telecommand_error_info;


void UART_track_error_from_isr(USART_TypeDef *huart_instance, uint32_t error_code);

uint8_t UART_single_subsystem_error_info_to_json(UART_error_counts_single_subsystem_struct_t *error_info_struct, char *json_buffer, uint16_t json_buffer_len);

uint8_t UART_get_errors_json(char * json_buf, uint16_t json_buf_len);
#endif // INCLUDE_GUARD__UART_ERROR_TRACKING_H