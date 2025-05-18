#include "comms_drivers/rf_antenna_switch.h"
#include "log/log.h"

#include "adcs_drivers/adcs_types.h"
#include "adcs_drivers/adcs_commands.h"

#include "main.h"

uint8_t COMMS_active_rf_switch_antenna = 1; // Default: LOW. Thus, Antenna 1.
COMMS_rf_switch_control_mode_enum_t COMMS_rf_switch_control_mode = COMMS_RF_SWITCH_CONTROL_MODE_TOGGLE_BEFORE_EVERY_BEACON;

/// @brief If the satellite goes this long without receiving an uplink, the 
///        `COMMS_rf_switch_control_mode` will be set back to `TOGGLE_BEFORE_EVERY_BEACON`.
uint32_t CONFIG_max_duration_without_uplink_before_setting_default_rf_switch_mode_sec = 15 * 60;


/// @brief Sets the state of the dipole switch on the OBC to either Antenna 1 or Antenna 2.
/// @param antenna_num Either 1 or 2. If not 1 or 2, then remains unchanged.
/// @note Specification in https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/417.
void COMMS_set_rf_switch_state(uint8_t antenna_num) {
    const uint8_t starting_antenna_num = COMMS_active_rf_switch_antenna;

    if (antenna_num == 1) {
        HAL_GPIO_WritePin(PIN_UHF_CTRL_OUT_GPIO_Port, PIN_UHF_CTRL_OUT_Pin, GPIO_PIN_RESET);
        COMMS_active_rf_switch_antenna = 1;
    }
    else if (antenna_num == 2) {
        HAL_GPIO_WritePin(PIN_UHF_CTRL_OUT_GPIO_Port, PIN_UHF_CTRL_OUT_Pin, GPIO_PIN_SET);
        COMMS_active_rf_switch_antenna = 2;
    }
    else {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Invalid antenna number %d. Must be 1 or 2.",
            antenna_num
        );
        return;
    }

    // Log on the new antenna number if it's updated.
    if (COMMS_active_rf_switch_antenna != starting_antenna_num) {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
            "Dipole switch changed to ANT%d",
            COMMS_active_rf_switch_antenna
        );
    }
}


/// @brief Determines the optimal antenna to use based on the satellite's attitude angles.
/// @return 1 if Antenna 1 is optimal, 2 if Antenna 2 is optimal, 0 if an error occurs in retrieving attitude data.
/// @note Utilizes estimated roll and pitch angles from the ADCS to make a decision on antenna orientation.
/// @note Angles were based of of the following logic -> https://docs.google.com/document/d/1kpgU5hM9LwyNtvdzwdpbq_XNiqCRhPLB0nauKUpm5I4/edit?tab=t.0
uint8_t COMMS_find_optimal_antenna_using_adcs() {
    ADCS_estimated_attitude_angles_struct_t output_struct;
    const uint8_t tlm_status = ADCS_get_estimated_attitude_angles(&output_struct);
    if (tlm_status != 0) {
        return 0;
    }

    int32_t roll_angle_mdeg = output_struct.estimated_roll_angle_mdeg;

    // Check if the roll angle is in the range of 0-360 degrees.
    if (roll_angle_mdeg < 0) {
        roll_angle_mdeg += 360000;
    }

    if (roll_angle_mdeg > 360000) {
        // Roll angle is out of bounds. No reasonable way to interpret this.
        return 0;
    }

    // Use threshold values to determine the optimal antenna.
    if ((roll_angle_mdeg < 45000 || roll_angle_mdeg > 315000)
        || (roll_angle_mdeg > 135000 && roll_angle_mdeg < 225000)
    ) {
        return 2;
    }
    return 1;
}

/// @brief Toggles the active antenna (selected by RF switch), from 1 to 2 or from 2 to 1.
/// @note This function can be used to manually toggle the active antenna, regardless of the result of COMMS_find_optimal_antenna.
void COMMS_toggle_rf_switch_antenna() {
    if (COMMS_active_rf_switch_antenna == 1) {
        COMMS_set_rf_switch_state(2);
    }
    else {
        COMMS_set_rf_switch_state(1);
    }
}

COMMS_rf_switch_control_mode_enum_t COMMS_rf_switch_control_mode_enum_from_string(const char *str) {
    if ((strcasecmp(str, "TOGGLE_BEFORE_EVERY_BEACON") == 0)
        || (strcasecmp(str, "TOGGLE") == 0)
        || (strcasecmp(str, "0") == 0)
        || (strcasecmp(str, "DEFAULT") == 0)
    ) {
        return COMMS_RF_SWITCH_CONTROL_MODE_TOGGLE_BEFORE_EVERY_BEACON;
    }
    else if ((strcasecmp(str, "FORCE_ANT1") == 0)
        || (strcasecmp(str, "FORCE_ANT_1") == 0)
        || (strcasecmp(str, "ANT1") == 0)
        || (strcasecmp(str, "ANT_1") == 0)
        || (strcasecmp(str, "ANTENNA_1") == 0)
        || (strcasecmp(str, "ANTENNA1") == 0)
        || (strcasecmp(str, "1") == 0)
    ) {
        return COMMS_RF_SWITCH_CONTROL_MODE_FORCE_ANT1;
    }
    else if ((strcasecmp(str, "FORCE_ANT2") == 0)
        || (strcasecmp(str, "FORCE_ANT_2") == 0)
        || (strcasecmp(str, "ANT2") == 0)
        || (strcasecmp(str, "ANT_2") == 0)
        || (strcasecmp(str, "ANTENNA_2") == 0)
        || (strcasecmp(str, "ANTENNA2") == 0)
        || (strcasecmp(str, "2") == 0)
    ) {
        return COMMS_RF_SWITCH_CONTROL_MODE_FORCE_ANT2;
    }
    else if ((strcasecmp(str, "USE_ADCS_NORMAL") == 0)
        || (strcasecmp(str, "USE_ADCS") == 0)
        || (strcasecmp(str, "ADCS") == 0)
        || (strcasecmp(str, "ADCS_NORMAL") == 0)
    ) {
        return COMMS_RF_SWITCH_CONTROL_MODE_USE_ADCS_NORMAL;
    }
    else if ((strcasecmp(str, "USE_ADCS_FLIPPED") == 0)
        || (strcasecmp(str, "USE_ADCS_FLIP") == 0)
        || (strcasecmp(str, "ADCS_FLIPPED") == 0)
        || (strcasecmp(str, "ADCS_FLIP") == 0)
    ) {
        return COMMS_RF_SWITCH_CONTROL_MODE_USE_ADCS_FLIPPED;
    }
    else {
        return COMMS_RF_SWITCH_CONTROL_MODE_UNKNOWN; // Default.
    }
}