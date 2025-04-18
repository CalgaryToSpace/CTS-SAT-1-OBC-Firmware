#include "telecommands/comms_telecommand_defs.h"
#include "comms_drivers/comms_drivers.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "cts_csp/cts_comms_tx.h"
#include "main.h" 

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
/// @brief downlinks a string to the ground station via the ax100.
/// @param args_str 
/// - Arg 0: The string to downlink, for now must be less than 220 characters
/// @return  result: 0 on success, failure otherwise
uint8_t TCMDEXEC_comms_downlink_str(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) 
{
    const uint16_t max_len = 255;
    char data[max_len];

    //TODO: check result
    TCMD_extract_string_arg(args_str, 0, data, max_len);


    HAL_I2C_DisableListen_IT(&hi2c1);


    uint8_t result = COMMS_downlink_data((uint8_t*)data, strlen(data)+1);

    HAL_I2C_EnableListen_IT(&hi2c1);
    return result;
}
