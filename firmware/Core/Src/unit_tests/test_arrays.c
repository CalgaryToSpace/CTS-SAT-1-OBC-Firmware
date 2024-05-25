#include "unit_tests/unit_test_helpers.h"
#include "unit_tests/test_assorted_prototypes.h"
#include "transforms/arrays.h"

uint8_t TEST_EXEC__GEN_get_index_of_substring_in_array() {
    // Test case 1: Substring is found at the beginning
    const char haystack1[] = "hello world";
    const char *needle1 = "hello";
    TEST_ASSERT(GEN_get_index_of_substring_in_array(haystack1, 11, needle1) == 0);

    // Test case 2: Substring is found in the middle
    const char *needle2 = "world";
    TEST_ASSERT(GEN_get_index_of_substring_in_array(haystack1, 11, needle2) == 6);

    // Test case 3: Substring is not found
    const char *needle3 = "test";
    TEST_ASSERT(GEN_get_index_of_substring_in_array(haystack1, 11, needle3) == -1);

    // Test case 4: Substring is empty
    const char *needle4 = "";
    TEST_ASSERT(GEN_get_index_of_substring_in_array(haystack1, 11, needle4) == 0);

    // Test case 5: haystack_arr is an array and not a C-string
    const char haystack2[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
    const char *needle5 = "cde";
    TEST_ASSERT(GEN_get_index_of_substring_in_array(haystack2, 8, needle5) == 2);

    // Test case 6: haystack_arr is an array and not a C-string, and substring is not found
    const char *needle6 = "xyz";
    TEST_ASSERT(GEN_get_index_of_substring_in_array(haystack2, 8, needle6) == -1);

    // Test case 7: haystack_arr has repeated characters
    const char haystack3[] = "aaaaaa";
    const char *needle7 = "aaa";
    TEST_ASSERT(GEN_get_index_of_substring_in_array(haystack3, 6, needle7) == 0);

    return 0;
}
