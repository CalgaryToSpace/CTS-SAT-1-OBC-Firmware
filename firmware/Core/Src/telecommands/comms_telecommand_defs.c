#include "telecommands/comms_telecommand_defs.h"
#include "comms_drivers/rf_antenna_switch.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "littlefs/littlefs_helper.h"
#include "comms_drivers/bulk_file_downlink.h"
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

/// @brief Initiate a bulk file downlink over the UHF radio.
/// @param args_str
/// - Arg 0: File path to downlink as string
/// - Arg 1: Start offset in file (uint32)
/// - Arg 2: The maximum number of bytes to downlink. Maximum value is 1000000 (1 MB, COMMS_bulk_file_downlink_total_bytes), for safety (to avoid a very very long-running downlink chain). Values >1 MB will be limited to 1 MB, and value 0 will be set to 1 MB.
/// @return 0 on success. Non-zero on failure. -2 = File does not exist.
/// @note 1 MB takes about 15 minutes to downlink at 9600 baud.
/// @note This function is safe to call at any point (including mid-downlink, or mid-pause).
///       It will close the previous file and start a new downlink.
uint8_t TCMDEXEC_comms_bulk_file_downlink_start(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    const uint8_t args_str_len = strlen(args_str);

    char arg_file_name[LFS_MAX_PATH_LENGTH];
    const uint8_t parse_file_name_result = TCMD_extract_string_arg(
        args_str, 0,
        arg_file_name, sizeof(arg_file_name)
    );
    if (parse_file_name_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing file name arg: TCMD_extract_string_arg() -> %d", parse_file_name_result
        );
        return 1;
    }

    uint64_t start_offset = 0;
    const uint8_t parse_offset_result = TCMD_extract_uint64_arg(args_str, args_str_len, 1, &start_offset);
    if (parse_offset_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing start offset arg: TCMD_extract_uint64_arg(arg=1) -> %d",
            parse_offset_result
        );
        return 2;
    }
    if (start_offset > UINT32_MAX) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error: start offset is too large."
        );
        return 3;
    }

    uint64_t max_bytes = 0;
    const uint8_t parse_max_bytes_result = TCMD_extract_uint64_arg(args_str, args_str_len, 2, &max_bytes);
    if (parse_max_bytes_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing max bytes arg: TCMD_extract_uint64_arg(arg=2) -> %d",
            parse_max_bytes_result
        );
        return 4;
    }
    if (max_bytes > UINT32_MAX) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error: max bytes is too large."
        );
        return 5;
    }

    const int32_t result = COMMS_bulk_file_downlink_start(arg_file_name, start_offset, max_bytes);
    if (result < 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Downlink start failed (LFS error). COMMS_bulk_file_downlink_start() -> %ld",
            result
        );
        return 10;
    }
    else if (result > 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Downlink start failed (non-LFS logical error). COMMS_bulk_file_downlink_start() -> %ld",
            result
        );
        return 11;
    }

    snprintf(
        response_output_buf, response_output_buf_len,
        "Bulk file downlink started successfully!"
    );
    return 0;
}

/// @brief Telecommand: Pause bulk file downlink
/// @param args_str (unused)
uint8_t TCMDEXEC_comms_bulk_file_downlink_pause(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    const uint8_t result = COMMS_bulk_file_downlink_pause();
    if (result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Bulk downlink pause failed. COMMS_bulk_file_downlink_pause() -> %d",
            result
        );
        return result;
    }

    snprintf(
        response_output_buf, response_output_buf_len,
        "Bulk file downlink paused."
    );
    return 0;
}


uint8_t TCMDEXEC_comms_bulk_file_downlink_resume(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    const uint8_t result = COMMS_bulk_file_downlink_resume();
    if (result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Bulk downlink resume failed. COMMS_bulk_file_downlink_resume() -> %d",
            result
        );
        return result;
    }

    snprintf(
        response_output_buf, response_output_buf_len,
        "Bulk file downlink resumed."
    );
    return 0;
}
