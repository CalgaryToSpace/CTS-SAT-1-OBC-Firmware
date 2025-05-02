#include "system/obc_internal_drivers.h"
#include "log/log.h"
#include "main.h"

#include "stm32l4xx_hal.h"

OBC_rbf_state_enum_t OBC_get_rbf_state() {
    const GPIO_PinState rbf_state = HAL_GPIO_ReadPin(
        PIN_REMOVE_BEFORE_FLIGHT_LOW_IS_FLYING_IN_GPIO_Port,
        PIN_REMOVE_BEFORE_FLIGHT_LOW_IS_FLYING_IN_Pin   
    );
    return (rbf_state == GPIO_PIN_SET) ? OBC_RBF_STATE_BENCH : OBC_RBF_STATE_FLYING;
}