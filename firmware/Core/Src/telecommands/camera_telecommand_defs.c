#include "telecommand_exec/telecommand_args_helpers.h"
#include "telecommands/camera_telecommand_defs.h"
#include "config/configuration.h"
#include "camera/camera_init.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/// @brief Setup camera by changing baudrate from default to 9600
/// @param args_str
/// @param response_output_buf Buffer to write the response to
/// @param response_output_buf_len Max length of the buffer
/// @return 0 if successful, >0 if an error occurred
uint8_t TCMDEXEC_camera_setup(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len)
{
    const uint8_t setup_status = CAM_setup();

    if (setup_status == 1){
    snprintf(response_output_buf, response_output_buf_len, "Error setting up camera (Most likely timeout on receive).  Error code %d\n", setup_status);
    return 1;
    }
    else if (setup_status == 2){
        snprintf(response_output_buf, response_output_buf_len, "Error: Invalid bitrate\n");
        return 1;
        }

    snprintf(response_output_buf, response_output_buf_len, "Successfully changed camera buadrate to 2400\n");
    return 0;
}


/// @brief Test to check if camera is connected
/// @param args_str
/// @param response_output_buf Buffer to write the response to
/// @param response_output_buf_len Max length of the buffer
/// @return 0 if successful, >0 if an error occurred
uint8_t TCMDEXEC_camera_test(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len)
{
    uint8_t setup_status = CAM_test();

    if (setup_status == 1){
        snprintf(response_output_buf, response_output_buf_len, "Error with camera\n");
        return 1;
    }

    snprintf(response_output_buf, response_output_buf_len, "Camera connection ready\n");
    return 0;
}


/// @brief Test to check if camera is connected
/// @param args_str
/// - Arg 1: Baudrate to change to (bits per second)
/// @return 0 if successful, >0 if an error occurred
uint8_t TCMDEXEC_camera_change_baud_rate(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len)
{
    const uint32_t new_baud_rate = atoi(args_str);

    const uint8_t status = CAM_change_baudrate(new_baud_rate);

    if (status != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error changing baudrate to %lu. Error code %d",
            new_baud_rate,
            status
        );
        return status;
    }

    snprintf(
        response_output_buf, response_output_buf_len,
        "Camera baudrate changed to %lu",
        new_baud_rate
    );
    return 0;
}
