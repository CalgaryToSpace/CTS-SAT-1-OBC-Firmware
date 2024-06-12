#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <stdint.h>

typedef struct
{
    char *variable_name;
    void *variable_pointer; // pointer to a uint8_t, uint16_t, etc.
    uint8_t width_bytes;    // width of the config variable (e.g., 1 for uint8_t, 2 for uint16_t, etc.)
} CONFIG_integer_config_entry_t;

typedef struct
{
    char *variable_name;
    char *variable_pointer;
    uint8_t max_length; // max length, including the null terminator
} CONFIG_string_config_entry_t;

extern CONFIG_integer_config_entry_t CTS1_Integer_Configuration_Variables[];

extern CONFIG_string_config_entry_t CTS1_String_Configuration_Variables[];

#endif