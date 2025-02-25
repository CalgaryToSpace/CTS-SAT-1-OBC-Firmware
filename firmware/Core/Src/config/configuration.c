#include "config/configuration.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

extern uint32_t TASK_heartbeat_period_ms;
extern uint32_t TCMD_require_valid_sha256;
extern uint32_t CONFIG_EPS_enable_uart_debug_print;

uint32_t CONFIG_int_demo_var_1 = 13345;
uint32_t CONFIG_int_demo_var_2 = 242344;

// extern
uint32_t toggleable_channels = 0x0001ffff;
uint32_t watchdog_timer = 7000, EPS_monitor_timer = 1000;       //TODO: Set watchdog timer and EPS monitor timer
uint16_t current_mA_threshhold[32] = {1000}; //TODO: Set current threshold for each channel

// extern
// 1 = require, 0 = don't require
uint32_t TCMD_require_unique_tssent = 0;

/// @brief The percentage of the stack space that should remain free. If the free space falls
/// below this percentage, a warning will be logged.
const uint32_t CONFIG_freertos_min_remaining_stack_percent = 20;

// extern
CONFIG_integer_config_entry_t CONFIG_int_config_variables[] = {
    {
        .variable_name = "CONFIG_int_demo_var_1",
        .num_config_var = &CONFIG_int_demo_var_1,
    },
    {
        .variable_name = "CONFIG_int_demo_var_2",
        .num_config_var = &CONFIG_int_demo_var_2,
    },

    {
        .variable_name = "TASK_heartbeat_period_ms",
        .num_config_var = &TASK_heartbeat_period_ms,
    },
    {
        .variable_name = "TCMD_require_valid_sha256",
        .num_config_var = &TCMD_require_valid_sha256,
    },
    //EPS Power Management Config
    {
        .variable_name = "toggleable_channels",
        .num_config_var = &toggleable_channels,
    },
    //Timer Config
    {
        .variable_name = "watchdog_timer",
        .num_config_var = &watchdog_timer,
    },
    {
        .variable_name = "EPS_monitor_timer",
        .num_config_var = &EPS_monitor_timer,
    },
    {
        .variable_name = "TCMD_require_unique_tssent",
        .num_config_var = &TCMD_require_unique_tssent,
    },

    {
        .variable_name = "CONFIG_EPS_enable_uart_debug_print",
        .num_config_var = &CONFIG_EPS_enable_uart_debug_print,
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
        // Variable not found.
        return 1;
    }
    CONFIG_string_config_entry_t config_var = CONFIG_str_config_variables[index];

    if (strlen(new_value) >= config_var.max_length)
    {
        // New value is too long.
        return 2;
    }
    strcpy(config_var.variable_pointer, new_value);

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
/// @return If successful, Length of the JSON string (>1), otherwise 1
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
