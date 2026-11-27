#include "stm32/stm32_i2c_recovery.h"

#include "log/log.h"

/// @brief Reset the I2C peripheral if its hardware BUSY flag is stuck while no transfer is in progress.
/// @param hi2c I2C handle to check (and reset, if stuck).
/// @param log_to_uhf_radio 1 to also send the reset log to the UHF radio, 0 to skip it. Pass 0 from the radio driver itself.
/// @note Call immediately before starting a blocking transfer.
/// @details BUSY latches on a START and only clears on a STOP. A bus glitch (e.g., a device being
///     power-cycled), a lost arbitration, or a HAL timeout mid-transfer (HAL never sends STOP)
///     can leave it set forever, making every later transfer fail until reboot.
///     Safe because the OBC is the only master, and tasks never yield mid-transfer, so
///     BUSY should never be set between transfers.
void STM32_i2c_reset_if_stuck(I2C_HandleTypeDef *hi2c, uint8_t log_to_uhf_radio) {
    if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_BUSY) == RESET) {
        return;
    }

    const uint32_t isr_before = hi2c->Instance->ISR;
    uint8_t bus_num = 0; // 0 = unknown.
    if (hi2c->Instance == I2C1) {
        bus_num = 1;
    }
    else if (hi2c->Instance == I2C2) {
        bus_num = 2;
    }
    else if (hi2c->Instance == I2C3) {
        bus_num = 3;
    }
    else if (hi2c->Instance == I2C4) {
        bus_num = 4;
    }

    // DeInit+Init resets the peripheral (clears BUSY, releases SCL/SDA) and the HAL handle state/lock.
    HAL_I2C_DeInit(hi2c);
    const HAL_StatusTypeDef init_status = HAL_I2C_Init(hi2c);

    // Log after the reset, so a UHF log goes out on the recovered bus.
    LOG_message(
        LOG_SYSTEM_OBC, LOG_SEVERITY_WARNING, (log_to_uhf_radio ? LOG_SINK_ALL : LOG_all_sinks_except(LOG_SINK_UHF_RADIO)),
        "I2C%u stuck BUSY (ISR=0x%08lX). Reset peripheral -> init_status=%d, still_busy=%d",
        bus_num, isr_before, init_status, (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_BUSY) != RESET)
    );
}
