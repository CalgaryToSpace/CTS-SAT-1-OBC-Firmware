#include "unit_tests/unit_test_helpers.h"
#include "unit_tests/unit_test_inventory.h"

#include "unit_tests/test_assorted_prototypes.h"
#include "unit_tests/test_byte_transforms.h"
#include "unit_tests/test_logging.h"
#include "unit_tests/test_telecommand_parser.h"
#include "unit_tests/test_tests.h"
#include "unit_tests/test_telecommand_arg_helpers.h"
#include "unit_tests/unit_test_helpers.h"
#include "unit_tests/test_configuration_variables.h"


// extern
const TEST_Definition_t TEST_definitions[] = {

    {
        .test_func = TEST_EXEC__test_asserts,
        .test_file = "unit_tests/test_tests",
        .test_func_name = "test_asserts"
    },
    {
        .test_func = TEST_EXEC__this_test_always_fails,
        .test_file = "unit_tests/test_tests",
        .test_func_name = "this_test_always_fails"
    },

    {
        .test_func = TEST_EXEC__TCMD_is_char_alphanumeric,
        .test_file = "telecommands/telecommand_parser",
        .test_func_name = "TCMD_is_char_alphanumeric"
    },
    {
        .test_func = TEST_EXEC__GEN_get_index_of_substring_in_array,
        .test_file = "telecommands/telecommand_parser",
        .test_func_name = "GEN_get_index_of_substring_in_array"
    },
    {
        .test_func = TEST_EXEC__TCMD_check_starts_with_device_id,
        .test_file = "telecommands/telecommand_parser",
        .test_func_name = "TCMD_check_starts_with_device_id"
    },
    {
        .test_func = TEST_EXEC__GEN_convert_big_endian_four_byte_array_to_uint32,
        .test_file = "transforms/byte_transforms",
        .test_func_name = "GEN_convert_big_endian_four_byte_array_to_uint32"
    },
    {
        .test_func = TEST_EXEC__TCMD_get_suffix_tag_uint64,
        .test_file = "telecommands/telecommand_parser",
        .test_func_name = "TCMD_get_suffix_tag_uint64"
    },
    {
        .test_func = TEST_EXEC__TCMD_extract_hex_array_arg,
        .test_file = "telecommands/telecommand_args_helpers",
        .test_func_name = "TCMD_extract_hex_array_arg"
    },
    {
        .test_func = TEST_EXEC__TCMD_ascii_to_double,
        .test_file = "telecommands/telecommand_args_helpers",
        .test_func_name = "TCMD_ascii_to_double"
    },
    {
        .test_func = TEST_EXEC__LOG_enable_disable_LOG_SINK_FILE,
        .test_file = "log/log",
        .test_func_name = "TEST_EXEC__LOG_enable_disable_LOG_SINK_FILE"
    },
    {
        .test_func = TEST_EXEC__LOG_enable_disable_LOG_SINK_UHF_RADIO,
        .test_file = "log/log",
        .test_func_name = "TEST_EXEC__LOG_enable_disable_LOG_SINK_UHF_RADIO"
    },
    {
        .test_func = TEST_EXEC__LOG_enable_disable_LOG_SINK_UMBILICAL_UART,
        .test_file = "log/log",
        .test_func_name = "TEST_EXEC__LOG_enable_disable_LOG_SINK_UMBILICAL_UART"
    },
    {
        .test_func = TEST_EXEC__LOG_enable_disable_LOG_FILE_LOGGING_FOR_SYSTEM_MPI,
        .test_file = "log/log",
        .test_func_name = "TEST_EXEC__LOG_enable_disable_LOG_FILE_LOGGING_FOR_SYSTEM_MPI"
    },
    {
        .test_func = TEST_EXEC__LOG_all_sinks_except,
        .test_file = "log/log",
        .test_func_name = "TEST_EXEC__LOG_all_sinks_except"
    },
    {
        .test_func = TEST_EXEC__setup_configuration_variables,
        .test_file = "configuration/configuration_variables",
        .test_func_name = "TEST_EXEC__setup_configuration_variables"
    },
    {
        .test_func = TEST_EXEC__get_int_var_index,
        .test_file = "configuration/configuration_variables",
        .test_func_name = "TEST_EXEC__get_int_var_index"
    },
    {
        .test_func = TEST_EXEC__get_str_var_index,
        .test_file = "configuration/configuration_variables",
        .test_func_name = "TEST_EXEC__get_str_var_index"
    },
    {
        .test_func = TEST_EXEC__set_int_variable,
        .test_file = "configuration/configuration_variables",
        .test_func_name = "TEST_EXEC__set_int_variable"
    },
    {
        .test_func = TEST_EXEC__set_str_variable,
        .test_file = "configuration/configuration_variables",
        .test_func_name = "TEST_EXEC__set_str_variable"
    },
    {
        .test_func = TEST_EXEC__TCMD_ascii_to_int64,
        .test_file = "telecommands/telecommand_args_helpers",
        .test_func_name = "TCMD_ascii_to_int64"
    },
    {
        .test_func = TEST_EXEC__TCMD_extract_int64_arg,
        .test_file = "telecommands/telecommand_args_helpers",
        .test_func_name = "TCMD_extract_int64_arg"
    },
};

// extern
const int16_t TEST_definitions_count = sizeof(TEST_definitions) / sizeof(TEST_Definition_t);
