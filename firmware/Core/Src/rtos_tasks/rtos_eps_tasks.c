
#include "eps_drivers/eps_commands.h"
#include "eps_drivers/eps_time.h"
#include "rtos_tasks/rtos_eps_tasks.h"
#include "rtos_tasks/rtos_task_helpers.h"
#include "log/log.h"
#include "cmsis_os.h"



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


void TASK_sync_eps_time(void *argument) {
    TASK_HELP_start_of_task();

    // Run on boot after 5000ms, but only run every 45 seconds in normal operation.

    uint32_t sleep_duration_ms = 5000;
    while(1) {
        // osDelay must be at the top of the while(1) loop so that `continue;` doesn't skip it.
        osDelay(sleep_duration_ms);

        // For all subsequent runs, sleep for 45 seconds.
        sleep_duration_ms = 45000;
      
        const uint8_t result = EPS_set_eps_time_to_obc_time();
        if (result != 0) {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_ERROR,
                LOG_SINK_ALL,
                "EPS_set_eps_time_to_obc_time() -> Error: %d", result
            );
            continue;
        }

        LOG_message(
            LOG_SYSTEM_EPS,
            LOG_SEVERITY_NORMAL, 
            LOG_SINK_ALL,
            "EPS_set_eps_time_to_obc_time() -> Success."
        );
    } // End of task while loop
}
