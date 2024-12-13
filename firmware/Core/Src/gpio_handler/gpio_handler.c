
#include "gpio_handler/gpio_handler.h"
#include "log/log.h"
#include "timekeeping/timekeeping.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
uint32_t GPS_pps_counter = 0;
uint64_t GPS_last_pps_time = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (TIM_get_current_unix_epoch_time_ms() - GPS_last_pps_time > 800) {
        GPS_last_pps_time = TIM_get_current_unix_epoch_time_ms();
        if (GPIO_Pin != PIN_GPS_PPS_IN_Pin) {
            return;
        }
        if (HAL_GPIO_ReadPin(PIN_GPS_PPS_IN_GPIO_Port, PIN_GPS_PPS_IN_Pin) == GPIO_PIN_SET) {
            GPS_pps_counter++;
        }
        // char message[100];
        // sprintf(message, "GPS PPS interrupt. Counter: %ld\r\n", GPS_pps_counter);
        // LOG_message(
        //     LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        //     message
        // );
    }
}