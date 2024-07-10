#include "configuration/configuration.h"
#include "eps/eps_config.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

// extern
CONFIG_integer_config_entry_t CTS1_Integer_Configuration_Variables[] = {
    {

        .variable_name = "test_integer_config_eps",
        .variable_pointer = &test_integer_config_eps,
        .width_bytes = sizeof(test_integer_config_eps)

    }

};

// extern
const uint8_t CTS1_Integer_Configuration_Variables_Count = sizeof(CTS1_Integer_Configuration_Variables) / sizeof(CONFIG_integer_config_entry_t);

// extern
CONFIG_string_config_entry_t CTS1_String_Configuration_Variables[] = {
    {

        .max_length = 10,
        .variable_name = "test_string_config_eps",
        .variable_pointer = &test_string_config_eps

    }

};

// extern
const uint8_t CTS1_String_Configuration_Variables_Count = sizeof(CTS1_String_Configuration_Variables) / sizeof(CONFIG_string_config_entry_t);

uint8_t CONFIG_get_config_integer_value(uint8_t width, uint64_t *value, const void *var_ptr)
{
    // ssize_t value = -1;
    switch (width)
    {
    case 1:
        strcat(type_as_str, "uint8_t ");
        *value = *(uint8_t *)var_ptr;
        break;
    case 2:
        strcat(type_as_str, "uint16_t ");
        *value = *(uint16_t *)var_ptr;
        break;
    case 4:
        strcat(type_as_str, "uint32_t ");
        *value = *(uint32_t *)var_ptr;
        break;
    case 8:
        strcat(type_as_str, "uint64_t ");
        *value = *(uint64_t *)var_ptr;
        break;
    default:
        strcat(type_as_str, "unknown ");
        return 1;
        break;
    }
    return 0;
}

/// @brief Populuates result with a table of all integer configuration variables
/// @param result Buffer to populate
/// @return Length of the result as uint16
uint16_t CONFIG_print_integer_table(char *result)
{

    for (uint8_t i = 0; i < CTS1_Integer_Configuration_Variables_Count; i++)
    {
        char index[5];
        snprintf(index, 5, "%d", i);
        strncat(result, index, strlen(index));
        strcat(result, "\t");

        const char *name = CTS1_Integer_Configuration_Variables[i].variable_name;
        const uint8_t width = CTS1_Integer_Configuration_Variables[i].width_bytes;
        const void *variable_pointer = CTS1_Integer_Configuration_Variables[i].variable_pointer;

        strncat(result, name, strlen(name));
        strcat(result, ":\t");
        ssize_t value = -1;
        switch (width)
        {
        case 1:
            strcat(result, "uint8_t ");
            value = *(uint8_t *)variable_pointer;
            break;
        case 2:
            strcat(result, "uint16_t ");
            value = *(uint16_t *)variable_pointer;
            break;
        case 4:
            strcat(result, "uint32_t ");
            value = *(uint32_t *)variable_pointer;
            break;
        case 8:
            strcat(result, "uint64_t ");
            value = *(uint64_t *)variable_pointer;
            break;
        default:
            break;
        }

        char value_str[20];
        snprintf(value_str, 20, "%d\n", value);
        strncat(result, value_str, strlen(value_str));
    }

    strcat(result, "\0");
    return strlen(result);
}

/// @brief Populates result with a table of all string configuration variables
/// @param result Buffer to populate
/// @return length of the result
uint16_t CONFIG_print_string_table(char *result)
{
    for (uint8_t i = 0; i < CTS1_String_Configuration_Variables_Count; i++)
    {
        char index[5];
        snprintf(index, 5, "%d", i);
        strncat(result, index, strlen(index));
        strcat(result, "\t");

        const char *name = CTS1_String_Configuration_Variables[i].variable_name;
        const uint8_t length = CTS1_String_Configuration_Variables[i].max_length;
        const char **value = CTS1_String_Configuration_Variables[i].variable_pointer;

        strncat(result, name, strlen(name));
        strcat(result, ":\t");

        strncat(result, *value, length);
        strcat(result, "\n\n");
    }

    strcat(result, "\0");
    return strlen(result);
}

/// @brief Finds an integer configuration variable by name
/// @param name Name of the variable
/// @param result Buffer to populate
/// @return 0 if found, 1 if not found
uint8_t CONFIG_find_integer_variable_by_name(const char *name, const uint8_t str_len, CONFIG_integer_config_entry_t *result)
{
    for (uint8_t i = 0; i < CTS1_Integer_Configuration_Variables_Count; i++)
    {
        const char *current_name = CTS1_Integer_Configuration_Variables[i].variable_name;
        if (strncmp(name, current_name, str_len) == 0)
        {
            result = &CTS1_Integer_Configuration_Variables[i];
            return 0;
        }
    }
    return 1;
}
/// @brief Finds a string configuration variable by name
/// @param name Name of the variable
/// @param result Buffer to populate
/// @return 0 if found, 1 if not found
uint8_t CONFIG_find_string_variable_by_name(const char *name, CONFIG_string_config_entry_t *result)
{
    for (uint8_t i = 0; i < CTS1_String_Configuration_Variables_Count; i++)
    {
        const char *current_name = CTS1_String_Configuration_Variables[i].variable_name;
        if (strcmp(name, current_name) == 0)
        {
            result = &CTS1_String_Configuration_Variables[i];

            return 0;
        }
    }
    return 1;
}
