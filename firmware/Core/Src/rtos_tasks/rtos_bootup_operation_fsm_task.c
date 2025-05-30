#include "rtos_tasks/rtos_bootup_operation_fsm_task.h"
#include "rtos_tasks/rtos_tasks_rx_telecommands.h"
#include "obc_systems/external_led_and_rbf.h"
#include "littlefs/littlefs_helper.h"
#include "eps_drivers/eps_channel_control.h"
#include "antenna_deploy_drivers/ant_commands.h"
#include "timekeeping/timekeeping.h"
#include "log/log.h"

/// @brief The uptime, as defined in the Launch Provider ICD, at which the antennas should be deployed.
uint32_t COMMS_uptime_to_start_ant_deployment_sec = 30 * 60;

CTS1_operation_state_enum_t CTS1_operation_state = CTS1_OPERATION_STATE_BOOTED_AND_WAITING;

static void set_operation_state_and_log_if_changed(CTS1_operation_state_enum_t new_state, const char *reason_str) {
    if (CTS1_operation_state == new_state) {
        return; // No change.
    }

    const CTS1_operation_state_enum_t old_state = CTS1_operation_state;

    // Set the new state. Ensure it's update BEFORE the LOG_message, so that the log message
    // is called with the new state active (i.e., so it can be sent over the radio).
    CTS1_operation_state = new_state;

    LOG_message(
        LOG_SYSTEM_OBC,
        LOG_SEVERITY_NORMAL,
        LOG_SINK_ALL,
        "CTS1 operation state changed from %s to %s (%s).",
        CTS1_operation_state_enum_TO_str(old_state),
        CTS1_operation_state_enum_TO_str(new_state),
        reason_str
    );
}

/// @brief Check if the filesystem has the file "/bypass_deployment_and_enable_radio.txt".
/// @return If the file exists, return 1. If it does not exist, or on failure, return 0.
/// @note This function returns the same result on failure as on non-existence. The distinction is not useful to the caller.
static uint8_t does_filesystem_have_bypass_deployment_and_enable_radio_file(void) {
    // Check if the file exists in the filesystem.
    const char* filename = "/bypass_deployment_and_enable_radio.txt";
    
    if (!LFS_is_lfs_mounted) {
        LFS_mount(); // Steamroll on error.
    }

    // Check if the file exists.
    lfs_soff_t file_size = LFS_file_size(filename);
    if (file_size > 0) {
        return 1; // File exists.
    }
    else {
        return 0; // File does not exist or error.
    }
}

/// @brief Power on EPS channel, check if all antennas have been deployed.
/// @note This function returns the same result on failure as on "antennas not deployed". The distinction is not useful to the caller.
/// @return 1 if all antennas are deployed. 0 if not all antennas are deployed. 0 if an error occurs.
static uint8_t have_all_antennas_deployed(enum ANT_i2c_bus_mcu ant_bus) {
    // Check if all antennas have deployed.
    EPS_set_channel_enabled(EPS_CHANNEL_3V3_UHF_ANTENNA_DEPLOY, 1);

    HAL_Delay(ANT_POWER_ON_BOOTUP_DURATION_MS);

    ANT_deployment_status_struct_t resp;
    const uint8_t failed = ANT_CMD_report_deployment_status(ant_bus, &resp);
    if (failed > 0) {
        // Log this as an error because failing communication with this thing is a big deal.
        LOG_message(
            LOG_SYSTEM_ANTENNA_DEPLOY,
            LOG_SEVERITY_ERROR,
            LOG_SINK_ALL,
            "ANT_CMD_report_deployment_status() -> Error: %d",
            failed
        );
        return 0; // Error.
    }

    // Check if all antennas are deployed. Return 1 if all deployed, 0 if not.
    return (
        resp.antenna_1_deployed
        && resp.antenna_2_deployed
        && resp.antenna_3_deployed
        && resp.antenna_4_deployed
    );
}

/// @brief State machine for the bootup operation state.
/// @note Implemented per https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/420
static inline void SUBTASK_bootup_operation_state_check_for_state_transitions(void) {
    // First group: Conditions checked in every state.
    {
        // Condition 1: If any uplink has been received - if (AX100_uptime_at_last_received_kiss_tcmd_ms > 1000) -> NOMINAL_WITH_RADIO_TX
        // Reason: Simple override to allow the satellite to function once we've asked it to.
        // Saves us in case successful antenna deployment occurs but is not detected.
        if (AX100_uptime_at_last_received_kiss_tcmd_ms > 1000) {
            set_operation_state_and_log_if_changed(
                CTS1_OPERATION_STATE_NOMINAL_WITH_RADIO_TX,
                "Condition 1: Uplink received"
            );
            return;
        }

        // Condition 2: If (RBF_STATE == BENCH) -> NOMINAL_WITHOUT_RADIO_TX
        // Reason: On bench testing, bypass the countdown, but disable the radio.
        if (OBC_get_rbf_state() == OBC_RBF_STATE_BENCH) {
            set_operation_state_and_log_if_changed(
                CTS1_OPERATION_STATE_NOMINAL_WITHOUT_RADIO_TX,
                "Condition 2: RBF == BENCH"
            );
            return;
        }
        
        // Condition 3: If /bypass_deployment_and_enable_radio.txt file exists in filesystem -> NOMINAL_WITH_RADIO_TX
        // Reason: On reboots (after day 1 of mission), bypass the waiting stage.
        // This serves as a way to add an override if there's an issue with the deployment board detection, for example.
        if (
            (CTS1_operation_state != CTS1_OPERATION_STATE_NOMINAL_WITH_RADIO_TX) // Optimization: Don't check if already in this state.
            && does_filesystem_have_bypass_deployment_and_enable_radio_file()
        ) {
            set_operation_state_and_log_if_changed(
                CTS1_OPERATION_STATE_NOMINAL_WITH_RADIO_TX,
                "Condition 3: Bypass file exists"
            );
            return;
        }
    }

    // Second group: Conditions checked only in the BOOTED_AND_WAITING state.
    if (CTS1_operation_state == CTS1_OPERATION_STATE_BOOTED_AND_WAITING) {
        // Condition 4: If (RBF_STATE == FLYING) && (uptime > 30 minutes) -> DEPLOYING
        // Reason: Nominal post-ejection transition
        if ((OBC_get_rbf_state() == OBC_RBF_STATE_FLYING) && (TIM_get_current_system_uptime_ms() > (COMMS_uptime_to_start_ant_deployment_sec * 1000))) {
            set_operation_state_and_log_if_changed(
                CTS1_OPERATION_STATE_DEPLOYING,
                "Condition 4: RBF == DEPLOY AND uptime > 30 minutes"
            );
            return;
        }

        // Condition 5: If all four antennas have been deployed - check using ANT_ commands -> NOMINAL_WITH_RADIO_TX
        // Reason: On reboot after day 1 of mission, bypass the waiting stage.
        if (have_all_antennas_deployed(ANT_I2C_BUS_A_MCU_A)) {
            set_operation_state_and_log_if_changed(
                CTS1_OPERATION_STATE_NOMINAL_WITH_RADIO_TX,
                "Condition 5A: All antennas deployed [BUS_MCU_A]"
            );
            return;
        }
        if (have_all_antennas_deployed(ANT_I2C_BUS_B_MCU_B)) {
            set_operation_state_and_log_if_changed(
                CTS1_OPERATION_STATE_NOMINAL_WITH_RADIO_TX,
                "Condition 5B: All antennas deployed [BUS_MCU_B]"
            );
            return;
        }
    }

    if (CTS1_operation_state == CTS1_OPERATION_STATE_DEPLOYING) {
        // Condition 6: If deployment successful -> NOMINAL_WITH_RADIO_TX
        // Note: Same as Condition 5, but from the DEPLOYING state.
        // Reason: Nominal exit after successful deployment.
        if (have_all_antennas_deployed(ANT_I2C_BUS_A_MCU_A)) {
            set_operation_state_and_log_if_changed(
                CTS1_OPERATION_STATE_NOMINAL_WITH_RADIO_TX,
                "Condition 6A: All antennas deployed [BUS_MCU_A]"
            );
            return;
        }
        if (have_all_antennas_deployed(ANT_I2C_BUS_B_MCU_B)) {
            set_operation_state_and_log_if_changed(
                CTS1_OPERATION_STATE_NOMINAL_WITH_RADIO_TX,
                "Condition 6B: All antennas deployed [BUS_MCU_B]"
            );
            return;
        }

        // Condition 7: If uptime > 4 hours -> NOMINAL_WITH_RADIO_TX
        // Reason: If deployment goes wrong for 3.5 hours, assume the issue is that the "is deployed" sensors failed.
        if (TIM_get_current_system_uptime_ms() > (4 * 60 * 60 * 1000)) {
            set_operation_state_and_log_if_changed(
                CTS1_OPERATION_STATE_NOMINAL_WITH_RADIO_TX,
                "Condition 7: Uptime > 4 hours"
            );
            return;
        }
    }
}


static inline void SUBTASK_deployment_state_execute(void) {
    if (CTS1_operation_state != CTS1_OPERATION_STATE_DEPLOYING) {
        return;
    }

    // Select a bus to use.
    enum ANT_i2c_bus_mcu bus;
    if (HAL_GetTick() % 60000 < 30000) { // Cycle back and forth every 30 seconds.
        bus = ANT_I2C_BUS_A_MCU_A;
    }
    else {
        bus = ANT_I2C_BUS_B_MCU_B;
    }
    LOG_message(
        LOG_SYSTEM_ANTENNA_DEPLOY, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Deploying antennas using bus %c.",
        (bus == ANT_I2C_BUS_A_MCU_A) ? 'A' : 'B'
    );

    // TODO: Validate if we need to attempt to enter EPS nominal mode here.
    
    // Enable power to the antennas.
    EPS_set_channel_enabled(EPS_CHANNEL_3V3_UHF_ANTENNA_DEPLOY, 1);
    HAL_Delay(ANT_POWER_ON_BOOTUP_DURATION_MS);

    // Arm the deployment system.
    ANT_CMD_arm_antenna_system(bus); // Steamroll.

    // Deploy the antennas.
    ANT_CMD_start_automated_sequential_deployment(bus, 30); // Steamroll.
}


static void pulse_external_led_blocking(uint32_t pulse_duration_ms) {
    OBC_set_external_led(1);
    HAL_Delay(pulse_duration_ms);
    OBC_set_external_led(0);
}

/// @brief Do the LED indication action for the current operation state, and return the LED operation period.
/// @return The time to yield for, in milliseconds.
static inline uint32_t SUBTASK_bootup_operation_state_do_led_indication_action_return_yield_time_ms(void) {
    // Note: We return the yield time to ensure that the LED indication task runs at a fixed rate,
    // regardless of how long the other SUBTASK functions take.
    if (CTS1_operation_state == CTS1_OPERATION_STATE_BOOTED_AND_WAITING) {
        if ((TIM_get_current_system_uptime_ms() / 1000) < (COMMS_uptime_to_start_ant_deployment_sec - (5 * 60))) {
            // LED Indicator: From boot until 25 minutes uptime (5 minutes before COMMS_uptime_to_start_ant_deployment_sec), external LED pulses 40ms per 1000ms.
            pulse_external_led_blocking(40);
            
            return (960);
        }
        else {
            // LED Indicator: From 25 minutes uptime to 30 minutes uptime, external LED blinks 40ms per 333ms
            // (warning indicator that the satellite is about to deploy its antenna).
            pulse_external_led_blocking(40);

            return (333);
        }
    }
    else if (CTS1_operation_state == CTS1_OPERATION_STATE_DEPLOYING) {
        // LED Indicator: Solid on.
        OBC_set_external_led(1);

        // Only restart attempting to deploy every 30 seconds.
        return (30000);
    }
    else if (CTS1_operation_state == CTS1_OPERATION_STATE_NOMINAL_WITH_RADIO_TX) {
        // LED Indicator: Pulses once per 10 seconds.
        pulse_external_led_blocking(40);

        return (10000);
    }
    else if (CTS1_operation_state == CTS1_OPERATION_STATE_NOMINAL_WITHOUT_RADIO_TX) {
        // LED Indicator: Pulses twice per 3 seconds.
        pulse_external_led_blocking(40);
        HAL_Delay(180);
        pulse_external_led_blocking(40);
        
        return (3000);
    }
    else {
        // Safety: Warn, then do RTOS yield.
        LOG_message(
            LOG_SYSTEM_OBC, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Unknown/invalid CTS1 operation state. Current state: %d",
            CTS1_operation_state
        );
        return 10;
    }
    return 10;
}

/// @brief FreeRTOS task - FSM which handles the bootup and operation state/state.
/// @details During LEOPS (launch and early operations), it deploys the comms antennas and permits radio TX.
///          During the rest of the mission, it bypasses the 30 minute deployment wait time, and also
///          blinks the LED to indicate the current state.
/// @note Implemented per https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/420
void TASK_bootup_operation_fsm(void *argument) {
    TASK_HELP_start_of_task();

    // Blink the LED a few times to show that the boot just happened.
    // The blink slowly gets longer in a distinctive pattern intentionally.
    for (uint8_t i = 0; i < 12; i++) {
        OBC_set_external_led((i+1) % 2);
        HAL_Delay(100 + (i*25));
    }

    // Wait for the system to be ready.
    osDelay(1000);

    // Main loop.
    while (1) {
        const uint32_t task_entry_ms = HAL_GetTick();

        SUBTASK_bootup_operation_state_check_for_state_transitions();
        const uint32_t yield_time_ms = SUBTASK_bootup_operation_state_do_led_indication_action_return_yield_time_ms();

        if (CTS1_operation_state == CTS1_OPERATION_STATE_DEPLOYING) {
            SUBTASK_deployment_state_execute();
        }

        // Yield for the LED indication time.
        const uint32_t yield_until_time = task_entry_ms + yield_time_ms;
        if (yield_until_time > HAL_GetTick()) {
            osDelayUntil(yield_until_time);
        }
        else {
            // If the yield time is already past, just yield for 10ms.
            osDelay(10);
        }
    }
}


char* CTS1_operation_state_enum_TO_str(CTS1_operation_state_enum_t state) {
    switch (state) {
        case CTS1_OPERATION_STATE_BOOTED_AND_WAITING:
            return "BOOTED_AND_WAITING";
        case CTS1_OPERATION_STATE_DEPLOYING:
            return "DEPLOYING";
        case CTS1_OPERATION_STATE_NOMINAL_WITH_RADIO_TX:
            return "NOMINAL_WITH_RADIO_TX";
        case CTS1_OPERATION_STATE_NOMINAL_WITHOUT_RADIO_TX:
            return "NOMINAL_WITHOUT_RADIO_TX";
        default:
            return "UNKNOWN";
    }
}