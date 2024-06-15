#include "configuration/configuration.h"
#include "eps_config.h"
#include "configuration.h"

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
