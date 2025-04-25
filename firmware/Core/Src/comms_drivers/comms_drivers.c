#include "comms_drivers/comms_drivers.h"
#include "log/log.h"
#include "main.h"
#include "string.h"

#include "adcs_drivers/adcs_types.h"
#include "adcs_drivers/adcs_commands.h"

uint8_t COMMS_current_active_antenna = 2; // Default: LOW. Thus, Antenna 2.
COMMS_antenna_selection_mode_enum_t COMMS_current_ant_mode = COMMS_ANTENNA_SELECTION_MODE_USE_ADCS;

/// @brief Sets the state of the dipole switch on the OBC to either Antenna 1 or Antenna 2.
/// @param dipole_switch_antenna_num Either 1 or 2. If not 1 or 2, then remains unchanged.
/// @note Created per https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/226.
void COMMS_set_dipole_switch_state(uint8_t dipole_switch_antenna_num) {
    if (dipole_switch_antenna_num == 1) {
        HAL_GPIO_WritePin(PIN_UHF_CTRL_OUT_GPIO_Port, PIN_UHF_CTRL_OUT_Pin, GPIO_PIN_SET);
        COMMS_current_active_antenna = 1;
    }
    else if (dipole_switch_antenna_num == 2) {
        HAL_GPIO_WritePin(PIN_UHF_CTRL_OUT_GPIO_Port, PIN_UHF_CTRL_OUT_Pin, GPIO_PIN_RESET);
        COMMS_current_active_antenna = 2;
    }
    else {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Invalid antenna number. Must be 1 or 2."
        );
    }
}

/// @brief Determines and sets the optimal dipole switch state based on antenna performance.
/// @return The current active antenna number after evaluation and switching (1 or 2). Returns 0 if no optimal antenna is found.
/// @note This function relies on COMMS_find_optimal_antenna to identify the best antenna, which in turn influences the dipole switch state.
uint8_t COMMS_determine_and_update_dipole_antenna_switch() {
    
    const uint8_t COMMS_current_active_antenna = COMMS_find_optimal_antenna();
    if (COMMS_current_active_antenna) {
        COMMS_set_dipole_switch_state(COMMS_current_active_antenna);
    }

    return COMMS_current_active_antenna;
}

/// @brief Determines the optimal antenna to use based on the satellite's attitude angles.
/// @return 1 if Antenna 1 is optimal, 2 if Antenna 2 is optimal, 0 if an error occurs in retrieving attitude data.
/// @note Utilizes estimated roll and pitch angles from the ADCS to make a decision on antenna orientation.
/// @note Angles were based of of the following logic -> https://docs.google.com/document/d/1kpgU5hM9LwyNtvdzwdpbq_XNiqCRhPLB0nauKUpm5I4/edit?tab=t.0
uint8_t COMMS_find_optimal_antenna() {

    ADCS_estimated_attitude_angles_struct_t output_struct;
    const uint8_t tlm_status = ADCS_get_estimated_attitude_angles(&output_struct);
    if (tlm_status != 0) {
        return 0;
    }

    bool dipole_b = (output_struct.estimated_roll_angle_mdeg < 45000 || output_struct.estimated_roll_angle_mdeg > 315000) || (output_struct.estimated_roll_angle_mdeg > 135000 && output_struct.estimated_roll_angle_mdeg < 225000);

    if (dipole_b) {
        return 2;
    }
    return 1;
}

/// @brief Toggles the active antenna, from 1 to 2 or from 2 to 1.
/// @note This function can be used to manually toggle the active antenna, regardless of the result of COMMS_find_optimal_antenna.
void COMMS_toggle_active_antenna() {
    if (COMMS_current_active_antenna == 1) {
        COMMS_set_dipole_switch_state(2);
    }
    else if (COMMS_current_active_antenna == 2) {
        COMMS_set_dipole_switch_state(1);
    }
}


COMMS_antenna_selection_mode_enum_t COMMS_mode_from_str(const char channel_name[]) {
    // Parse Numbers.
    if (strcmp(channel_name, "0") == 0) return COMMS_ANTENNA_SELECTION_MODE_USE_ADCS;
    if (strcmp(channel_name, "1") == 0) return COMMS_ANTENNA_SELECTION_MODE_TOGGLE_BEFORE_EVERY_BEACON;
    if (strcmp(channel_name, "2") == 0) return COMMS_ANTENNA_SELECTION_MODE_OVERRIDE_BY_TELECOMMAND_FOR_30_MINUTES;
    
    // Parse Strings.
    if (strcmp(channel_name, "COMMS_ANTENNA_SELECTION_MODE_USE_ADCS") == 0) return COMMS_ANTENNA_SELECTION_MODE_USE_ADCS;
    if (strcmp(channel_name, "COMMS_ANTENNA_SELECTION_MODE_TOGGLE_BEFORE_EVERY_BEACON") == 0) return COMMS_ANTENNA_SELECTION_MODE_TOGGLE_BEFORE_EVERY_BEACON;
    if (strcmp(channel_name, "COMMS_ANTENNA_SELECTION_MODE_OVERRIDE_BY_TELECOMMAND_FOR_30_MINUTES") == 0) return COMMS_ANTENNA_SELECTION_MODE_OVERRIDE_BY_TELECOMMAND_FOR_30_MINUTES;
    
    return COMMS_ANTENNA_SELECTION_MODE_UNKNOWN;
}