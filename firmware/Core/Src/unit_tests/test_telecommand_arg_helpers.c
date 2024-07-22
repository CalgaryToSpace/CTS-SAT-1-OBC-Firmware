#include "telecommands/telecommand_args_helpers.h"
#include "unit_tests/unit_test_helpers.h"

uint8_t TEST_EXEC__TCMD_ascii_to_uint64() {
    uint64_t result;

    // Success: Nominal test
    TEST_ASSERT(TCMD_ascii_to_uint64("123", 3, &result) == 0);
    TEST_ASSERT(result == 123);
/*
    // Error: Empty string
    TEST_ASSERT(TCMD_ascii_to_uint64("", 0, result) == 1);
    TEST_ASSERT(result == 0);

    // Error: String doesn't start with an integer
    TEST_ASSERT(TCMD_ascii_to_uint64("abc123", 6, result) == 2)
    TEST_ASSERT(result == 0);

    // Error: Result length doesn't match str_len
    TEST_ASSERT(TCMD_ascii_to_uint64("123abc", 6, result) == 3);
    TEST_ASSERT(result == 123);

    // Error: String is too long
    TEST_ASSERT(TCMD_ascii_to_uint64("123456789123456789123", 21, result) == 4)
    TEST_ASSERT(result == 0);
*/
    return 0;
}

uint8_t TEST_EXEC__TCMD_extract_hex_array_arg() {
    uint8_t result[4];
    uint16_t res_len;

    // Success: Nominal test.
    TEST_ASSERT(TCMD_extract_hex_array_arg("FFFFFFFF", 0, result, sizeof(result), &res_len) == 0);
    TEST_ASSERT(res_len==4);
    TEST_ASSERT(result[0]==0xFF);
    TEST_ASSERT(result[1]==0xFF);
    TEST_ASSERT(result[2]==0xFF);
    TEST_ASSERT(result[3]==0xFF);

    // Success: input length = 1
    TEST_ASSERT(TCMD_extract_hex_array_arg("12", 0, result, sizeof(result), &res_len) == 0);
    TEST_ASSERT(res_len==1);
    TEST_ASSERT(result[0]==0x12);

    // Error: Output array size exceeded.
    uint8_t seg_result[3];
    TEST_ASSERT(TCMD_extract_hex_array_arg("FFFFFFFF", 0, seg_result, sizeof(seg_result), &res_len) > 0);

    // Error: Separator found in the middle of a byte.
    TEST_ASSERT(TCMD_extract_hex_array_arg("1 234", 0, result, sizeof(result), &res_len) > 0);

    // Success: Delimiters are ignored. Case insensitive.
    TEST_ASSERT(TCMD_extract_hex_array_arg("FF_e2AA 01", 0, result, sizeof(result), &res_len) == 0);
    TEST_ASSERT(res_len==4);
    TEST_ASSERT(result[0]==0xFF);
    TEST_ASSERT(result[1]==0xE2);
    TEST_ASSERT(result[2]==0xAA);
    TEST_ASSERT(result[3]==0x01);

    // Success: non-0 arg_index
    TEST_ASSERT(TCMD_extract_hex_array_arg("FF,012200FF,FFF", 1, result, sizeof(result), &res_len) == 0);
    TEST_ASSERT(res_len==4);
    TEST_ASSERT(result[0]==0x01);
    TEST_ASSERT(result[1]==0x22);
    TEST_ASSERT(result[2]==0x00);
    TEST_ASSERT(result[3]==0xFF);

    // Error: Argument has uneven number of hex chars.
    TEST_ASSERT(TCMD_extract_hex_array_arg("FF,012200FF,FFF", 2, result, sizeof(result), &res_len) > 0);
    TEST_ASSERT(TCMD_extract_hex_array_arg("FFF", 0, result, sizeof(result), &res_len) > 0);
    TEST_ASSERT(TCMD_extract_hex_array_arg("FF_F", 0, result, sizeof(result), &res_len) > 0);
    
    // Error: Invalid character found.
    TEST_ASSERT(TCMD_extract_hex_array_arg("0xFF,0xFF", 0, result, sizeof(result), &res_len) > 0);
    TEST_ASSERT(TCMD_extract_hex_array_arg("0xFF,0xFF", 1, result, sizeof(result), &res_len) > 0);

    // Error: Empty string and empty arg.
    TEST_ASSERT(TCMD_extract_hex_array_arg("", 0, result, sizeof(result), &res_len) > 0);
    TEST_ASSERT(TCMD_extract_hex_array_arg(",", 0, result, sizeof(result), &res_len) > 0);
    TEST_ASSERT(TCMD_extract_hex_array_arg(",", 1, result, sizeof(result), &res_len) > 0);

    // Success: Output array is longer than the actual result.
    uint8_t big_result[6];
    TEST_ASSERT(TCMD_extract_hex_array_arg("0Fe2,FFA401", 0, big_result, sizeof(big_result), &res_len) == 0);
    TEST_ASSERT(res_len==2);
    TEST_ASSERT(big_result[0]==0x0F);
    TEST_ASSERT(big_result[1]==0xe2);

    TEST_ASSERT(TCMD_extract_hex_array_arg("0Fe2,FFA401", 1, big_result, sizeof(big_result), &res_len) == 0);
    TEST_ASSERT(res_len==3);
    TEST_ASSERT(big_result[0]==0xff);
    TEST_ASSERT(big_result[1]==0xa4);
    TEST_ASSERT(big_result[2]==0x01);

    // Error: Not enough arguments
    TEST_ASSERT(TCMD_extract_hex_array_arg("0Fe2,FFA401", 2, big_result, sizeof(big_result), &res_len) > 0);

    return 0;
}
