#include "unit_tests/unit_test_helpers.h"
#include "unit_tests/unit_test_inventory.h"

#include "telecommands/telecommand_parser.h"

uint8_t TEST_EXEC__TCMD_is_char_alphanumeric() {
    uint8_t result = 0;
    result = TCMD_is_char_alphanumeric('a');
    TEST_ASSERT(result == 1);
    result = TCMD_is_char_alphanumeric('A');
    TEST_ASSERT(result == 1);
    result = TCMD_is_char_alphanumeric('z');
    TEST_ASSERT(result == 1);
    result = TCMD_is_char_alphanumeric('Z');
    TEST_ASSERT(result == 1);
    result = TCMD_is_char_alphanumeric('0');
    TEST_ASSERT(result == 1);
    result = TCMD_is_char_alphanumeric('9');
    TEST_ASSERT(result == 1);
    result = TCMD_is_char_alphanumeric(' ');
    TEST_ASSERT(result == 0);
    result = TCMD_is_char_alphanumeric('!');
    TEST_ASSERT(result == 0);
    result = TCMD_is_char_alphanumeric('a');
    TEST_ASSERT(result == 1);
    result = TCMD_is_char_alphanumeric('a');
    TEST_ASSERT(result == 1);
    result = TCMD_is_char_alphanumeric('a');
    TEST_ASSERT(result == 1);
    result = TCMD_is_char_alphanumeric('a');
    TEST_ASSERT(result == 1);
    result = TCMD_is_char_alphanumeric('a');
    TEST_ASSERT(result == 1);
    result = TCMD_is_char_alphanumeric('a');
    TEST_ASSERT(result == 1);
    result = TCMD_is_char_alphanumeric('a');
    TEST_ASSERT(result == 1);
    result = TCMD_is_char_alphanumeric('a');
    TEST_ASSERT(result == 1);
    result = TCMD_is_char_alphanumeric('a');
    TEST_ASSERT(result == 1);
    result = TCMD_is_char_alphanumeric('a');
    TEST_ASSERT(result == 1);
    return 0;
}
