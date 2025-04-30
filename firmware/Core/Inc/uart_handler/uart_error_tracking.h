#ifndef INCLUDE_GUARD__UART_ERROR_TRACKING_H
#define INCLUDE_GUARD__UART_ERROR_TRACKING_H

#include <stdint.h>
#include "stm32l4xx_hal_uart.h"

typedef struct {
    /// @brief These will be an error defined in the stm32l4xx_hal_uart.h file.
    /// @note UART_Error_Definition: stm32l4xx_hal_uart.h
    
    uint8_t parity_error_count; // HAL_UART_ERROR_PE
    uint8_t noise_error_count; // HAL_UART_ERROR_NE
    uint8_t frame_error_count; // HAL_UART_ERROR_FE
    uint8_t overrun_error_count; // HAL_UART_ERROR_ORE
    uint8_t dma_transfer_error_count; // HAL_UART_ERROR_DMA
    uint8_t receiver_timeout_error_count; // HAL_UART_ERROR_RTO

    /// @brief The UART port that generated the error.
    /// @note This is the same as the huart->Instance 
    USART_TypeDef *uart_instance;
    /// @brief The time when the error occurred.
    uint32_t timestamp;
} UART_Error_Info_t;

UART_Error_Info_t UART_mpi_error_info = {
    .error_code = 0,
    .uart_instance = NULL,
    .timestamp = 0
};

UART_Error_Info_t UART_gps_error_info = {
    .error_code = 0,
    .uart_instance = NULL,
    .timestamp = 0
};

UART_Error_Info_t UART_camera_error_info = {
    .error_code = 0,
    .uart_instance = NULL,
    .timestamp = 0
};

UART_Error_Info_t UART_eps_error_info = {
    .error_code = 0,
    .uart_instance = NULL,
    .timestamp = 0
};

UART_Error_Info_t UART_telecommand_error_info = {
    .error_code = 0,
    .uart_instance = NULL,
    .timestamp = 0
};

#endif // INCLUDE_GUARD__UART_ERROR_TRACKING_H