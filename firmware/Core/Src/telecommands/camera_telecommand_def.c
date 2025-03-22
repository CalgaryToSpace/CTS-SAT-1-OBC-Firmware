#include "telecommands/telecommand_args_helpers.h"
#include "telecommands/camera_telecommand_def.h"
#include "config/configuration.h"
#include "debug_tools/debug_uart.h"
#include "camera/camera.h"
#include "camera/camera_bitrate.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>


/// @brief Set an integer configuration variable
/// @param args_str
/// - Arg 0: variable name
/// - Arg 1: new value
/// @param response_output_buf Buffer to write the response to
/// @param response_output_buf_len Max length of the buffer
/// @return 0 if successful, >0 if an error occurred
uint8_t TCMDEXEC_camera_capture(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                         char *response_output_buf, uint16_t response_output_buf_len)
{
    const int args_str_len = strlen(args_str);
    char lighting[2];
    // memset(config_var_name, 0, CONFIG_MAX_VARIABLE_NAME_LENGTH);
    const uint8_t parse_result = TCMD_extract_string_arg(args_str, 0, lighting, sizeof(lighting));
    if (parse_result > 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Could not parse arg 0 for: %s", args_str);
        return 1;
    }

    uint64_t flash_TCMD = 2;
    const uint8_t parse_result2 = TCMD_extract_uint64_arg(args_str, args_str_len, 1, &flash_TCMD);
    if (parse_result2 > 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Could not parse arg 1 for: %s", args_str);
        return 1;
    }
    uint8_t flash = (uint8_t)flash_TCMD;
    if (flash != 1 && flash != 0){
        snprintf(response_output_buf, response_output_buf_len, "Value must be 0 or 1, not %d", flash);
        return 1;
    }

   enum Capture_Status img = CAM_Capture_Image((bool)flash, lighting[0]);

   if (img != Transmit_Success){
        snprintf(response_output_buf, response_output_buf_len, "Wrong lighting input.\n");
        return 1;
   }

    // snprintf(response_output_buf, response_output_buf_len, "SUCCESS: Set config var: %s to: %lu", config_var_name, (uint32_t)config_var_new_value);
    snprintf(response_output_buf, response_output_buf_len, "Successfully captured image\n");
    return 0;
}

/// @brief Setup camera by changing baudrate from default to 9600
/// @param args_str
/// @param response_output_buf Buffer to write the response to
/// @param response_output_buf_len Max length of the buffer
/// @return 0 if successful, >0 if an error occurred
uint8_t TCMDEXEC_camera_setup(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len)
{

uint8_t setup_status = CAM_setup();

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
uint8_t TCMDEXEC_camera_test(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
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