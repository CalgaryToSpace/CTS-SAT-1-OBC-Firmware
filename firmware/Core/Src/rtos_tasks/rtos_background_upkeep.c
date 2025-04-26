#include "rtos_tasks/rtos_background_upkeep.h"
#include "log/log.h"
#include "config/configuration.h"
#include "timekeeping/timekeeping.h"
#include "rtos_tasks/rtos_task_helpers.h"
#include "system/system_low_power_mode.h"

#include "main.h"

#include "cmsis_os.h"

#include "eps_drivers/eps_power_management.h"

uint64_t EPS_monitor_last_uptime = 0;

void TASK_background_upkeep(void *argument) {
    TASK_HELP_start_of_task();

    uint32_t last_time_checked_for_low_power_mode = 0;
    const uint32_t low_power_mode_interval_ms = 600000; // 10 minutes
    while(1) {
        if (HAL_GetTick() - last_time_checked_for_low_power_mode > low_power_mode_interval_ms) {
            // TODO: Is it ok if both happen?

            // Check if EPS goes into low power mode, enter low power mode if it does.
            SYS_check_eps_and_enter_low_power_mode();
            
            // Check if battery is below 10%, enter low power mode if it is.
            SYS_check_battery_and_enter_low_power_mode();

            last_time_checked_for_low_power_mode = HAL_GetTick();
        }

        //EPS overcurrent monitor upkeep
        const uint64_t current_time = TIM_get_current_unix_epoch_time_ms();
        if (EPS_monitor_last_uptime + EPS_monitor_interval_ms < current_time) {

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
                    LOG_SEVERITY_NORMAL, 
                    LOG_SINK_ALL,
                    "EPS overcurrent monitor serviced successfully." 
                );
            }
            EPS_monitor_last_uptime = current_time;
        }
        
        if (TIM_get_current_system_uptime_ms() > STM32_system_reset_interval_ms) {
            LOG_message(
                LOG_SYSTEM_OBC,
                LOG_SEVERITY_NORMAL,
                LOG_SINK_ALL,
                "System reset triggered due to max uptime exceeded: %ld ms",
                STM32_system_reset_interval_ms
            );
            NVIC_SystemReset();
        }
        osDelay(1000);
    }
}