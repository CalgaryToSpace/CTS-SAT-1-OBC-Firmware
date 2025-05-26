#include "rtos_tasks/rtos_background_upkeep.h"
#include "log/log.h"
#include "config/configuration.h"
#include "timekeeping/timekeeping.h"
#include "rtos_tasks/rtos_task_helpers.h"
#include "main.h"

#include "cmsis_os.h"

#include "eps_drivers/eps_power_management.h"

static uint32_t EPS_monitor_last_uptime_ms = 0;

static void subtask_monitor_eps_power(void) {
    // EPS overcurrent monitor upkeep
    const uint64_t current_time = HAL_GetTick();
    if (EPS_monitor_last_uptime_ms + EPS_monitor_interval_ms < current_time) {

        const uint8_t EPS_monitor_result = EPS_monitor_and_disable_overcurrent_channels();

        if (EPS_monitor_result != 0) {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_ERROR,
                LOG_SINK_ALL,
                "EPS_monitor_and_disable_overcurrent_channels() -> Error: %d", EPS_monitor_result
            );
        }
        else {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_DEBUG,
                LOG_SINK_ALL,
                "EPS overcurrent monitor checked successfully."
            );
        }
        EPS_monitor_last_uptime_ms = current_time;
    }
}

/// @brief If the system exceeds a very long uptime, reset the system.
/// @param  
/// @note Associated issue: https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/282
/// @note The intenion of this function is to recover the system in case of a radiation-induced hang
///       that isn't caught by the watchdog timer. The configuration can be extended to even longer if
///       necessary.
static void subtask_reset_system_after_very_long_uptime(void) {
    if (TIM_get_current_system_uptime_ms() > STM32_system_reset_interval_ms) {
        LOG_message(
            LOG_SYSTEM_OBC,
            LOG_SEVERITY_NORMAL,
            LOG_SINK_ALL,
            "System reset triggered due to max uptime exceeded: %ld ms",
            STM32_system_reset_interval_ms
        );
        HAL_Delay(1000); // Give time for the log to be sent.
        
        NVIC_SystemReset();
    }
}

void TASK_background_upkeep(void *argument) {
    TASK_HELP_start_of_task();
    while(1) {
        subtask_monitor_eps_power();
        osDelay(10); // Yield.

        subtask_reset_system_after_very_long_uptime();
        osDelay(10); // Yield.
        
        osDelay(1000);
    }
}