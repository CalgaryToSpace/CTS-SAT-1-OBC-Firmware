/*
 * GPS_init.h
 *
 *  Created on: June 27, 2023
 *  Author: Matjaz Cigler
 */

#ifndef INC_GPS_INIT_H_
#define INC_GPS_INIT_H_

#include <string.h>
#include "stm32l4xx_hal.h"

extern UART_HandleTypeDef hlpuart1;
extern UART_HandleTypeDef huart3;

void initialize_GPS();

#endif INC_GPS_INIT_H_
