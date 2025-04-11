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

/// @brief Time since the ADCS last checked optimum antenna based on ADCS
uint64_t AX100_ADCS_based_ant_selection_last_uptime = 0;

/// @brief The amount of time a overrided antenna should be selected. After the time is up, the
/// antenna selection mode will be set to use ADCS. 
uint64_t AX100_override_selected_ant_interval_ms = 1800000; // 30 min

uint8_t beacon = 0;

void TASK_background_upkeep(void *argument) {
    TASK_HELP_start_of_task();

    uint64_t last_COMM_response_interval_ms = 1200000; // 20 min

    while(1) {

        // EPS overcurrent monitor upkeep
        const uint64_t current_time = TIM_get_current_system_uptime_ms();
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
        // End EPS overcurrent monitor upkeep
        
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
        
        // ADCS based antenna selection upkeep
        // Check if the last time the ADCS checked for optimum antenna is greater than the interval
        if (COMMS_current_ant_mode == COMMS_ANTENNA_SELECTION_MODE_USE_ADCS && AX100_ADCS_based_ant_selection_last_uptime + AX100_ADCS_based_ant_selection_interval_ms < current_time) {

            const uint8_t persistent_dipole_result = COMMS_determine_and_update_dipole_antenna_switch();

            if (!persistent_dipole_result) {
                LOG_message(
                    LOG_SYSTEM_UHF_RADIO,
                    LOG_SEVERITY_ERROR,
                    LOG_SINK_ALL,
                    "COMMS_determine_and_update_dipole_antenna_switch() -> Error: %d", persistent_dipole_result
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
            AX100_ADCS_based_ant_selection_last_uptime = current_time;
        }
        // End ADCS based antenna selection upkeep

        // Beacon logic 
        // Check if received no response in last 20 min
        if (COMMS_current_ant_mode == COMMS_ANTENNA_SELECTION_MODE_TOGGLE_BEFORE_EVERY_BEACON && !beacon && AX100_last_received_uplink_uptime_ms + last_COMM_response_interval_ms <= current_time) {
            beacon = 1;
            LOG_message(
                LOG_SYSTEM_UHF_RADIO,
                LOG_SEVERITY_NORMAL, 
                LOG_SINK_ALL,
                "Antenna beaconing is on."
            );
        }

        // If beacon is on and received new response since the last response
        if (COMMS_current_ant_mode == COMMS_ANTENNA_SELECTION_MODE_TOGGLE_BEFORE_EVERY_BEACON && beacon && AX100_last_received_uplink_uptime_ms + last_COMM_response_interval_ms > current_time) {
            beacon = 0;
            LOG_message(
                LOG_SYSTEM_UHF_RADIO,
                LOG_SEVERITY_NORMAL, 
                LOG_SINK_ALL,
                "Antenna beaconing is off."
            );
        }
        // Switch ant every sec
        else if(COMMS_current_ant_mode == COMMS_ANTENNA_SELECTION_MODE_TOGGLE_BEFORE_EVERY_BEACON && beacon) {
            COMMS_toggle_active_antenna();
        }
        // End Beacon logic

        // User override Antenna selection
        if (COMMS_current_ant_mode == COMMS_ANTENNA_SELECTION_MODE_OVERRIDE_BY_TELECOMMAND_FOR_30_MINUTES && AX100_ADCS_based_ant_selection_last_uptime + AX100_override_selected_ant_interval_ms < current_time) {
            COMMS_current_ant_mode = COMMS_ANTENNA_SELECTION_MODE_USE_ADCS;
            LOG_message(
                LOG_SYSTEM_UHF_RADIO,
                LOG_SEVERITY_NORMAL, 
                LOG_SINK_ALL,
                "Antenna selection mode is set to use ADCS."
            );
        }
        // End User override Antenna selection

        osDelay(1000);
    }
}