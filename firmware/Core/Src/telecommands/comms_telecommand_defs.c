#include "telecommands/comms_telecommand_defs.h"
#include "comms_drivers/rf_antenna_switch.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "log/log.h"

#include <string.h>
#include <stdio.h>

/// @brief Sets the state of the dipole switch on the OBC to either Antenna 1 or Antenna 2.
/// @param args_str 
/// - Arg 0: Enum: "toggle_before_beacon" (default), "ant1", "ant2", "adcs", "adcs_flipped". Case-insensitive.
/// @return 0 on success, 1 on error.
uint8_t TCMDEXEC_comms_set_rf_switch_control_mode(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    // Parse the arguments.
    const COMMS_rf_switch_control_mode_enum_t mode = COMMS_rf_switch_control_mode_enum_from_string(args_str);
    if (mode == COMMS_RF_SWITCH_CONTROL_MODE_UNKNOWN) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error: Invalid RF switch control mode."
        );
        return 1;
    }
    else {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
            "RF switch control mode update: %s -> %s",
            COMMS_rf_switch_control_mode_enum_to_string(COMMS_rf_switch_control_mode), // Existing mode.
            COMMS_rf_switch_control_mode_enum_to_string(mode) // New mode.
        );
    }

    // Set the RF switch control mode.
    COMMS_rf_switch_control_mode = mode;

    // Set the RF switch state based on the new mode (if applicable).
    if (mode == COMMS_RF_SWITCH_CONTROL_MODE_FORCE_ANT1) {
        COMMS_set_rf_switch_state(1);
    }
    else if (mode == COMMS_RF_SWITCH_CONTROL_MODE_FORCE_ANT2) {
        COMMS_set_rf_switch_state(2);
    }

    snprintf(
        response_output_buf, response_output_buf_len,
        "RF switch control mode set to %s (%s).",
        COMMS_rf_switch_control_mode_enum_to_string(mode),

        // Add a trailing note indicating if the switch was set for this reply.
        // Gives operator confidence that, if the message is received, the switch is now in a state that's communicable.
        (mode == COMMS_RF_SWITCH_CONTROL_MODE_FORCE_ANT1 || mode == COMMS_RF_SWITCH_CONTROL_MODE_FORCE_ANT2) ? "and switch set" : "switch not updated yet"
    );

    return 0;
}


/// @brief Gets the current RF switch control mode and the current antenna state.
/// @param args_str No args.
/// @return 
uint8_t TCMDEXEC_comms_get_rf_switch_info(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    const uint8_t adcs_antenna_num = COMMS_find_optimal_antenna_using_adcs();
    
    char* adcs_normal_antenna_num_str = "null"; // Default: JSON null means error reading antenna number.
    char* adcs_flipped_antenna_num_str = "null";
    if (adcs_antenna_num != 0) {
        adcs_normal_antenna_num_str = (adcs_antenna_num == 1) ? "1" : "2";
        adcs_flipped_antenna_num_str = (adcs_antenna_num == 1) ? "2" : "1"; // Flipped suggestion
    }
    
    snprintf(
        response_output_buf, response_output_buf_len,
        "{\"rf_switch_control_mode\":\"%s\",\"active_antenna\":%d,"
        "\"adcs_suggestion_normal\":%s,\"adcs_suggestion_flipped\":%s}",
        COMMS_rf_switch_control_mode_enum_to_string(COMMS_rf_switch_control_mode),
        COMMS_active_rf_switch_antenna,
        adcs_normal_antenna_num_str,
        adcs_flipped_antenna_num_str // Flipped suggestion
    );

    return 0;
}
