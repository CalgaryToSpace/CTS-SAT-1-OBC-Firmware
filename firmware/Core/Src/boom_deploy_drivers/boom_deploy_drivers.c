
#include "boom_deploy_drivers/boom_deploy_drivers.h"
#include "main.h"
#include <stdint.h>

/// @brief Returns 1 if boom power is reported as "good", by the regulator. 0 otherwise.
/// @return 1 if good. 0 if less good.
uint8_t BOOM_get_pgood_status() {
    return HAL_GPIO_ReadPin(PIN_BOOM_PGOOD_IN_GPIO_Port, PIN_BOOM_PGOOD_IN_Pin);
}

/// @brief Sets the boom deploy enable pin state (on or off).
/// @param enabled 0 to disable, 1 to enable.
void BOOM_set_burn_enabled(uint8_t enabled) {
    HAL_GPIO_WritePin(PIN_BOOM_DEPLOY_EN_OUT_GPIO_Port, PIN_BOOM_DEPLOY_EN_OUT_Pin, enabled);
}
