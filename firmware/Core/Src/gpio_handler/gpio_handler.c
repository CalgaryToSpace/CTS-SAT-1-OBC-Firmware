
#include "gpio_handler/gpio_handler.h"
uint32_t GPS_pps_counter = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin != PIN_GPS_PPS_IN_Pin) {
        return;
    }
    if (HAL_GPIO_ReadPin(PIN_GPS_PPS_IN_GPIO_Port, PIN_GPS_PPS_IN_Pin) == GPIO_PIN_SET) {
        GPS_pps_counter++;
    }
}