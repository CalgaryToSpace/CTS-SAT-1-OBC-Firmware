#include "stm32l4xx_hal.h"
#include "gps/gps_pps_interrupt.h"

#include "main.h"

uint32_t pps_counter = 0;
volatile uint8_t gps_pps_enabled = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == PIN_GPS_PPS_IN_Pin) {
        if (__HAL_GPIO_EXTI_GET_IT(GPIO_Pin) != RESET) {
            __HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);
            if (HAL_GPIO_ReadPin(PIN_GPS_PPS_IN_GPIO_Port, PIN_GPS_PPS_IN_Pin) == GPIO_PIN_SET && gps_pps_enabled) {
                pps_counter++;
            }
        }
    }
    if (HAL_GPIO_ReadPin(PIN_GPS_PPS_IN_GPIO_Port, PIN_GPS_PPS_IN_Pin) == SET) {
        // GPS power is off, disable interrupts
        Disable_GPIO_EXTI();
    }
}

void GPIO_EXTI_Init(void) {
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0); 
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    gps_pps_enabled = 1;
}

void Disable_GPIO_EXTI(void) {
    HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
    gps_pps_enabled = 0;
}