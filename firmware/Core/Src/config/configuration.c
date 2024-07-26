#include "configuration.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

uint32_t CONFIG_int_demo_var_1 = 13345;
uint32_t CONFIG_int_demo_var_2 = 242344;
// extern
CONFIG_integer_config_entry_t CONFIG_int_config_variables[] = {
    {
        .variable_name = "CONFIG_int_demo_var_1",
        .num_config_var = &CONFIG_int_demo_var_1,
    },
    {
        .variable_name = "CONFIG_int_demo_var_2",
        .num_config_var = &CONFIG_int_demo_var_2,
    }

};

// extern
const uint8_t CONFIG_int_config_variables_count = sizeof(CONFIG_int_config_variables) / sizeof(CONFIG_integer_config_entry_t);

char CONFIG_str_demo_var_1[25] = "CONFIG_str_demo_var_1";
char CONFIG_str_demo_var_2[50] = "CONFIG_str_demo_var_2";
// extern
CONFIG_string_config_entry_t CONFIG_str_config_variables[] = {
    {
        .variable_name = "CONFIG_str_demo_var_1",
        .variable_pointer = CONFIG_str_demo_var_1,
        .max_length = sizeof(CONFIG_str_demo_var_1)
    },
    {
        .variable_name = "CONFIG_str_demo_var_2",
        .variable_pointer = CONFIG_str_demo_var_2,
        .max_length = sizeof(CONFIG_str_demo_var_2)
    }

};

// extern
const uint8_t CONFIG_str_config_variables_count = sizeof(CONFIG_str_config_variables) / sizeof(CONFIG_string_config_entry_t);


/// @brief Finds an int config variable in `CONFIG_int_config_variables` and returns its index.
/// @param name Name of the variable being searched, as registered in `CONFIG_int_config_variables`
/// @return -1 if not found, otherwise the index of the variable in `CONFIG_int_config_variables`
int16_t CONFIG_get_int_var_index(const char *search_name)
{
    for (uint8_t i = 0; i < CONFIG_int_config_variables_count; i++)
    {
        if (strcmp(search_name, CONFIG_int_config_variables[i].variable_name) == 0)
        {
            return i;
        }
    }
    return -1;
}

/// @brief Finds a string config variable in `CONFIG_str_config_variables` and returns its index.
/// @param name Name of the variable being searched, as registered in `CONFIG_str_config_variables`
/// @return -1 if not found, otherwise the index of the variable in `CONFIG_str_config_variables`
int16_t CONFIG_get_str_var_index(const char *search_name)
{
    for (uint8_t i = 0; i < CONFIG_str_config_variables_count; i++)
    {
        if (strcmp(search_name, CONFIG_str_config_variables[i].variable_name) == 0)
        {
            return i;
        }
    }
    return -1;
}


/// @brief Assigns a new value to an integer configuration variable
/// @param var_name Name of the variable
/// @param new_value New value
/// @return 0 if success, > 0 if failure
uint8_t CONFIG_set_int_variable(const char *var_name, const uint64_t new_value)
{
    if (new_value > UINT32_MAX)
    {
        return 1;
    }

    const int16_t index = CONFIG_get_int_var_index(var_name);
    if (index < 0)
    {
        return 2;
    }
    CONFIG_integer_config_entry_t config_var = CONFIG_int_config_variables[index];

    *config_var.num_config_var = new_value;
    return 0;
}

/// @brief Assigns a new value to a string configuration variable
/// @param config_var Pointer to the configuration variable
/// @param new_value New value
/// @return 0 if success, > 0 if failure
uint8_t CONFIG_set_str_variable(const char *var_name, const char *new_value)
{
    const int16_t index = CONFIG_get_str_var_index(var_name);
    if (index < 0)
    {
        return 1;
    }
    CONFIG_string_config_entry_t config_var = CONFIG_str_config_variables[index];

    const size_t len = strlen(new_value);

    if ((uint8_t)len > config_var.max_length - 1)
    {
        return 2;
    }
    strncpy(config_var.variable_pointer, new_value, len);
    config_var.variable_pointer[len] = '\0';

    return 0;
}

/// @brief Converts an integer configuration variable to a JSON string
/// @param var_name Name of the variable
/// @param json_str Buffer to write the JSON string to
/// @param json_str_max_len Max length of the buffer
/// @return Length of the JSON string
/// @note The JSON string is in the format: {"name":"var_name","value":value}\n
uint16_t CONFIG_int_var_to_json(const char *var_name, char *json_str, const uint16_t json_str_max_len)
{
    const int16_t index = CONFIG_get_int_var_index(var_name);
    if (index < 0)
    {
        return 1; // the base string for json is at least 27 chars, so this works
    }
    CONFIG_integer_config_entry_t config_var = CONFIG_int_config_variables[index];
    return snprintf(
        json_str, json_str_max_len, "{\"name\":\"%s\",\"value\":%lu}\n",
        config_var.variable_name, *config_var.num_config_var);
}

/// @brief Converts a string configuration variable to a JSON string
/// @param var_name Name of the variable
/// @param json_str Buffer to write the JSON string to
/// @param json_str_max_len Max length of the buffer
/// @return Length of the JSON string
/// @note The JSON string is in the format: {"name":"var_name","value":"value"}\n
uint16_t CONFIG_str_var_to_json(const char *var_name, char *json_str, const uint16_t json_str_max_len)
{
    const int16_t index = CONFIG_get_str_var_index(var_name);
    if (index < 0)
    {
        return 1; // the base string for json is at least 28 chars, so this works
    }
    CONFIG_string_config_entry_t config_var = CONFIG_str_config_variables[index];

    return snprintf(
        json_str, json_str_max_len, "{\"name\":\"%s\",\"value\":\"%s\"}\n",
        config_var.variable_name, config_var.variable_pointer);
}
