#include "unit_tests/unit_test_helpers.h"
#include "unit_tests/unit_test_inventory.h"

#include "unit_tests/test_telecommand_parser.h"
#include "unit_tests/test_tests.h"


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
    }
    
};

// extern
const int16_t TEST_definitions_count = sizeof(TEST_definitions) / sizeof(TEST_Definition_t);
