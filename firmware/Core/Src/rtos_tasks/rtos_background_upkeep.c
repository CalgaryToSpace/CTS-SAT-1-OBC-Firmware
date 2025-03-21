#include "rtos_tasks/rtos_background_upkeep.h"
#include "log/log.h"
#include "config/configuration.h"
#include "timekeeping/timekeeping.h"
#include "rtos_tasks/rtos_task_helpers.h"
#include "main.h"

#include "cmsis_os.h"

#include "eps_drivers/eps_power_management.h"
#include "comms_drivers/comms_drivers.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"


uint64_t EPS_monitor_last_uptime = 0;
uint64_t persistent_dipole_last_uptime = 0;
uint8_t beacon = 0;

void TASK_background_upkeep(void *argument) {
    TASK_HELP_start_of_task();

    uint64_t last_COMM_response_interval_ms = 1200000; 

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
                "System reset triggered due to max uptime exceeded: %ld ms",
                STM32_system_reset_interval_ms
            );
            NVIC_SystemReset();
        }
        //Check optimum antenna every 20 min
        if (persistent_dipole_last_uptime + persistent_dipole_interval_ms < current_time && !persistent_dipole_interval_ms) {

            const uint8_t persistent_dipole_result = COMMS_persistant_dipole_logic();

            if (!persistent_dipole_result) {
                LOG_message(
                    LOG_SYSTEM_UHF_RADIO,
                    LOG_SEVERITY_ERROR,
                    LOG_SINK_ALL,
                    "COMMS_persistant_dipole_logic() -> Error: %d", persistent_dipole_result
                );
            }
            else {
                LOG_message(
                    LOG_SYSTEM_UHF_RADIO,
                    LOG_SEVERITY_NORMAL, 
                    LOG_SINK_ALL,
                    "Persistint dipole control serviced successfully to %u.",
                    persistent_dipole_result
                );
            }
            persistent_dipole_last_uptime = current_time;
        }
        
        //Check if received no response in last 20 min
        if (!beacon && last_response_ms + last_COMM_response_interval_ms <= current_time) {
            beacon = 1;
            LOG_message(
                LOG_SYSTEM_UHF_RADIO,
                LOG_SEVERITY_NORMAL, 
                LOG_SINK_ALL,
                "Antenna beaconing is on."
            );
        }

        //Beacon logic
        //If beacon is on and received response in last 20 min, turn off beacon
        if (beacon && last_response_ms + last_COMM_response_interval_ms > current_time) {
            beacon = 0;
            LOG_message(
                LOG_SYSTEM_UHF_RADIO,
                LOG_SEVERITY_NORMAL, 
                LOG_SINK_ALL,
                "Antenna beaconing is off."
            );
        }
        //Switch ant every sec
        else if(beacon) {
            COMMS_set_dipole_switch_state(HAL_GPIO_ReadPin(PIN_UHF_CTRL_OUT_GPIO_Port, PIN_UHF_CTRL_OUT_Pin) ? 1 : 2);
        }

        osDelay(1000);
    }
}