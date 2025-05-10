#include "system/obc_internal_drivers.h"
#include "log/log.h"
#include "main.h"

#include "stm32l4xx_hal.h"

OBC_rbf_state_enum_t OBC_get_rbf_state() {
    const GPIO_PinState rbf_state = HAL_GPIO_ReadPin(
        PIN_REMOVE_BEFORE_FLIGHT_LOW_IS_FLYING_IN_GPIO_Port,
        PIN_REMOVE_BEFORE_FLIGHT_LOW_IS_FLYING_IN_Pin   
    );
    return (rbf_state == GPIO_PIN_SET) ? OBC_RBF_STATE_FLYING : OBC_RBF_STATE_BENCH;
}


/// @brief Set the state of the LED visible from the outside of the satellite (on solar panel).
/// @param enabled 
/// @note This LED is ALSO the LED on the STM32 dev kit, and the LED on the OBC.
/// @note This LED is used for the early bootup sequence indication. It is not for "debugging".
void OBC_set_external_led(uint8_t enabled) {
    HAL_GPIO_WritePin(PIN_LED_DEVKIT_LD2_GPIO_Port, PIN_LED_DEVKIT_LD2_Pin, enabled);
    HAL_GPIO_WritePin(PIN_LED_GP1_OUT_GPIO_Port, PIN_LED_GP1_OUT_Pin, enabled);
}
