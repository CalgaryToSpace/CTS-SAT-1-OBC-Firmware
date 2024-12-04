
#ifndef __INCLUDE_GUARD__GPIO_HANDLER_H
#define __INCLUDE_GUARD__GPIO_HANDLER_H

#include "stm32l4xx_hal.h"
#include "main.h"

extern uint32_t GPS_pps_counter;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
#endif
