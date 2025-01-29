
#include "eps_drivers/eps_commands.h"
#include "eps_drivers/eps_time.h"
#include "rtos_tasks/rtos_eps_tasks.h"
#include "rtos_tasks/rtos_task_helpers.h"
#include "log/log.h"
#include "timekeeping/timekeeping.h"
#include "eps_drivers/eps_power_management.h"
#include "config/configuration.h"

#include "cmsis_os.h"

#include <string.h>
#include <inttypes.h>
#include <stdint.h>

uint32_t watchdog_timer = 7000, EPS_monitor_timer = 1000;       //TODO: Set watchdog timer and EPS monitor timer

/**
 * @brief Services the EPS watchdog periodically.
 *
 * The task services the watchdog periodically. If the task does not run for longer than
 * TTC_WDG_TIMEOUT, the peripheral reset will occur and communication with the EPS will be
 * interrupted. The TTC_WDG_TIMEOUT is set to 300s in the ISIS EPS2.
 *
 * The task also periodically checks the power consumption of the satellite and disables any
 * channels that exceed a certain threshold.
 *.
 */
void TASK_service_eps_watchdog(void *argument) {
    TASK_HELP_start_of_task();
    // This task should sleep for 0.25*TTC_WDG_TIMEOUT = 75s to avoid the peripheral reset
    // going off. Timing also ensures that in the case the peripheral reset restores
    // communication, the watch dog timer will reset before "watchdog reset" causes a full 
    // system reset (See ISIS.EPS2.ICD.SW.IVID.7 pg.9 for further explanation).
    // To avoid resets, we sleep for much shorter than that.
    const uint32_t sleep_duration_ms = 1000;

    // Sleep 10s at the start so that more important tasks work first.
    // Important to service the watchdog near the start, though.
    osDelay(10000);
    uint64_t last_time_watch_dog = TIM_get_current_unix_epoch_time_ms();

    uint64_t last_time_EPS = TIM_get_current_unix_epoch_time_ms();

    while(1) {
        if (TIM_get_current_unix_epoch_time_ms() - last_time_watch_dog > watchdog_timer) {
            const uint8_t result = EPS_CMD_watchdog();

            if (result != 0) {
                LOG_message(
                    LOG_SYSTEM_EPS,
                    LOG_SEVERITY_ERROR,
                    LOG_SINK_ALL,
                    "EPS_CMD_watchdog() -> Error: %d", result
                );
            }
            else {
                LOG_message(
                    LOG_SYSTEM_EPS,
                    LOG_SEVERITY_NORMAL, 
                    LOG_SINK_ALL,
                    "EPS watchdog serviced successfully." 
                );
            }
            last_time_watch_dog = TIM_get_current_unix_epoch_time_ms();
        }
        
        if (TIM_get_current_unix_epoch_time_ms() - last_time_EPS > EPS_monitor_timer) {
            const uint8_t result_power = EPS_power_monitoring();
            if (result_power != 0) {
                LOG_message(
                    LOG_SYSTEM_EPS,
                    LOG_SEVERITY_ERROR,
                    LOG_SINK_ALL,
                    "EPS_power_monitoring() -> Error: %d", result_power
                );
            }
            else {
                LOG_message(
                    LOG_SYSTEM_EPS,
                    LOG_SEVERITY_NORMAL, 
                    LOG_SINK_ALL,
                    "EPS power monitoring serviced successfully." 
                );
            }
            last_time_EPS = TIM_get_current_unix_epoch_time_ms();
        }
        osDelay(sleep_duration_ms);
    }
}

/*
* @brief on boot this thread sets the obc time to the eps time. After that it periodically
* checks the obc time against the eps time and logs when they diverge by more than 10 seconds.
*/
void TASK_time_sync(void *argument) {
    TASK_HELP_start_of_task();

    // Run on boot after 5000ms, but only run every 45 seconds in normal operation.
    uint32_t sleep_duration_ms = 5000;
    while(1) {
        // osDelay must be at the top of the while(1) loop so that `continue;` doesn't skip it.
        osDelay(sleep_duration_ms);


        // First, try to sync EPS-to-OBC for the initial boot (or in case stuff gets funky later).
        if (TIM_last_synchronization_source == TIM_SOURCE_NONE) {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_NORMAL,
                LOG_SINK_ALL,
                "Setting OBC time based on EPS time because last_source == TIM_SOURCE_NONE"
            );

            if (EPS_set_obc_time_based_on_eps_time() != 0) {
                LOG_message(
                    LOG_SYSTEM_EPS,
                    LOG_SEVERITY_ERROR,
                    LOG_SINK_ALL,
                    "EPS_set_obc_time_based_on_eps_time() -> Error"
                );
            }
            continue;
        }
        // If the OBC's time is ever somehow less than 2010-01-01T00:00:00Z, then sync to EPS time.
        if (TIM_get_current_unix_epoch_time_ms() < 1262329200000) {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_NORMAL, 
                LOG_SINK_ALL,
                "Setting OBC time based on EPS time because current time < 2010-01-01"
            );
            if (EPS_set_obc_time_based_on_eps_time() != 0) {
                LOG_message(
                    LOG_SYSTEM_EPS,
                    LOG_SEVERITY_ERROR,
                    LOG_SINK_ALL,
                    "EPS_set_obc_time_based_on_eps_time() -> Error"
                );
            }
            continue;
        }

        // For all subsequent runs, sleep for 10 mins.
        sleep_duration_ms = 600000;
      
        EPS_struct_system_status_t status;
        const uint8_t result_status = EPS_CMD_get_system_status(&status);
        if (result_status != 0) {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_ERROR,
                LOG_SINK_ALL,
                "EPS_CMD_get_system_status() -> Error: %d",
                result_status
            );
            continue;
        }

        // Use uint32_t for these values in seconds, as int32_t won't overflow until 2038.
        const uint32_t eps_time_sec = status.unix_time_sec;
        const uint32_t obc_time_sec = TIM_get_current_unix_epoch_time_ms() / 1000;
        const int32_t delta_seconds = ((int32_t)obc_time_sec) - ((int32_t)eps_time_sec);
        if (abs(delta_seconds) > 10) {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_WARNING,
                LOG_SINK_ALL,
                "Warning: EPS time and OBC time differ by more than 10 seconds. \n EPS time seconds: %" PRIi32 ",\n OBC time seconds: %" PRIi32,
                eps_time_sec, obc_time_sec
            );
        }
    } // End of task while loop
}

