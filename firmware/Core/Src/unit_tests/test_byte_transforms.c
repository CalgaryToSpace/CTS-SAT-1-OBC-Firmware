// File: unit_tests/test_bytes_transforms.c

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
    TEST_ASSERT_TRUE(buffer[0] == '3');
    TEST_ASSERT_TRUE(buffer[1] == '1');
    TEST_ASSERT_TRUE(buffer[2] == '4');
    TEST_ASSERT_TRUE(buffer[3] == '1');
    TEST_ASSERT_TRUE(buffer[4] == '5');
    TEST_ASSERT_TRUE(buffer[5] == '9');
    TEST_ASSERT_TRUE(buffer[6] == '\0');

    test = 0;
    GEN_int64_to_str(test, &buffer[0]);
    TEST_ASSERT_TRUE(buffer[0] == '0');
    TEST_ASSERT_TRUE(buffer[1] == '\0');

    test = -8675309;
    GEN_int64_to_str(test, &buffer[0]);
    TEST_ASSERT_TRUE(buffer[0] == '-');
    TEST_ASSERT_TRUE(buffer[1] == '8');
    TEST_ASSERT_TRUE(buffer[2] == '6');
    TEST_ASSERT_TRUE(buffer[3] == '7');
    TEST_ASSERT_TRUE(buffer[4] == '5');
    TEST_ASSERT_TRUE(buffer[5] == '3');
    TEST_ASSERT_TRUE(buffer[6] == '0');
    TEST_ASSERT_TRUE(buffer[7] == '9');
    TEST_ASSERT_TRUE(buffer[8] == '\0');

    return 0;
}