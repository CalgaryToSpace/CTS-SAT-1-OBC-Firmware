#include "telecommands/telecommand_args_helpers.h"
#include "telecommands/config_telecommand_defs.h"
#include "config/configuration.h"

#include <string.h>
#include <stdio.h>


/// @brief Set an integer configuration variable
/// @param args_str first argument is the variable name, second argument is the new value
/// @param response_output_buf Buffer to write the response to
/// @param response_output_buf_len Max length of the buffer
/// @return 0 if successful, >0 if an error occurred
uint8_t TCMDEXEC_set_int_config_variable(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                         char *response_output_buf, uint16_t response_output_buf_len)
{
    const int args_str_len = strlen(args_str);
    char config_var_name[CONFIG_MAX_VARIABLE_NAME_LENGTH];
    memset(config_var_name, 0, CONFIG_MAX_VARIABLE_NAME_LENGTH);
    const uint8_t parse_result = TCMD_extract_string_arg(args_str, 0, config_var_name, CONFIG_MAX_VARIABLE_NAME_LENGTH);
    if (parse_result > 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "%s : %s\n", "Could not parse index 0 for: ", args_str);
        return 1;
    }

    uint64_t config_var_new_value;
    const uint8_t parse_result2 = TCMD_extract_uint64_arg(args_str, args_str_len, 1, &config_var_new_value);
    if (parse_result2 > 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "%s : %s\n", "Could not parse index 1 for: ", args_str);
        return 1;
    }

    const uint8_t res = CONFIG_set_int_variable(config_var_name, config_var_new_value);
    if (res > 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Could not set config var: %s\n", config_var_name);
        return 1;
    }

    snprintf(response_output_buf, response_output_buf_len, "SUCCESS: Set config var: %s to: %lu\n", config_var_name, (uint32_t)config_var_new_value);
    return 0;
}

/// @brief Set a string configuration variable
/// @param args_str first argument is the variable name, second argument is the new value
/// @return 0 if successful, >0 if an error occurred
uint8_t TCMDEXEC_set_str_config_variable(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                         char *response_output_buf, uint16_t response_output_buf_len)

{

    char config_var_name[CONFIG_MAX_VARIABLE_NAME_LENGTH];
    memset(config_var_name, 0, CONFIG_MAX_VARIABLE_NAME_LENGTH);
    const uint8_t parse_result = TCMD_extract_string_arg(args_str, 0, config_var_name, CONFIG_MAX_VARIABLE_NAME_LENGTH);
    if (parse_result > 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "%s : %s\n", "Could not parse index 0 for: ", args_str);
        return 1;
    }

    char config_var_new_value[CONFIG_MAX_VARIABLE_NAME_LENGTH];
    memset(config_var_new_value, 0, CONFIG_MAX_VARIABLE_NAME_LENGTH);
    const uint8_t parse_result2 = TCMD_extract_string_arg(args_str, 1, config_var_new_value, CONFIG_MAX_VARIABLE_NAME_LENGTH);
    if (parse_result2 > 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "%s : %s\n", "Could not parse index 1 for: ", args_str);
        return 1;
    }

    const uint8_t res = CONFIG_set_str_variable(config_var_name, config_var_new_value);
    if (res > 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Could not set config var: %s\n", config_var_name);
        return 1;
    }

    snprintf(response_output_buf, response_output_buf_len, "SUCCESS: Set config var: %s to: %s\n", config_var_name, config_var_new_value); 

    return 0;
}

/// @brief Get an integer configuration variable
/// @param args_str variable name
/// @return 0 if successful, >0 if an error occurred
uint8_t TCMDEXEC_get_integer_configuration_variable(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                    char *response_output_buf, uint16_t response_output_buf_len)
{
    const uint16_t res = CONFIG_num_var_to_json(args_str, response_output_buf, response_output_buf_len);
    if (res == 1)
    {
        snprintf(response_output_buf, response_output_buf_len, "%s : %s\n", "Could not find", args_str);
        return 1;
    }
    return 0;

}

/// @brief Get a string configuration variable
/// @param args_str variable name
/// @return 0 if successful, >0 if an error occurred
uint8_t TCMDEXEC_get_string_configuration_variable(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel, char *response_output_buf, uint16_t response_output_buf_len)
{

    const uint16_t res = CONFIG_str_var_to_json(args_str, response_output_buf, response_output_buf_len);
    if (res == 1)
    {
        snprintf(response_output_buf, response_output_buf_len, "%s : %s\n", "Could not find", args_str);
        return 1;
    }
    return 0;
}


/// @brief Get all configuration variables, prints the integer and string variables
/// @param args_str not used, no arguments
/// @return 0 if successful, >0 if an error occurred
uint8_t TCMDEXEC_get_all_configuration_variables(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                 char *response_output_buf, uint16_t response_output_buf_len)
{

    const char *header = "Available Configuration Variables:\n\n";
    const char *integer_header = "Integer Configuration Variables:\n\n";
    snprintf(response_output_buf, response_output_buf_len, "%s%s", header, integer_header);

    char num_print_table[CONFIG_MAX_JSON_STRING_LENGTH];
    num_print_table[0] = '\0';
    for (uint8_t i = 0; i < CONFIG_int_config_variables_count; i++)
    {
        const CONFIG_integer_config_entry_t current_var = CONFIG_int_config_variables[i];
        uint16_t num_table_chars_written = CONFIG_num_var_to_json(current_var.variable_name, num_print_table, CONFIG_MAX_JSON_STRING_LENGTH);
        strncat(response_output_buf, num_print_table, num_table_chars_written);
        num_print_table[0] = '\0';
    }

    strcat(response_output_buf, "\n");

    const char *string_header = "String Configuration Variables:\n\n";
    strcat(response_output_buf, string_header);
    char str_print_table[CONFIG_MAX_JSON_STRING_LENGTH];
    str_print_table[0] = '\0';
    for (uint8_t i = 0; i < CONFIG_str_config_variables_count; i++)
    {
        const CONFIG_string_config_entry_t current_var = CONFIG_str_config_variables[i];
        uint16_t str_table_chars_written = CONFIG_str_var_to_json(current_var.variable_name, str_print_table, CONFIG_MAX_JSON_STRING_LENGTH);
        strncat(response_output_buf, str_print_table, str_table_chars_written);
        str_print_table[0] = '\0';
    }

    return 0;
}
