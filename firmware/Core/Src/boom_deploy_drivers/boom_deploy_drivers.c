
#include "boom_deploy_drivers/boom_deploy_drivers.h"
#include "main.h"
#include <stdint.h>


/// @brief Sets the boom deploy enable pin state (on or off).
/// @param boom_channel_num 1 for channel 1 (BOOM_CTRL_1), 2 for channel 2 (BOOM_CTRL_2).
/// @param enabled 0 to disable, 1 to enable.
/// @return 0 on success, 1 on failure (invalid channel).
uint8_t BOOM_set_burn_enabled(uint8_t boom_channel_num, uint8_t enabled) {
    if (boom_channel_num == 1) {
        HAL_GPIO_WritePin(PIN_BOOM_DEPLOY_EN_1_OUT_GPIO_Port, PIN_BOOM_DEPLOY_EN_1_OUT_Pin, enabled);
    }
    else if (boom_channel_num == 2) {
        HAL_GPIO_WritePin(PIN_BOOM_DEPLOY_EN_2_OUT_GPIO_Port, PIN_BOOM_DEPLOY_EN_2_OUT_Pin, enabled);
    }
    else {
        return 1; // Invalid channel
    }
    return 0;
}

/// @brief Disable all boom deploy enable channels. No EPS change.
void BOOM_disable_all_burns(void) {
    HAL_GPIO_WritePin(PIN_BOOM_DEPLOY_EN_1_OUT_GPIO_Port, PIN_BOOM_DEPLOY_EN_1_OUT_Pin, 0);
    HAL_GPIO_WritePin(PIN_BOOM_DEPLOY_EN_2_OUT_GPIO_Port, PIN_BOOM_DEPLOY_EN_2_OUT_Pin, 0);
}
