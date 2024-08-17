#include "unit_tests/test_configuration_variables.h"
#include "unit_tests/unit_test_helpers.h"
#include "config/configuration.h"

#include <string.h>


uint8_t TEST_EXEC__setup_configuration_variables()
{
    // NOTE: This isn't a stable unit test, but rather a check of the current configuration variables.
    
    // Test that all int variables are <= UINT32_MAX
    // Pretty sure this test is impossible to fail, as the size is literally uint32_t.
    for (uint16_t i = 0; i < CONFIG_int_config_variables_count; i++)
    {
        const CONFIG_integer_config_entry_t current_var = CONFIG_int_config_variables[i];
        TEST_ASSERT(*current_var.num_config_var <= UINT32_MAX);
    }

    // Test that all string variables length <= max length
    // and <= CONFIG_MAX_VARIABLE_NAME_LENGTH.
    // Again, pretty sure this test is impossible to fail.
    for (uint16_t i = 0; i < CONFIG_str_config_variables_count; i++)
    {
        const CONFIG_string_config_entry_t current_var = CONFIG_str_config_variables[i];
        const uint16_t var_len = strlen(current_var.variable_name);
        TEST_ASSERT(var_len <= current_var.max_length);
        TEST_ASSERT(var_len <= CONFIG_MAX_VARIABLE_NAME_LENGTH);
    }

    return 0;
}

uint8_t TEST_EXEC__get_int_var_index()
{
    // Test 1.1 (Failure): Find variable which does not exist.
    TEST_ASSERT(CONFIG_get_int_var_index("yo dog, this shouldn't exist") == -1);

    // Test 1.2 (Success): find variable which does exist.
    const CONFIG_integer_config_entry_t first_var = CONFIG_int_config_variables[0];
    TEST_ASSERT(CONFIG_get_int_var_index(first_var.variable_name) == 0);

    return 0;
}

uint8_t TEST_EXEC__get_str_var_index()
{
    // Test 1.1 (Failure): find variable which does not exist.
    TEST_ASSERT(CONFIG_get_str_var_index("yo dog, this doesn't exist") == -1);

    // Test 1.2 (Success): find variable which does exist.
    const CONFIG_string_config_entry_t first_var = CONFIG_str_config_variables[0];
    TEST_ASSERT(CONFIG_get_str_var_index(first_var.variable_name) == 0);

    return 0;
}

uint8_t TEST_EXEC__set_int_variable()
{
    const CONFIG_integer_config_entry_t first_var = CONFIG_int_config_variables[0];
    // Test 1.1: set variable which does not exist
    // should return 2
    const uint32_t initial_val = *first_var.num_config_var;
    TEST_ASSERT(CONFIG_set_int_variable("yo dog, this doesn't exist", 789) == 2);
    TEST_ASSERT(*first_var.num_config_var == initial_val);

    // Test 1.2: set variable which does exist
    // but value is out of range
    // should return 1
    const uint64_t new_val_too_big = (uint64_t)UINT32_MAX + 1;
    TEST_ASSERT(CONFIG_set_int_variable(first_var.variable_name, new_val_too_big) == 1);
    TEST_ASSERT(*first_var.num_config_var == initial_val);

    // Test 1.3: set variable which does exist
    // should return 0
    const uint32_t new_val = UINT32_MAX - initial_val;
    TEST_ASSERT(new_val != initial_val); // faulty test
    TEST_ASSERT(CONFIG_set_int_variable(first_var.variable_name, new_val) == 0);
    TEST_ASSERT(*first_var.num_config_var != initial_val);
    TEST_ASSERT(*first_var.num_config_var == new_val);
    return 0;
}

uint8_t TEST_EXEC__set_str_variable()
{
    const CONFIG_string_config_entry_t first_var = CONFIG_str_config_variables[0];
    char inital_val[first_var.max_length];
    strncpy(inital_val, first_var.variable_pointer, first_var.max_length);
    inital_val[first_var.max_length - 1] = '\0';

    // Test 1.1 (Failure): Set variable which does not exist.
    TEST_ASSERT(CONFIG_set_str_variable("yo dog, this shouldn't exist", "hello") == 1);
    TEST_ASSERT(strcmp(inital_val, first_var.variable_pointer) == 0);

    // Test 1.2 (Failure): Set variable which does exist but length of new value is out of range.
    const uint32_t double_len = strlen(first_var.variable_name) * 2;
    char new_val_too_big[double_len + 1];
    memset(new_val_too_big, 's', double_len);
    new_val_too_big[double_len] = '\0';
    TEST_ASSERT(CONFIG_set_str_variable(first_var.variable_name, new_val_too_big) == 2);
    TEST_ASSERT(strcmp(inital_val, first_var.variable_pointer) == 0);

    // Test 1.3 (Success): Set variable which does exist.
    char new_val[first_var.max_length - 1];
    memset(new_val, 'a', first_var.max_length - 2);
    new_val[first_var.max_length - 1] = '\0';
    TEST_ASSERT(CONFIG_set_str_variable(first_var.variable_name, new_val) == 0);
    TEST_ASSERT(strcmp(new_val, first_var.variable_pointer) == 0);

    return 0;
}
