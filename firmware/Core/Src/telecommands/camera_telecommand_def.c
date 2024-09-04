#include "telecommands/telecommand_args_helpers.h"
#include "telecommands/camera_telecommand_def.h"
#include "config/configuration.h"
#include "debug_tools/debug_uart.h"
#include "camera/camera.h"

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

    uint64_t flash = 2;
    const uint8_t parse_result2 = TCMD_extract_uint64_arg(args_str, args_str_len, 1, &flash);
    if (parse_result2 > 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Could not parse arg 1 for: %s", args_str);
        return 1;
    }
    if (flash != 1 && flash != 0){
        snprintf(response_output_buf, response_output_buf_len, "Value must be 0 or 1, not %d", flash);
        return 1;
    }

   enum Capture_Status img = Capture_Image((bool)flash, lighting[0]);

   if (img == Wrong_input){
        snprintf(response_output_buf, response_output_buf_len, "Wrong lighting input.");
        return 1;
   }

    // const uint8_t res = CONFIG_set_int_variable(config_var_name, config_var_new_value);
    // if (res > 0)
    // {
    //     snprintf(response_output_buf, response_output_buf_len, "Could not set config var: %s", config_var_name);
    //     return 1;
    // }

    // snprintf(response_output_buf, response_output_buf_len, "SUCCESS: Set config var: %s to: %lu", config_var_name, (uint32_t)config_var_new_value);

    return 0;
}