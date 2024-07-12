#ifndef __INCLUDE_GUARD__CONFIGURATION_H
#define __INCLUDE_GUARD__CONFIGURATION_H

#include <stdint.h>
#include <stdlib.h>

typedef struct
{
    const char *variable_name;
    uint32_t *num_config_var;
} CONFIG_integer_config_entry_t;

typedef struct
{
    const char *variable_name;
    char *variable_pointer;
    const uint8_t max_length; // max length, including the null terminator
} CONFIG_string_config_entry_t;

static const uint8_t CONFIG_MAX_VARIABLE_NAME_LENGTH = 50;

static const uint8_t CONFIG_MAX_JSON_STRING_LENGTH = UINT8_MAX;

// extern
extern CONFIG_integer_config_entry_t CONFIG_int_config_variables[];

// extern
extern const uint8_t CONFIG_int_config_variables_count;

// extern
extern CONFIG_string_config_entry_t CONFIG_str_config_variables[];

// extern
extern const uint8_t CONFIG_str_config_variables_count;

uint8_t CONFIG_comapre_search_config_names(const char *search_name, const uint16_t search_name_len, const char *config_name, const uint16_t config_name_len);

int16_t CONFIG_get_int_var_index(const char *search_name, const uint8_t search_name_len);

int16_t CONFIG_get_str_var_index(const char *search_name, const uint8_t search_name_len);

uint8_t CONFIG_get_int_var(const char *name, const uint8_t name_len, CONFIG_integer_config_entry_t **result);

uint8_t CONFIG_get_str_var(const char *name, const uint8_t name_len, CONFIG_string_config_entry_t **result);

uint8_t CONFIG_set_int_variable(const char *var_name, const uint64_t new_value);

uint8_t CONFIG_set_str_variable(const char *var_name, const char *new_value);

uint16_t CONFIG_num_var_to_json(const char *config_var, char *json_str, const uint16_t json_str_max_len);

uint16_t CONFIG_str_var_to_json(const char *var_name, char *json_str, const uint16_t json_str_max_len);
#endif //  __INCLUDE_GUARD__CONFIGURATION_H