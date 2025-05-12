#include "telecommands/comms_telecommand_defs.h"
#include "comms_drivers/comms_drivers.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "littlefs/littlefs_helper.h"
#include "comms_drivers/bulk_file_downlink.h"

#include <string.h>
#include <stdio.h>

/// @brief Sets the state of the dipole switch on the OBC to either Antenna 1 or Antenna 2.
/// @param args_str 
/// - Arg 0: The state of the dipole switch. Either "1" or "2".
/// @return 
uint8_t TCMDEXEC_comms_dipole_switch_set_state(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    uint64_t antenna_num_u64;

    const uint8_t arg0_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &antenna_num_u64);
   
    if (arg0_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing arguments. Return code: %d",
            arg0_result);
        return 1;
    }

    if (antenna_num_u64 != 1 && antenna_num_u64 != 2) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Invalid antenna number. Must be 1 or 2.");
        return 2;
    }

    COMMS_set_dipole_switch_state((uint8_t)antenna_num_u64);

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
