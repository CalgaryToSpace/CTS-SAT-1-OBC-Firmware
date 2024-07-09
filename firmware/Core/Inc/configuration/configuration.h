#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <stdint.h>
#include <stdlib.h>

typedef struct
{
    const char *variable_name;
    const void *variable_pointer; // pointer to a uint8_t, uint16_t, etc.
    uint8_t width_bytes;          // width of the config variable (e.g., 1 for uint8_t, 2 for uint16_t, etc.)
} CONFIG_integer_config_entry_t;

typedef struct
{
    const char *variable_name;
    char **variable_pointer;
    const uint8_t max_length; // max length, including the null terminator
} CONFIG_string_config_entry_t;

extern CONFIG_integer_config_entry_t CTS1_Integer_Configuration_Variables[];

extern const uint8_t CTS1_Integer_Configuration_Variables_Count;

extern CONFIG_string_config_entry_t CTS1_String_Configuration_Variables[];

extern const uint8_t CTS1_String_Configuration_Variables_Count;

uint8_t CONFIG_get_config_integer_value(uint8_t width, uint64_t *value, const void *var_ptr);

uint16_t CONFIG_print_integer_table(char *result);

uint16_t CONFIG_print_string_table(char *result);

uint8_t CONFIG_find_integer_variable_by_name(const char *name, const uint8_t str_len, CONFIG_integer_config_entry_t *result);

uint8_t CONFIG_find_string_variable_by_name(const char *name, CONFIG_string_config_entry_t *result);
#endif