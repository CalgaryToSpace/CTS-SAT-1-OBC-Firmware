#include "main.h"
#include "comms_drivers/i2c_sharing.h"
#include "log/log.h"

static uint8_t i2c_borrowed = 0;

/// @brief Call this function to borrow the hi2c1 bus for a short time. Stops AX100 RX.
/// @note This function can be called several times in a row (no need to track state externally).
void I2C_borrow_bus_1(void) {
    // TODO: Decide if we want this check here or not.
    // if (i2c_borrowed) { // Already borrowed.
    //     return;
    // }
    const HAL_StatusTypeDef result = HAL_I2C_DisableListen_IT(&hi2c1);
    if (result != HAL_OK) {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "HAL_I2C_DisableListen_IT(&hi2c1) -> %d", result
        );
        return;
    }
    i2c_borrowed = 1; // Set borrowed state.
}

/// @brief Call this function when done borrowing the hi2c1 bus. Starts/resumes AX100 RX.
/// @note This function can be called several times in a row (no need to track state externally).
void I2C_done_borrowing_bus_1(void) {
    // TODO: Decide if we want this check here or not.
    // if (!i2c_borrowed) { // Not borrowed, so nothing to do.
    //     return;
    // }
    const HAL_StatusTypeDef result = HAL_I2C_EnableListen_IT(&hi2c1);
    if (result != HAL_OK) {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "HAL_I2C_EnableListen_IT(&hi2c1) -> %d", result
        );
        return;
    }
    i2c_borrowed = 0; // Reset borrowed state.
}

/// @brief Initialize the I2C bus 1 interrupt for receiving data from the AX100.
void I2C_init_bus_1_interrupt_for_rx(void) {
    i2c_borrowed = 0; // Reset borrowed state.
    
    const HAL_StatusTypeDef result = HAL_I2C_EnableListen_IT(&hi2c1);
    if (result != HAL_OK) {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "HAL_I2C_EnableListen_IT(&hi2c1) -> %d", result
        );
        return;
    }
}
