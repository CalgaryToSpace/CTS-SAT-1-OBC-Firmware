
#include "eps_drivers/eps_commands.h"
#include "eps_drivers/eps_time.h"
#include "rtos_tasks/rtos_eps_tasks.h"
#include "rtos_tasks/rtos_task_helpers.h"
#include "log/log.h"
#include "timekeeping/timekeeping.h"

#include "cmsis_os.h"

#include <inttypes.h>
#include <stdint.h>



/// @brief periodically sends a command to the eps to reset the watchdog timer
/// @note The eps has two watchdog timers: The "watchdog reset" and the "peripheral reset".
/// The "watchdog reset" resets the entire eps system when TTC_WDG_TIMEOUT (default 300s, 
/// see ISIS.EPS2.ICD.SW.IVID.7 pg. 85) seconds pass without a command being received. 
/// The "peripheral reset" resets the communication peripheral on the eps's mcu 
/// after 0.65 * TTC_WDG_TIMEOUT seconds. The "peripheral reset" allows for more graceful 
/// resets if the eps's communication peripheral was responsible for loss of connection.
void TASK_service_eps_watchdog(void *argument) {
    TASK_HELP_start_of_task();
    // This task should sleep for 0.25*TTC_WDG_TIMEOUT = 75s to avoid the peripheral reset
    // going off. Timing also ensures that in the case the peripheral reset restores
    // communication, the watch dog timer will reset before "watchdog reset" causes a full 
    // system reset (See ISIS.EPS2.ICD.SW.IVID.7 pg.9 for further explanation).
    // To avoid resets, we sleep for much shorter than that.
    const uint32_t sleep_duration_ms = 20000;

    // Sleep 10s at the start so that more important tasks work first.
    // Important to service the watchdog near the start, though.
    osDelay(10000);

    while(1) {
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
