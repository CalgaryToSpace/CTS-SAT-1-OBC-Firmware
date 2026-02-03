#include "rtos_tasks/rtos_background_upkeep.h"
#include "log/log.h"
#include "config/configuration.h"
#include "timekeeping/timekeeping.h"
#include "rtos_tasks/rtos_task_helpers.h"
#include "main.h"
#include "rtos_tasks/rtos_tasks_rx_telecommands.h"
#include "comms_drivers/rf_antenna_switch.h"
#include "comms_drivers/comms_tx.h"
#include "log/lazy_file_log_sink.h"

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
    if (TIME_get_current_system_uptime_ms() > STM32_system_reset_interval_ms) {
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

/// @brief Update the RF switch state based on the current mode.
/// @note Implemented per https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/228
static void subtask_update_rf_switch(void) {
    const uint32_t duration_since_last_uplink_sec = (TIME_get_current_system_uptime_ms() - AX100_uptime_at_last_received_kiss_tcmd_ms) / 1000;

    if ((COMMS_rf_switch_control_mode != COMMS_RF_SWITCH_CONTROL_MODE_TOGGLE_BEFORE_EVERY_BEACON) // Log minimiation condition.
        && (duration_since_last_uplink_sec > COMMS_max_duration_without_uplink_before_setting_default_rf_switch_mode_sec)
    ) {
        COMMS_rf_switch_control_mode = COMMS_RF_SWITCH_CONTROL_MODE_TOGGLE_BEFORE_EVERY_BEACON;
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "RF switch control mode set to default due to no uplinks: %ld sec > %ld sec",
            duration_since_last_uplink_sec,
            COMMS_max_duration_without_uplink_before_setting_default_rf_switch_mode_sec
        );
        return;
    }

    if (COMMS_rf_switch_control_mode == COMMS_RF_SWITCH_CONTROL_MODE_FORCE_ANT1) {
        COMMS_set_rf_switch_state(1);
    }
    else if (COMMS_rf_switch_control_mode == COMMS_RF_SWITCH_CONTROL_MODE_FORCE_ANT2) {
        COMMS_set_rf_switch_state(2);
    }
    else if ((COMMS_rf_switch_control_mode == COMMS_RF_SWITCH_CONTROL_MODE_USE_ADCS_NORMAL)
        || (COMMS_rf_switch_control_mode == COMMS_RF_SWITCH_CONTROL_MODE_USE_ADCS_FLIPPED)
    ) {
        const uint8_t antenna_num = COMMS_find_optimal_antenna_using_adcs();
        if (antenna_num == 0) {
            LOG_message(
                LOG_SYSTEM_ADCS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                "Failed to find optimal antenna using ADCS. Resetting to TOGGLE_BEFORE_EVERY_BEACON mode."
            );
            COMMS_rf_switch_control_mode = COMMS_RF_SWITCH_CONTROL_MODE_TOGGLE_BEFORE_EVERY_BEACON;
            return;
        }
        
        // Main update: Set the RF switch state based on the ADCS recommendation.
        if (COMMS_rf_switch_control_mode == COMMS_RF_SWITCH_CONTROL_MODE_USE_ADCS_FLIPPED) {
            COMMS_set_rf_switch_state(3 - antenna_num); // 1 -> 2, 2 -> 1
        }
        else {
            COMMS_set_rf_switch_state(antenna_num);
        }
    }
}

static void subtask_send_beacon(void) {
    if (COMMS_rf_switch_control_mode == COMMS_RF_SWITCH_CONTROL_MODE_TOGGLE_BEFORE_EVERY_BEACON) {
        COMMS_toggle_rf_switch_state();

        HAL_Delay(20); // Wait for the RF switch to settle. Should only take <100 nanoseconds.
    }

    // TODO: Add configuration for beacon interval.
    COMMS_downlink_beacon_basic_packet();

    // TOOD: If complex beacon packet is enabled, also send that too.
}

void TASK_background_upkeep(void *argument) {
    TASK_HELP_start_of_task();
    while(1) {
        subtask_send_beacon();
        osDelay(10); // Yield.

        subtask_monitor_eps_power();
        osDelay(10); // Yield.

        subtask_reset_system_after_very_long_uptime();
        osDelay(10); // Yield.

        subtask_update_rf_switch();
        osDelay(10); // Yield.

        LOG_subtask_handle_sync_and_close_of_current_log_file();
        osDelay(10); // Yield.
        
        osDelay(3000);
    }
}
