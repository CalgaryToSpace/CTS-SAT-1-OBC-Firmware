#ifndef INCLUDE_GUARD__STM32_I2C_RECOVERY_H
#define INCLUDE_GUARD__STM32_I2C_RECOVERY_H

#include "stm32l4xx_hal.h"
#include <stdint.h>

void STM32_i2c_reset_if_stuck(I2C_HandleTypeDef *hi2c, uint8_t log_to_uhf_radio);

#endif // INCLUDE_GUARD__STM32_I2C_RECOVERY_H
