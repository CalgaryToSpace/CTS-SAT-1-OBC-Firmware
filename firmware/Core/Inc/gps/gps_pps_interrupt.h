
#ifndef __INCLUDE_GUARD__GPS_PPS_INTERRUPT_H
#define __INCLUDE_GUARD__GPS_PPS_INTERRUPT_H

#include "stm32l4xx_hal.h"

extern uint32_t pps_counter;

extern uint8_t gps_pps_enabled;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

void GPIO_EXTI_Init(void);

void Disable_GPIO_EXTI(void);

#endif // __INCLUDE_GUARD__GPS_PPS_INTERRUPT_H