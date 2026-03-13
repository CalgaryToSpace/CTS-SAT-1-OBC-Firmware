#include "rtos_tasks/rtos_background_upkeep.h"
#include "log/log.h"
#include "config/configuration.h"
#include "timekeeping/timekeeping.h"
#include "rtos_tasks/rtos_task_helpers.h"
#include "transforms/arrays.h"
#include "main.h"
#include "rtos_tasks/rtos_tasks_rx_telecommands.h"
#include "comms_drivers/rf_antenna_switch.h"
#include "comms_drivers/comms_tx.h"
#include "log/lazy_file_log_sink.h"
#include "eps_drivers/eps_power_management.h"
#include "eps_drivers/eps_time.h"
#include "eps_drivers/eps_commands.h"

#include "cmsis_os.h"

#include <inttypes.h>
#include <stdint.h>

/// @brief If the system uptime exceeds this value, the system will reset (reboot).
/// @note This is to recover the system in case of a radiation-induced hang or other invalid state.
/// @note Default: 604800 sec = 7 days.
/// @note Set to 0 to disable this feature.
uint32_t STM32_system_reset_interval_sec = 604800;


/// @brief If the duration since an AX100 uplink telecommand exceeds this value, the system will reset (reboot).
/// @note This is to recover the system in case of a radiation-induced hang or other invalid state.
/// @note Default: 216000 sec = 2.5 days.
/// @note Set to 0 to disable this feature.
uint32_t STM32_system_reset_no_uplink_interval_sec = 216000;

uint32_t COMMS_total_beacon_count_since_boot = 0;

/// @brief How frequently to set the OBC time based on the EPS time if the time divergence is >2 seconds.
/// @note Default: 600 seconds = 10 minutes.
/// @note Set to 0 to disable time syncing.
uint32_t EPS_time_sync_period_sec = 600;

/// @brief If the OBC time and EPS time differ by more than this value, the OBC time will be set based on the EPS time.
/// @note Default: 2000 ms = 2 seconds.
/// @note Strongly related to EPS_time_sync_period_sec.
/// @note Recommendation: Do not set to < 1500-2000ms, as the EPS time is only granular to 1 second.
uint32_t EPS_max_time_deviation_for_sync_ms = 2000;

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
    if (
        (STM32_system_reset_interval_sec > 0) // Allow disabling this feature.
        && (TIME_get_current_system_uptime_ms() > (STM32_system_reset_interval_sec * 1000))
    ) {
        LOG_message(
            LOG_SYSTEM_OBC,
            LOG_SEVERITY_NORMAL,
            LOG_SINK_ALL,
            "System reset triggered due to max uptime exceeded: %ld ms > %ld sec",
            TIME_get_current_system_uptime_ms(),
            STM32_system_reset_interval_sec
        );
        HAL_Delay(1000); // Give time for the log to be sent.

        // Considered doing an LFS unmount here, but what if LFS is the cause of problems?
        
        NVIC_SystemReset();
    }
}


/// @brief If the system has not received an uplink in a very long time, reset the system.
/// @param  
/// @note Associated issue: https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/547
/// @note The intenion of this function is to recover the system in case of a radiation-induced hang
///       that isn't caught by the watchdog timer. The configuration can be extended to even longer if
///       necessary.
static void subtask_reset_system_after_no_recent_uplinks(void) {
    const uint32_t time_since_last_uplink_sec = (
        (TIME_get_current_system_uptime_ms() - AX100_uptime_at_last_received_kiss_tcmd_ms) / 1000
    );

    if (
        (STM32_system_reset_no_uplink_interval_sec > 0) // Allow disabling this feature.
        && (time_since_last_uplink_sec > STM32_system_reset_no_uplink_interval_sec)
    ) {
        LOG_message(
            LOG_SYSTEM_OBC,
            LOG_SEVERITY_NORMAL,
            LOG_SINK_ALL,
            "System reset triggered due to no recent uplinks: %ld sec > %ld sec",
            time_since_last_uplink_sec,
            STM32_system_reset_no_uplink_interval_sec
        );
        HAL_Delay(1000); // Give time for the log to be sent.

        // Considered doing an LFS unmount here, but what if LFS is the cause of problems?
        
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

    // TODO: Add configuration for beacon interval (https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/554).
    COMMS_downlink_beacon_basic_packet();
    COMMS_total_beacon_count_since_boot += 1;

    // TOOD: If complex beacon packet is enabled, also send that too.
}

static uint32_t uptime_of_last_eps_time_sync_ms = 0;

/// Periodically check the OBC time against the EPS time, and then set the OBC time based on the
/// EPS time if they diverge by more than 2000ms (or as configured).
/// We make an assumption here that the EPS's time is going to be more reliable/accurate than the OBC's time,
/// since the EPS uses a high-quality RTC.
static void subtask_sync_obc_time_based_on_eps_time(void) {
    if (
        (EPS_time_sync_period_sec > 0) // Allow disabling this feature.
        && (((TIME_get_current_system_uptime_ms() - uptime_of_last_eps_time_sync_ms) / 1000) >= EPS_time_sync_period_sec)
     ) {
        uptime_of_last_eps_time_sync_ms = TIME_get_current_system_uptime_ms();

        EPS_struct_system_status_t status;
        const uint8_t result_status = EPS_CMD_get_system_status(&status);
        if (result_status != 0) {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_ERROR,
                LOG_SINK_ALL,
                "In time syncing, EPS_CMD_get_system_status() -> Error: %d",
                result_status
            );
            return;
        }

        const uint64_t eps_time_ms = ((uint64_t)status.unix_time_sec) * 1000;
        const uint64_t obc_time_ms = TIME_get_current_unix_epoch_time_ms();
        const int64_t delta_ms = ((int64_t)obc_time_ms) - ((int64_t)eps_time_ms);
        const int64_t configured_delta_threshold_ms = (int64_t)EPS_max_time_deviation_for_sync_ms;
        if (  // Abs value greater than threshold.
            (delta_ms > configured_delta_threshold_ms)
            || (delta_ms < -configured_delta_threshold_ms)
        ) {
            char delta_ms_str[50];
            GEN_int64_to_str(delta_ms, delta_ms_str);
            LOG_message(
                LOG_SYSTEM_EPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
                "EPS vs. OBC time differ by %s (> %ldms). Setting OBC time based on EPS time.",
                delta_ms_str,
                EPS_max_time_deviation_for_sync_ms
            );

            const uint8_t sync_result = EPS_set_obc_time_based_on_eps_time(); // Emits log message!
            if (sync_result != 0) {
                LOG_message(
                    LOG_SYSTEM_EPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
                    "EPS_set_obc_time_based_on_eps_time() -> %d",
                    sync_result
                );
            }
        }
    }
}


void TASK_background_upkeep(void *argument) {
    TASK_HELP_start_of_task();
    while(1) {
        // Set the OBC time accurately first. Important step on boot.
        subtask_sync_obc_time_based_on_eps_time();
        osDelay(10); // Yield.

        subtask_send_beacon();
        osDelay(10); // Yield.

        subtask_monitor_eps_power();
        osDelay(10); // Yield.

        subtask_reset_system_after_no_recent_uplinks();
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
