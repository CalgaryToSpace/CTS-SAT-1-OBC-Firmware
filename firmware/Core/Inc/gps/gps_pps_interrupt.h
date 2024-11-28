
#ifndef __INCLUDE_GUARD__GPS_PPS_INTERRUPT_H
#define __INCLUDE_GUARD__GPS_PPS_INTERRUPT_H

#include "stm32l4xx_hal.h"

uint32_t pps_counter;

uint8_t gps_pps_enabled;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

void GPIO_EXTI_Init(void);

void Disable_GPIO_EXTI(void);

uint8_t get_gps_pps_enabled(void);

void set_gps_pps_enabled(void);

void set_gps_pps_disabled(void);

uint32_t get_pps_counter(void);

void reset_pps_counter(void);

#endif // __INCLUDE_GUARD__GPS_PPS_INTERRUPT_H