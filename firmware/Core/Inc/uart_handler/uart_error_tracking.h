#ifndef INCLUDE_GUARD__UART_ERROR_TRACKING_H
#define INCLUDE_GUARD__UART_ERROR_TRACKING_H

#include <stdint.h>
#include "stm32l4xx_hal.h" // Need to include this before including line below
#include "stm32l4xx_hal_uart.h"

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
} UART_Error_Info_t;

extern UART_Error_Info_t UART_Error_mpi_error_info;

extern UART_Error_Info_t UART_Error_gps_error_info;

extern UART_Error_Info_t UART_Error_camera_error_info;

extern UART_Error_Info_t UART_Error_eps_error_info;


void UART_Error_tracking(USART_TypeDef *huart_instance, uint32_t error_code);
uint8_t UART_Error_tracking_get_tracking_struct_from_uart_instance(USART_TypeDef *huart_instance, UART_Error_Info_t *result_error_info_struct);

#endif // INCLUDE_GUARD__UART_ERROR_TRACKING_H