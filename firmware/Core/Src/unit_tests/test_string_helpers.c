
#include "transforms/string_helpers.h"
#include "unit_tests/unit_test_helpers.h"

#include <stdint.h>
#include <string.h>

// Function which tests the split_string_by_delimiter function
uint8_t TEST_EXEC__HELPER_split_string_by_delimiter()
{
    const uint8_t space_delim = ' ';

    // Test 1.1 First iteration, normal case
    char hello_result[20];
    memset(hello_result, 0, 20);
    const uint8_t hello_result_len = strlen("hello\0");
    char *input_str = "hello world\0";
    const uint8_t input_str_len = strlen(input_str);
    const uint8_t hello_res = split_string_by_delimiter(&input_str, input_str_len, space_delim, hello_result);
    TEST_ASSERT_TRUE(hello_res == 0);
    TEST_ASSERT_TRUE(strncmp(hello_result, "hello\0", hello_result_len) == 0);
    TEST_ASSERT_TRUE(strncmp(input_str, "world\0", input_str_len) == 0);

    // Test 1.2 Second iteration
    uint8_t world_result_len = strlen("world\0");
    char world_result[20];
    memset(world_result, 0, 20);
    const uint8_t modified_str_len = strlen(input_str);
    const uint8_t world_res = split_string_by_delimiter(&input_str, modified_str_len, space_delim, world_result);
    TEST_ASSERT_TRUE(world_res == 0);
    TEST_ASSERT_TRUE(strncmp(world_result, "world\0", world_result_len) == 0);
    TEST_ASSERT_TRUE(strncmp(input_str, "\0", 1) == 0);

    // Test 2.1, string has no length
    char *empty_input_str = "\0";
    uint8_t empty_input_str_len = strlen(empty_input_str);
    char empty_result[20];
    memset(empty_result, 0, 20);
    const uint8_t empty_res = split_string_by_delimiter(&empty_input_str, empty_input_str_len, space_delim, empty_result);
    TEST_ASSERT_TRUE(empty_res == 1);
    TEST_ASSERT_TRUE(strncmp(empty_result, "\0", 1) == 0);
    TEST_ASSERT_TRUE(strncmp(empty_input_str, "\0", 1) == 0);

    // Test 3.1, Delimiter at the end of the string
    char *delimiter_at_the_end = "hello,\0";
    uint8_t delimiter_at_the_end_len = strlen(delimiter_at_the_end);
    const uint8_t delimiter_at_the_end_res_len = strlen("hello\0");
    char delimiter_at_the_end_result[20];
    memset(delimiter_at_the_end_result, 0, 20);
    const uint8_t delimiter_at_the_end_res = split_string_by_delimiter(&delimiter_at_the_end, delimiter_at_the_end_len, ',', delimiter_at_the_end_result);
    TEST_ASSERT_TRUE(delimiter_at_the_end_res == 0);
    TEST_ASSERT_TRUE(strncmp(delimiter_at_the_end_result, "hello\0", delimiter_at_the_end_res_len) == 0);
    TEST_ASSERT_TRUE(strncmp(delimiter_at_the_end, "\0", 1) == 0);

    // Test 3.2, Delimiter at the beginning of the string
    char *delimiter_at_the_beginning = ",hello\0";

    const uint8_t delimiter_at_the_beginning_res_len = 0;
    char delimiter_at_the_beginning_result[20];
    memset(delimiter_at_the_beginning_result, 0, 20);

    const uint8_t delimiter_at_the_beginning_res = split_string_by_delimiter(&delimiter_at_the_beginning, strlen(delimiter_at_the_beginning), ',', delimiter_at_the_beginning_result);

    TEST_ASSERT_TRUE(delimiter_at_the_beginning_res == 0);
    TEST_ASSERT_TRUE(strncmp(delimiter_at_the_beginning_result, "\0", delimiter_at_the_beginning_res_len) == 0);
    TEST_ASSERT_TRUE(strncmp(delimiter_at_the_beginning, "hello\0", strlen(delimiter_at_the_beginning)) == 0);

    // Test 3.3, Iteration 2

    const uint8_t modified_delimiter_at_the_beginning_res_len = strlen("hello\0");
    char modified_delimiter_at_the_beginning_result[20];
    memset(modified_delimiter_at_the_beginning_result, 0, 20);
    const uint8_t modified_delimiter_at_the_beginning_res = split_string_by_delimiter(&delimiter_at_the_beginning, strlen(delimiter_at_the_beginning), ',', modified_delimiter_at_the_beginning_result);
    TEST_ASSERT_TRUE(modified_delimiter_at_the_beginning_res == 0);
    TEST_ASSERT_TRUE(strncmp(modified_delimiter_at_the_beginning_result, "hello\0", modified_delimiter_at_the_beginning_res_len) == 0);
    TEST_ASSERT_TRUE(strncmp(delimiter_at_the_beginning, "\0", 1) == 0);

    // Test 4.1, consecutive delimiters
    char *consecutive_delimiters = ",,\0";
    const uint8_t consecutive_delimiters_res_len = 0;
    char consecutive_delimiters_result[20];
    memset(consecutive_delimiters_result, 0, 20);
    const uint8_t consecutive_delimiters_res = split_string_by_delimiter(&consecutive_delimiters, strlen(consecutive_delimiters), ',', consecutive_delimiters_result);
    TEST_ASSERT_TRUE(consecutive_delimiters_res == 0);
    TEST_ASSERT_TRUE(strncmp(consecutive_delimiters_result, "\0", consecutive_delimiters_res_len) == 0);
    TEST_ASSERT_TRUE(strncmp(consecutive_delimiters, ",\0", 1) == 0);

    // Test 4.2, consecutive delimiters iteration 2

    char modified_consectutive_delimiters_result[20];
    memset(modified_consectutive_delimiters_result, 0, 20);
    const uint8_t modified_consectutive_delimiters_res = split_string_by_delimiter(&consecutive_delimiters, strlen(consecutive_delimiters), ',', modified_consectutive_delimiters_result);
    TEST_ASSERT_TRUE(modified_consectutive_delimiters_res == 0);
    TEST_ASSERT_TRUE(strncmp(modified_consectutive_delimiters_result, "\0", 0) == 0);
    TEST_ASSERT_TRUE(strncmp(consecutive_delimiters, "\0", 1) == 0);

    return 0;
}