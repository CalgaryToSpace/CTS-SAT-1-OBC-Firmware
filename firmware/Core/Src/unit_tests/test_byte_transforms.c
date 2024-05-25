// File: unit_tests/test_bytes_transforms.c

#include "unit_tests/unit_test_helpers.h" // for all unit tests
#include "transforms/byte_transforms.h" // for the Function-Under-Test

uint8_t TEST_EXEC__GEN_convert_big_endian_four_byte_array_to_uint32() {
    uint8_t test_array_1[4] = {0x12, 0x34, 0x56, 0x78};
    uint32_t expected_result_1 = 0x12345678;
    TEST_ASSERT_TRUE(GEN_convert_big_endian_four_byte_array_to_uint32(test_array_1) == expected_result_1);

    uint8_t test_array_2[4] = {0xFE, 0xDC, 0xBA, 0x98};
    uint32_t expected_result_2 = 4275878552; // 0xFEDCBA98
    TEST_ASSERT_TRUE(GEN_convert_big_endian_four_byte_array_to_uint32(test_array_2) == expected_result_2);

    return 0;
}
