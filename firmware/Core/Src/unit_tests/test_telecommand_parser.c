#include "unit_tests/unit_test_helpers.h"
#include "telecommands/telecommand_parser.h"

uint8_t TEST_EXEC__TCMD_is_char_alphanumeric() {
    TEST_ASSERT(TCMD_is_char_alphanumeric('a') == 1);
    TEST_ASSERT(TCMD_is_char_alphanumeric('M') == 1);
    TEST_ASSERT(TCMD_is_char_alphanumeric('A') == 1);
    TEST_ASSERT(TCMD_is_char_alphanumeric('z') == 1);
    TEST_ASSERT(TCMD_is_char_alphanumeric('m') == 1);
    TEST_ASSERT(TCMD_is_char_alphanumeric('Z') == 1);
    TEST_ASSERT(TCMD_is_char_alphanumeric('0') == 1);
    TEST_ASSERT(TCMD_is_char_alphanumeric('9') == 1);
    TEST_ASSERT(TCMD_is_char_alphanumeric(' ') == 0);
    TEST_ASSERT(TCMD_is_char_alphanumeric('!') == 0);
    TEST_ASSERT(TCMD_is_char_alphanumeric('@') == 0);
    TEST_ASSERT(TCMD_is_char_alphanumeric('[') == 0);
    TEST_ASSERT(TCMD_is_char_alphanumeric('+') == 0); // important, on left side of command name
    TEST_ASSERT(TCMD_is_char_alphanumeric('(') == 0); // important, on right side of command name
    return 0;
}

uint8_t TEST_EXEC__TCMD_check_starts_with_device_id() {
    uint8_t result = 0;
    result = TCMD_check_starts_with_device_id("CTS1+hello_world", 16);
    TEST_ASSERT(result == 1);
    
    // this case should fail, as the command needs to be longer than just the prefix
    result = TCMD_check_starts_with_device_id("CTS1+", 5);
    TEST_ASSERT(result == 0);

    // test that CTS-SAT-2 won't break our molten satellite
    result = TCMD_check_starts_with_device_id("CTS2+hello_world", 16);
    TEST_ASSERT(result == 0);

    // test shortest allowable command
    result = TCMD_check_starts_with_device_id("CTS1+a", 6);
    TEST_ASSERT(result == 1);

    return 0;
}
