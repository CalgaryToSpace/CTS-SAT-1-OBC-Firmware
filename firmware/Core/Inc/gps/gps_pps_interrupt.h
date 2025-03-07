
#ifndef __INCLUDE_GUARD__GPS_PPS_INTERRUPT_H
#define __INCLUDE_GUARD__GPS_PPS_INTERRUPT_H

#include "stm32l4xx_hal.h"
#include "gpio_handler/gpio_handler.h"

void GPS_PPS_EXTI_Init(void);

void Disable_GPS_PPS_EXTI(void);

#endif // __INCLUDE_GUARD__GPS_PPS_INTERRUPT_H