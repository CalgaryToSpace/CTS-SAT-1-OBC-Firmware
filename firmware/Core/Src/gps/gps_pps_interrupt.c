#include "stm32l4xx_hal.h"
#include "gps/gps_pps_interrupt.h"

#include "main.h"

void GPIO_EXTI_Init(void) {
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0); 
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void Disable_GPIO_EXTI(void) {
    HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
}
