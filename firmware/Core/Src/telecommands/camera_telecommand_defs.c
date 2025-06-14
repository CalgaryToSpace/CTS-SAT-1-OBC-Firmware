#include "telecommand_exec/telecommand_args_helpers.h"
#include "telecommands/camera_telecommand_defs.h"
#include "config/configuration.h"
#include "camera/camera_init.h"
#include "camera/camera_capture.h"
#include "log/log.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/// @brief Set up the camera by powering on and changing the baudrate to 230400.
/// @param args_str
/// @param response_output_buf Buffer to write the response to
/// @param response_output_buf_len Max length of the buffer
/// @return 0 if successful, >0 if an error occurred
uint8_t TCMDEXEC_camera_setup(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    const uint8_t setup_status = CAM_setup();

    if (setup_status != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error setting up camera. CAM_setup() -> %d",
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
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len)
{
    uint8_t test_return = CAM_test();

    if (test_return != 0) {
        snprintf(
            response_output_buf, response_output_buf_len, "Camera test failed. CAM_test -> %d",
            test_return
        );
        CAM_repeated_error_log_message();
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
    const char *args_str,
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
/// - Arg 0: filename to save the image to (max 32 chars)
/// - Arg 1: lighting mode (single character: d,m,n,s)
/// @param response_output_buf Buffer to write the response to
/// @param response_output_buf_len Max length of the buffer
/// @return 0 if successful, >0 if an error occurred
uint8_t TCMDEXEC_camera_capture(const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len)
{
    // Extract arg 0 - filename
    // Note: extract_string function null-terminates the string
    char filename[32] = {0};
    const uint8_t parse_result_filename = TCMD_extract_string_arg(args_str, 0, filename, sizeof(filename));
    if (parse_result_filename > 0) {
        snprintf(response_output_buf, response_output_buf_len, "Could not parse filename (arg 0) for: %s: Error:  %u", args_str, parse_result_filename);
        return 1;
    }

    // Extract arg 1 - single char for lighting mode (d,m,n,s)
    char lighting[2];
    const uint8_t parse_result_lighting_mode = TCMD_extract_string_arg(args_str, 1, lighting, sizeof(lighting));
    if (parse_result_lighting_mode > 0) {
        snprintf(response_output_buf, response_output_buf_len, "Could not parse lighting mode (arg 1) for: %s. Error: %u", args_str, parse_result_lighting_mode);
        return 2;
    }

    CAM_capture_status_enum img_status = CAM_capture_image(filename, lighting[0]);

    if (img_status != CAM_CAPTURE_STATUS_TRANSMIT_SUCCESS) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error capturing image. CAM_capture_image() -> %d", img_status);
        CAM_repeated_error_log_message();
        return img_status;
    }

    snprintf(response_output_buf, response_output_buf_len, "Successfully captured image\n");
    return 0;
}
