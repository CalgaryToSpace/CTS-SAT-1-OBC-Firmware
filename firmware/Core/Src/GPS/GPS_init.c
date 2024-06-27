/*
 * GPS_init.c
 *
 *  Created on: June 27, 2023
 *  Author: Matjaz Cigler
 */

#include "../../Inc/GPS/send.h"
// #include "../../Drivers/STM32L4xx_HAL_Driver/Inc/stm32l4xx_hal_uart.h"
// #include "stm32l4xx_hal_def.h"

void initialize_GPS()
{
	uint8_t test[] = "GPS Test initialized";
	while (1)
	{
		HAL_UART_Transmit(&hlpuart1, test, strlen((char *)test), HAL_MAX_DELAY);
	}
}
