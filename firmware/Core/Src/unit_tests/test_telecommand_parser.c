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


uint8_t TEST_EXEC__TCMD_get_index_of_substring_in_array() {
    // Test case 1: Substring is found at the beginning
    const char haystack1[] = "hello world";
    const char *needle1 = "hello";
    TEST_ASSERT(TCMD_get_index_of_substring_in_array(haystack1, 11, needle1) == 0);

    // Test case 2: Substring is found in the middle
    const char *needle2 = "world";
    TEST_ASSERT(TCMD_get_index_of_substring_in_array(haystack1, 11, needle2) == 6);

    // Test case 3: Substring is not found
    const char *needle3 = "test";
    TEST_ASSERT(TCMD_get_index_of_substring_in_array(haystack1, 11, needle3) == -1);

    // Test case 4: Substring is empty
    const char *needle4 = "";
    TEST_ASSERT(TCMD_get_index_of_substring_in_array(haystack1, 11, needle4) == 0);

    // Test case 5: haystack_arr is an array and not a C-string
    const char haystack2[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
    const char *needle5 = "cde";
    TEST_ASSERT(TCMD_get_index_of_substring_in_array(haystack2, 8, needle5) == 2);

    // Test case 6: haystack_arr is an array and not a C-string, and substring is not found
    const char *needle6 = "xyz";
    TEST_ASSERT(TCMD_get_index_of_substring_in_array(haystack2, 8, needle6) == -1);

    // Test case 7: haystack_arr has repeated characters
    const char haystack3[] = "aaaaaa";
    const char *needle7 = "aaa";
    TEST_ASSERT(TCMD_get_index_of_substring_in_array(haystack3, 6, needle7) == 0);

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
