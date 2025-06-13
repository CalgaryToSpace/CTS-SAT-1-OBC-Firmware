#ifndef INCLUDE_GUARD__UART_HANDLER_H
#define INCLUDE_GUARD__UART_HANDLER_H

#include "stm32l4xx_hal.h"

#include <stdint.h>

uint8_t UART_set_baud_rate(UART_HandleTypeDef *port_handle, uint32_t new_baud_rate);

UART_HandleTypeDef *UART_get_port_handle_by_name(const char port_name[]);

#endif // INCLUDE_GUARD__UART_HANDLER_H
