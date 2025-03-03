// File: unit_tests/test_bytes_transforms.c

#include <string.h>

#include "unit_tests/unit_test_helpers.h" // for all unit tests
#include "transforms/byte_transforms.h" // for the Function-Under-Test
#include "transforms/arrays.h"

uint8_t TEST_EXEC__GEN_convert_big_endian_four_byte_array_to_uint32() {
    uint8_t test_array_1[4] = {0x12, 0x34, 0x56, 0x78};
    uint32_t expected_result_1 = 0x12345678;
    TEST_ASSERT_TRUE(GEN_convert_big_endian_four_byte_array_to_uint32(test_array_1) == expected_result_1);

    uint8_t test_array_2[4] = {0xFE, 0xDC, 0xBA, 0x98};
    uint32_t expected_result_2 = 4275878552; // 0xFEDCBA98
    TEST_ASSERT_TRUE(GEN_convert_big_endian_four_byte_array_to_uint32(test_array_2) == expected_result_2);

    return 0;
}

uint8_t TEST_EXEC__GEN_int64_to_str() {
    char buffer[32];
    int64_t test;

    test = 314159;
    GEN_int64_to_str(test, &buffer[0]);
    TEST_ASSERT_TRUE(strcmp(&buffer[0], "314159") == 0)

    test = 0;
    GEN_int64_to_str(test, &buffer[0]);
    TEST_ASSERT_TRUE(strcmp(&buffer[0], "0") == 0)

    test = -8675309;
    GEN_int64_to_str(test, &buffer[0]);
    TEST_ASSERT_TRUE(strcmp(&buffer[0], "-8675309") == 0)

    return 0;
}