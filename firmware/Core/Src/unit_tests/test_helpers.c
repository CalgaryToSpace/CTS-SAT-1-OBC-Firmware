#include "test_helpers.h"
#include "helpers/helpers.h"
#include "unit_tests/unit_test_helpers.h"

#include <stdint.h>
#include <string.h>

void TEST_HELPER_Split_String_By_Delimiter()
{
    uint8_t *output[10] = {"\0"};

    // Test case 1: Normal case
    const uint8_t *fruits = (uint8_t *)"apple,banana,cherry,date\0";
    split_string_by_delimiter(fruits, strlen((char *)fruits), ',', output);
    TEST_ASSERT_TRUE(strcmp((char *)output[0], "apple") == 0);
    TEST_ASSERT_TRUE(strcmp((char *)output[1], "banana") == 0);
    TEST_ASSERT_TRUE(strcmp((char *)output[2], "cherry") == 0);
    TEST_ASSERT_TRUE(strcmp((char *)output[3], "date") == 0);

    memset(output, 0, sizeof(output));

    // Test case 2: Empty string
    split_string_by_delimiter((uint8_t *)"", 0, ',', output);
    TEST_ASSERT_TRUE(output[0] == 0);

    memset(output, 0, sizeof(output));

    // Test case 3: Single element
    const uint8_t *single = (uint8_t *)"single\0";
    split_string_by_delimiter(single, strlen((char *)single), ',', output);
    TEST_ASSERT_TRUE(strcmp((char *)output[0], "single") == 0);

    memset(output, 0, sizeof(output));

    // Test case 4: Trailing comma
    const uint8_t *trailing = (uint8_t *)"one,two,\0";
    split_string_by_delimiter(trailing, strlen((char *)trailing), ',', output);
    TEST_ASSERT_TRUE(strcmp((char *)output[0], "one") == 0);
    TEST_ASSERT_TRUE(strcmp((char *)output[1], "two") == 0);

    memset(output, 0, sizeof(output));

    // Test case 5: Leading comma
    const uint8_t *leading = (uint8_t *)",one,two\0";
    split_string_by_delimiter(leading, strlen((char *)leading), ',', output);
    TEST_ASSERT_TRUE(strcmp((char *)output[0], "one") == 0);
    TEST_ASSERT_TRUE(strcmp((char *)output[1], "two") == 0);

    memset(output, 0, sizeof(output));

    // Test case 6: Multiple consecutive commas
    const uint8_t *consecutive = (uint8_t *)"one,,two\0";
    split_string_by_delimiter(consecutive, strlen((char *)consecutive), ',', output);
    TEST_ASSERT_TRUE(strcmp((char *)output[0], "one") == 0);
    TEST_ASSERT_TRUE(strcmp((char *)output[1], "two") == 0);
}