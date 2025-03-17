#include "stm32l4xx_hal.h"
#include "gps/gps_pps_interrupt.h"

#include "main.h"

void GPS_PPS_EXTI_Init(void) {
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0); 
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void Disable_GPS_PPS_EXTI(void) {
    GPS_pps_counter = 0;
    HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
}