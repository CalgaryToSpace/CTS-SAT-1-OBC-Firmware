#include "telecommands/telecommand_args_helpers.h"
#include "telecommands/config_telecommand_defs.h"
#include "config/configuration.h"
#include "debug_tools/debug_uart.h"

#include <string.h>
#include <stdio.h>
#include "log/log.h"

/// @brief Set an integer configuration variable
/// @param args_str
/// - Arg 0: variable name
/// - Arg 1: new value
/// @param response_output_buf Buffer to write the response to
/// @param response_output_buf_len Max length of the buffer
/// @return 0 if successful, >0 if an error occurred
uint8_t TCMDEXEC_config_set_int_var(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                    char *response_output_buf, uint16_t response_output_buf_len)
{
    const int args_str_len = strlen(args_str);
    char config_var_name[CONFIG_MAX_VARIABLE_NAME_LENGTH];
    memset(config_var_name, 0, CONFIG_MAX_VARIABLE_NAME_LENGTH);
    const uint8_t parse_result = TCMD_extract_string_arg(args_str, 0, config_var_name, CONFIG_MAX_VARIABLE_NAME_LENGTH);
    if (parse_result > 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Could not parse arg 0 for: %s", args_str);
        return 1;
    }

    uint64_t config_var_new_value;
    const uint8_t parse_result2 = TCMD_extract_uint64_arg(args_str, args_str_len, 1, &config_var_new_value);
    if (parse_result2 > 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Could not parse arg 1 for: %s", args_str);
        return 1;
    }

    const uint8_t res = CONFIG_set_int_variable(config_var_name, config_var_new_value);
    if (res > 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Could not set config var: %s", config_var_name);
        return 1;
    }

    snprintf(response_output_buf, response_output_buf_len, "SUCCESS: Set config var: %s to: %lu", config_var_name, (uint32_t)config_var_new_value);
    return 0;
}

/// @brief Set a string configuration variable
/// @param args_str
/// - Arg 0: variable name
/// - Arg 1: new value
/// @return 0 if successful, >0 if an error occurred
uint8_t TCMDEXEC_config_set_str_var(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                    char *response_output_buf, uint16_t response_output_buf_len)

{

    char config_var_name[CONFIG_MAX_VARIABLE_NAME_LENGTH];
    memset(config_var_name, 0, CONFIG_MAX_VARIABLE_NAME_LENGTH);
    const uint8_t parse_result = TCMD_extract_string_arg(args_str, 0, config_var_name, CONFIG_MAX_VARIABLE_NAME_LENGTH);
    if (parse_result > 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Could not parse arg 0 for: %s", args_str);
        return 1;
    }

    char config_var_new_value[CONFIG_MAX_VARIABLE_NAME_LENGTH];
    memset(config_var_new_value, 0, CONFIG_MAX_VARIABLE_NAME_LENGTH);
    const uint8_t parse_result2 = TCMD_extract_string_arg(args_str, 1, config_var_new_value, CONFIG_MAX_VARIABLE_NAME_LENGTH);
    if (parse_result2 > 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Could not parse arg 1 for: %s", args_str);
        return 1;
    }

    const uint8_t res = CONFIG_set_str_variable(config_var_name, config_var_new_value);
    if (res > 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Could not set config var: %s", config_var_name);
        return 1;
    }

    snprintf(response_output_buf, response_output_buf_len, "SUCCESS: Set config var: %s to: %s", config_var_name, config_var_new_value);

    return 0;
}

/// @brief Get an integer configuration variable
/// @param args_str
/// - Arg 0: variable name
/// @return 0 if successful, >0 if an error occurred
uint8_t TCMDEXEC_config_get_int_var_json(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                         char *response_output_buf, uint16_t response_output_buf_len)
{
    const uint16_t res = CONFIG_int_var_to_json(args_str, response_output_buf, response_output_buf_len);
    if (res == 1)
    {
        snprintf(response_output_buf, response_output_buf_len, "Could not find variable '%s'", args_str);
        return 1;
    }
    return 0;
}

/// @brief Get a string configuration variable
/// @param args_str
/// - Arg 0: variable name
/// @return 0 if successful, >0 if an error occurred
uint8_t TCMDEXEC_config_get_str_var_json(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel, char *response_output_buf, uint16_t response_output_buf_len)
{

    const uint16_t res = CONFIG_str_var_to_json(args_str, response_output_buf, response_output_buf_len);
    if (res == 1)
    {
        snprintf(response_output_buf, response_output_buf_len, "Could not find variable '%s'", args_str);
        return 1;
    }
    return 0;
}

/// @brief Get all configuration variables, as JSON. One variable per line.
/// @param args_str No arguments.
/// @return 0 if successful, >0 if an error occurred
uint8_t TCMDEXEC_config_get_all_vars_jsonl(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                           char *response_output_buf, uint16_t response_output_buf_len)
{
    char json_str[CONFIG_MAX_JSON_STRING_LENGTH];
    for (uint8_t i = 0; i < CONFIG_int_config_variables_count; i++)
    {
        CONFIG_int_var_to_json(
            CONFIG_int_config_variables[i].variable_name,
            json_str,
            sizeof(json_str));
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "%s, ", json_str);
    }

    for (uint8_t i = 0; i < CONFIG_str_config_variables_count; i++)
    {
        CONFIG_str_var_to_json(
            CONFIG_str_config_variables[i].variable_name,
            json_str,
            sizeof(json_str));
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "%s, ", json_str);
    }

    snprintf(
        response_output_buf, response_output_buf_len,
        "SUCCESS: Showed all config variables (%d int, %d str)",
        CONFIG_int_config_variables_count,
        CONFIG_str_config_variables_count);

    return 0;
}
