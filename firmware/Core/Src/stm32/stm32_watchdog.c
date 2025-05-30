#include "stm32/stm32_watchdog.h"

#include "log/log.h"

#include "main.h"

// Allow using the STM32 watchdog here ONLY. Everywhere else must use this file's API.
extern IWDG_HandleTypeDef hiwdg;

/// @brief Uptime at the last time the watchdog was petted, in milliseconds.
uint32_t STM32_watchdog_uptime_last_pet_ms = 0;

/// @brief Total number of times the STM32 watchdog has been petted since boot.
uint32_t STM32_watchdog_total_pets_since_boot = 0;

/// @brief Pet the STM32 Independent Watchdog (IWDG).
/// @note Also, updates tracking variables and logs warnings if the petting took too long or too short.
/// @note The watchdog must be petted at least once every 16 seconds, and no more than once every 200ms.
/// @note A warning is issued if the time since the last pet is more than 15 seconds or less than 1 second.
void STM32_pet_watchdog() {
    // Actually pet the watchdog.
    HAL_IWDG_Refresh(&hiwdg);

    // Do checks about stats of watchdog petting.
    const uint32_t cur_time_ms = HAL_GetTick();
    STM32_watchdog_uptime_last_pet_ms = cur_time_ms;
    STM32_watchdog_total_pets_since_boot++;

    // Check the interval since the last pet.
    const uint32_t time_since_last_pet_ms = cur_time_ms - STM32_watchdog_uptime_last_pet_ms;
    if (time_since_last_pet_ms > 15000) {
        LOG_message(
            LOG_SYSTEM_OBC, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "STM32 watchdog petting took a long time: %lu ms since last pet (>15 sec)",
            time_since_last_pet_ms
        );
    }
    else if (time_since_last_pet_ms < 1000) {
        LOG_message(
            LOG_SYSTEM_OBC, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
            "STM32 watchdog petting took a short time: %lu ms since last pet (<1000ms)",
            time_since_last_pet_ms
        );
    }
}
