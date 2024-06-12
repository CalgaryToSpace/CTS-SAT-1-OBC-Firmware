#include "configuration/configuration.h"
#include "eps_config.h"

// extern
CONFIG_integer_config_entry_t CTS1_Integer_Configuration_Variables[] = {
    {

        .variable_name = "test_integer_config_eps",
        .variable_pointer = &test_integer_config_eps,
        .width_bytes = sizeof(test_integer_config_eps)

    }

};

// extern
CONFIG_string_config_entry_t CTS1_String_Configuration_Variables[] = {
    {.max_length = 10,
     .variable_name = "test_string_config_eps",
     .variable_pointer = &test_string_config_eps}

};
