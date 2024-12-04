#include "stm32l4xx_hal.h"
#include "gps/gps_pps_interrupt.h"

#include "main.h"

uint32_t GPS_pps_counter = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin != PIN_GPS_PPS_IN_Pin) {
        return;
    }
    if (HAL_GPIO_ReadPin(PIN_GPS_PPS_IN_GPIO_Port, PIN_GPS_PPS_IN_Pin) == GPIO_PIN_SET) {
        GPS_pps_counter++;
    }
}

void GPIO_EXTI_Init(void) {
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0); 
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void Disable_GPIO_EXTI(void) {
    HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
}
