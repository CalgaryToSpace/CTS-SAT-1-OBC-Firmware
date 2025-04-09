#include "rtos_tasks/rtos_background_upkeep.h"
#include "log/log.h"
#include "config/configuration.h"
#include "timekeeping/timekeeping.h"
#include "rtos_tasks/rtos_task_helpers.h"
#include "main.h"

#include "cmsis_os.h"

#include "eps_drivers/eps_power_management.h"

uint64_t EPS_monitor_last_uptime = 0;

void TASK_background_upkeep(void *argument) {
    TASK_HELP_start_of_task();
    while(1) {

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
                "System reset triggered."
            );
            NVIC_SystemReset();
        }
        osDelay(1000);
    }
}