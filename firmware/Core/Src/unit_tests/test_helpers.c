
#include "helpers/helpers.h"
#include "unit_tests/unit_test_helpers.h"

#include <stdint.h>
#include <string.h>

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
    TEST_ASSERT_TRUE(strcmp((char *)fruitOutput[0], "apple") == 0);
    TEST_ASSERT_TRUE(strcmp((char *)fruitOutput[1], "banana") == 0);
    TEST_ASSERT_TRUE(strcmp((char *)fruitOutput[2], "cherry") == 0);
    TEST_ASSERT_TRUE(strcmp((char *)fruitOutput[3], "date") == 0);

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

    return 0;

    // memset(output, 0, sizeof(output));

    // // Test case 3: Single element
    // const uint8_t *single = (uint8_t *)"single\0";
    // split_string_by_delimiter(single, strlen((char *)single), ',', output);
    // TEST_ASSERT_TRUE(strcmp((char *)output[0], "single") == 0);

    // memset(output, 0, sizeof(output));

    // // Test case 4: Trailing comma
    // const uint8_t *trailing = (uint8_t *)"one,two,\0";
    // split_string_by_delimiter(trailing, strlen((char *)trailing), ',', output);
    // TEST_ASSERT_TRUE(strcmp((char *)output[0], "one") == 0);
    // TEST_ASSERT_TRUE(strcmp((char *)output[1], "two") == 0);

    // memset(output, 0, sizeof(output));

    // // Test case 5: Leading comma
    // const uint8_t *leading = (uint8_t *)",one,two\0";
    // split_string_by_delimiter(leading, strlen((char *)leading), ',', output);
    // TEST_ASSERT_TRUE(strcmp((char *)output[0], "one") == 0);
    // TEST_ASSERT_TRUE(strcmp((char *)output[1], "two") == 0);

    // memset(output, 0, sizeof(output));

    // // Test case 6: Multiple consecutive commas
    // const uint8_t *consecutive = (uint8_t *)"one,,two\0";
    // split_string_by_delimiter(consecutive, strlen((char *)consecutive), ',', output);
    // TEST_ASSERT_TRUE(strcmp((char *)output[0], "one") == 0);
    // TEST_ASSERT_TRUE(strcmp((char *)output[1], "two") == 0);
}