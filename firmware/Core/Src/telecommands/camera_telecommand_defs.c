#include "telecommand_exec/telecommand_args_helpers.h"
#include "telecommands/camera_telecommand_defs.h"
#include "config/configuration.h"
#include "camera/camera_init.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/// @brief Set up the camera by powering on and changing the baudrate to 2400.
/// @param args_str
/// @param response_output_buf Buffer to write the response to
/// @param response_output_buf_len Max length of the buffer
/// @return 0 if successful, >0 if an error occurred
uint8_t TCMDEXEC_camera_setup(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len)
{
    const uint8_t setup_status = CAM_setup();

    if (setup_status != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error setting up camera.  CAM_setup() -> %d",
            setup_status
        );
        return setup_status;
    }

    snprintf(response_output_buf, response_output_buf_len, "Successfully changed camera baudrate.");
    return 0;
}


/// @brief Send the 't' test command to the camera, and check if the response is valid.
/// @param args_str
/// @param response_output_buf Buffer to write the response to
/// @param response_output_buf_len Max length of the buffer
/// @return 0 if successful, >0 if an error occurred
uint8_t TCMDEXEC_camera_test(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len)
{
    uint8_t setup_status = CAM_test();

    if (setup_status != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error with camera");
        return 1;
    }

    snprintf(response_output_buf, response_output_buf_len, "Camera connection ready");
    return 0;
}


/// @brief Debugging only. Set the baud rate of the camera to the specified value. Use `camera_setup` normally.
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
