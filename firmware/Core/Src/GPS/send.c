/*
 * send.c
 *
 *
 *  Created on: Jul 7, 2022
 *  Updated: 2023-11-21
 *      Author: mswel, Matjaz Cigler
 *  Updated on 2024-04-23
 *  Author: Matjaz Cigler
 */

#include "../../Inc/GPS/send.h"
// #include "../../Drivers/STM32L4xx_HAL_Driver/Inc/stm32l4xx_hal_uart.h"
// #include "stm32l4xx_hal_def.h"

void sendGpsCommand(char *command)
{
	/**
	 * Okay let's do this one last time
	 *
	 * method input is string of command
	 *
	 * Need to transmit command onto USART 3 port
	 */

	// first command string will be transmitted to virtual com selected on the UART_HandleTypeDef
	// that is passed to verify it was able to send
	HAL_UART_Transmit(&huart3, (uint8_t *)command, strlen(command), HAL_MAX_DELAY);

	// use this once code is complete and ready to test with the rasberry pi
	//\HAL_UART_Transmit(&huart3, (uint8_t*)command, strlen(command), HAL_MAX_DELAY)`;
}
