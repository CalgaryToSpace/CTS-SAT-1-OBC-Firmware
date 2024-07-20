#include "main.h"

#include "telecommands/antenna_telecommand_defs.h"
#include "antenna_deploy_drivers/ant_commands.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"
#include "telecommands/telecommand_args_helpers.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "inttypes.h"

/// @brief Telecommand: Arm the antenna deploy system
/// @param args_str no args
/// @retval 0 if successful, >0 if error occurred
uint8_t TCMDEXEC_ant_arm_antenna_system(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    const uint8_t comms_err = ANT_CMD_arm_antenna_system();
    if (comms_err != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: %d", comms_err);
    } else {
        snprintf(response_output_buf, response_output_buf_len, "Success");
    }
    return 0;
}

uint8_t TCMDEXEC_ant_deploy_antenna1(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    uint64_t arg_activation_time;
    const uint8_t parse_activation_time_result = TCMD_extract_uint64_arg((char*)args_str, strlen((char*)args_str),  0, &arg_activation_time);
    if (parse_activation_time_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing activation time arg: %d", parse_activation_time_result);
        return 3;
    }
    if(arg_activation_time > 255) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error: Activation time must be less than 255");
        return 4;
    }

    const uint8_t comms_err = ANT_CMD_deploy_antenna1((uint8_t)arg_activation_time);
    if (comms_err != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: %d", comms_err);
    } else {
        snprintf(response_output_buf, response_output_buf_len, "Success");
    }
    return 0;
}

uint8_t TCMDEXEC_ant_measure_temp(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    const uint8_t comms_err = ANT_CMD_measure_temp();
    if (comms_err != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: %d", comms_err);
    } else {
        snprintf(response_output_buf, response_output_buf_len, "Success");
    }
    return 0;
}
