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
    uint8_t test_return = CAM_test();

    if (test_return != 0) {
        snprintf(
            response_output_buf, response_output_buf_len, "Camera test failed. CAM_test -> %d",
            test_return
        );
        return test_return;
    }

    snprintf(response_output_buf, response_output_buf_len, "Camera test passed.");
    return 0;
}


/// @brief Debugging only. Set the baud rate of the camera to the specified value. Use `camera_setup` normally.
/// @param args_str
/// - Arg 1: Baudrate to change to (bits per second). 10 options from 1200 to 921600.
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

/// @brief Send telecommand to camera and capture an image. RUN CAM_SETUP BEFORE THIS EVERY TIME!
/// @param args_str
/// - Arg 0: filename to save the image to
/// - Arg 1: lighting mode (single character: d,m,n,s)
/// @param response_output_buf Buffer to write the response to
/// @param response_output_buf_len Max length of the buffer
/// @return 0 if successful, >0 if an error occurred
uint8_t TCMDEXEC_camera_capture(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len)
{
    // FIXME: Extract filename here.

    // Extract arg 0 - single char for lighting mode (d,m,n,s)
    char lighting[2];
    // memset(config_var_name, 0, CONFIG_MAX_VARIABLE_NAME_LENGTH);
    const uint8_t parse_result = TCMD_extract_string_arg(args_str, 1, lighting, sizeof(lighting));
    if (parse_result > 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Could not parse arg 0 for: %s", args_str);
        return 1;
    }

    enum CAM_capture_status_enum img = CAM_capture_image("image.ascii", lighting[0]);

    if (img == CAM_CAPTURE_STATUS_WRONG_INPUT){
        snprintf(response_output_buf, response_output_buf_len, "Wrong lighting input.\n");
        return 1;
    }
    else if (img == CAM_CAPTURE_STATUS_CAPTURE_FAILURE){
        snprintf(response_output_buf, response_output_buf_len, "Camera Capture Failure\n");
        return 1;
    }

    // snprintf(response_output_buf, response_output_buf_len, "SUCCESS: Set config var: %s to: %lu", config_var_name, (uint32_t)config_var_new_value);
    snprintf(response_output_buf, response_output_buf_len, "Successfully captured image\n");
    return 0;
}
