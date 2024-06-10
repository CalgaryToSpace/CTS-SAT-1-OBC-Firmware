
#include "helpers/helpers.h"
#include "unit_tests/unit_test_helpers.h"

#include <stdint.h>
#include <string.h>

// Function which tests the split_string_by_delimiter function
uint8_t TEST_EXEC__HELPER_Split_String_By_Delimiter()
{
    // Test case 1: Normal case
    const uint8_t max_string_num = 4;
    const uint8_t max_string_len = 20;
    uint8_t fruitOutput[max_string_num][max_string_len];
    for (uint8_t i = 0; i < max_string_num; i++)
    {
        memset(fruitOutput[i], 0, sizeof(fruitOutput[i]));
    }
    const uint8_t *fruits = (uint8_t *)"apple,banana,cherry,date\0";
    const uint8_t num_strings_split = split_string_by_delimiter(fruits, strlen((char *)fruits), ',', fruitOutput, max_string_num);
    TEST_ASSERT_TRUE(num_strings_split == 4);
    TEST_ASSERT_TRUE(strncmp((char *)fruitOutput[0], "apple\0", 7) == 0);
    TEST_ASSERT_TRUE(strncmp((char *)fruitOutput[1], "banana\0", 8) == 0);
    TEST_ASSERT_TRUE(strncmp((char *)fruitOutput[2], "cherry\0", 8) == 0);
    TEST_ASSERT_TRUE(strncmp((char *)fruitOutput[3], "date\0", 6) == 0);

    // Test case 2: Empty string
    const char empty[] = "";
    TEST_ASSERT_TRUE(strlen(empty) == 0);
    uint8_t empty_output[max_string_num][max_string_len];
    for (uint8_t i = 0; i < max_string_num; i++)
    {
        memset(empty_output[i], 0, sizeof(empty_output[i]));
    }
    const uint8_t num_strings_split_empty = split_string_by_delimiter((uint8_t *)empty, 0, ',', empty_output, 0);
    TEST_ASSERT_TRUE(num_strings_split_empty == 0);

    for (uint8_t i = 0; i < max_string_num; i++)
    {
        for (uint8_t j = 0; j < max_string_len; j++)
        {
            TEST_ASSERT_TRUE(empty_output[i][j] == 0);
        }
    }

    // Test case 3: Single element
    const uint8_t *single = (uint8_t *)"single\0";
    uint8_t single_output[max_string_num][max_string_len];
    for (uint8_t i = 0; i < max_string_num; i++)
    {
        memset(single_output[i], 0, sizeof(single_output[i]));
    }
    const uint8_t num_strings_split_single = split_string_by_delimiter(single, strlen((char *)single), ',', single_output, 1);
    TEST_ASSERT_TRUE(num_strings_split_single == 1);
    TEST_ASSERT_TRUE(strncmp((char *)single_output[0], "single\0", 8) == 0);

    // Test case 4: Trailing comma
    const uint8_t *trailing = (uint8_t *)"one,two,\0";
    uint8_t trailing_output[max_string_num][max_string_len];
    for (uint8_t i = 0; i < max_string_num; i++)
    {
        memset(trailing_output[i], 0, sizeof(trailing_output[i]));
    }
    const uint8_t num_strings_split_trailing = split_string_by_delimiter(trailing, strlen((char *)trailing), ',', trailing_output, 2);
    TEST_ASSERT_TRUE(num_strings_split_trailing == 2);
    TEST_ASSERT_TRUE(strncmp((char *)trailing_output[1], "two\0", 5) == 0);
    TEST_ASSERT_TRUE(strncmp((char *)trailing_output[0], "one\0", 5) == 0);

    // Test case 5: Leading comma
    const uint8_t *leading = (uint8_t *)",one,two,three\0";
    uint8_t leading_output[max_string_num][max_string_len];
    for (uint8_t i = 0; i < max_string_num; i++)
    {
        memset(leading_output[i], 0, sizeof(leading_output[i]));
    }
    const uint8_t num_strings_split_leading = split_string_by_delimiter(leading, strlen((char *)leading), ',', leading_output, 3);

    TEST_ASSERT_TRUE(num_strings_split_leading == 3);
    TEST_ASSERT_TRUE(strncmp((char *)leading_output[0], "one\0", 5) == 0);
    TEST_ASSERT_TRUE(strncmp((char *)leading_output[1], "two\0", 5) == 0);
    TEST_ASSERT_TRUE(strncmp((char *)leading_output[2], "three\0", 7) == 0);

    // Test case 6: Multiple consecutive commas
    const uint8_t *consecutive = (uint8_t *)"one,,two\0";
    uint8_t consecutive_output[max_string_num][max_string_len];
    for (uint8_t i = 0; i < max_string_num; i++)
    {
        memset(consecutive_output[i], 0, sizeof(consecutive_output[i]));
    }
    const uint8_t num_strings_split_consecutive = split_string_by_delimiter(consecutive, strlen((char *)consecutive), ',', consecutive_output, 2);
    TEST_ASSERT_TRUE(num_strings_split_consecutive == 2);
    TEST_ASSERT_TRUE(strncmp((char *)consecutive_output[0], "one\0", 5) == 0);
    TEST_ASSERT_TRUE(strncmp((char *)consecutive_output[1], "two\0", 5) == 0);

    return 0;
}