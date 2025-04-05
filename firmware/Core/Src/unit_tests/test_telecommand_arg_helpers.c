#include "telecommand_exec/telecommand_args_helpers.h"
#include "unit_tests/unit_test_helpers.h"

uint8_t TEST_EXEC__TCMD_ascii_to_uint64() {
    uint64_t result;

    // Success: Nominal test
    TEST_ASSERT(TCMD_ascii_to_uint64("123", 3, &result) == 0);
    TEST_ASSERT(result == 123);

    // Error: Empty string
    TEST_ASSERT(TCMD_ascii_to_uint64("", 0, &result) == 1);

    // Error: String is too long
    TEST_ASSERT(TCMD_ascii_to_uint64("12345678912345678912", 20, &result) == 2);

    // Success: String has a digit length of 19
    TEST_ASSERT(TCMD_ascii_to_uint64("1234567891234567891", 19, &result) == 0);
    TEST_ASSERT(result == 1234567891234567891);

    // Error: String doesn't start with an integer
    TEST_ASSERT(TCMD_ascii_to_uint64("abc123", 6, &result) == 3);

    // Error: Result digit length doesn't match str_len
    TEST_ASSERT(TCMD_ascii_to_uint64("123abc", 6, &result) == 3);

    return 0;
}

uint8_t TEST_EXEC__TCMD_extract_uint64_arg() {
    uint64_t result;
    char str[] = "1,2,3,4,a,6,7,8,9";
    // Success: Nominal Test
    TEST_ASSERT(TCMD_extract_uint64_arg(str, sizeof(str), 0, &result) == 0);
    TEST_ASSERT(result == 1);

    char str2[] = "0";
    // Error: Empty string
    TEST_ASSERT(TCMD_extract_uint64_arg(str2, 0, 0, &result) == 1);

    // Error: Not enough arguments
    TEST_ASSERT(TCMD_extract_uint64_arg(str, sizeof(str), 10, &result) == 2);

    // Error: Argument is not an integer
    TEST_ASSERT(TCMD_extract_uint64_arg(str, sizeof(str), 4, &result) == 3);

    char str3[] = "1,2,3,4,12345678912345678912";
    // Error: Argument is longer than can be taken by TCMD_ascii_to_uint64()
    TEST_ASSERT(TCMD_extract_uint64_arg(str3, sizeof(str3), 4, &result) == 4);

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

uint8_t TEST_EXEC__TCMD_ascii_to_int64() {
    int64_t output_val;

    // Nominal test.
    TEST_ASSERT(TCMD_ascii_to_int64("6", 1, &output_val) == 0);
    TEST_ASSERT_TRUE(output_val == 6);

    // Nominal test (negative).
    TEST_ASSERT(TCMD_ascii_to_int64("-23", 3, &output_val) == 0);
    TEST_ASSERT_TRUE(output_val == -23);
    // assert: output_val approx equals -23

    // Error: str_len is given as longer than the string, and thus the first 3 chars contain non-int characters.
    TEST_ASSERT(TCMD_ascii_to_int64("6", 3, &output_val) != 0);

    // Error: empty string
    TEST_ASSERT_TRUE(TCMD_ascii_to_int64("", 0, &output_val));

    // Error: just a space
    TEST_ASSERT_TRUE(TCMD_ascii_to_int64(" ", 1, &output_val));

    // Error: letters
    TEST_ASSERT_TRUE(TCMD_ascii_to_int64("abc", 3, &output_val));

    // Error: trailing letters
    TEST_ASSERT_TRUE(TCMD_ascii_to_int64("23.7a", 5, &output_val));

    // Error: numbers in the middle
    TEST_ASSERT_TRUE(TCMD_ascii_to_int64("123a123", 7, &output_val));

    // More assorted error tests.
    TEST_ASSERT_TRUE(TCMD_ascii_to_int64("a23.7a", 6, &output_val) != 0);
    TEST_ASSERT_TRUE(TCMD_ascii_to_int64("1.2", 3, &output_val) != 0);
    TEST_ASSERT_TRUE(TCMD_ascii_to_int64("23a7", 4, &output_val) != 0);
    TEST_ASSERT_TRUE(TCMD_ascii_to_int64("23ar.7", 6, &output_val) != 0);
    TEST_ASSERT_TRUE(TCMD_ascii_to_int64(" 23.7", 5, &output_val) != 0); // has a space
    TEST_ASSERT_TRUE(TCMD_ascii_to_int64("23.7 ", 5, &output_val) != 0); // has a space
    TEST_ASSERT_TRUE(TCMD_ascii_to_int64("2 3.7", 5, &output_val) != 0); // has a space
    TEST_ASSERT_TRUE(TCMD_ascii_to_int64("23.7.1", 6, &output_val) != 0); // two decimal points
    TEST_ASSERT_TRUE(TCMD_ascii_to_int64(".7", 2, &output_val) != 0); // can't have decimal point 
    TEST_ASSERT_TRUE(TCMD_ascii_to_int64("7.", 2, &output_val) != 0); // can't have decimal point
    TEST_ASSERT_TRUE(TCMD_ascii_to_int64("7-7", 3, &output_val) != 0); // can't have minus sign in the middle
    TEST_ASSERT_TRUE(TCMD_ascii_to_int64("1.5-", 3, &output_val) != 0); // can't have minus sign at the end

    return 0;
}

uint8_t TEST_EXEC__TCMD_extract_int64_arg() {
    int64_t output_val;

    // Nominal test
    TEST_ASSERT_FALSE(TCMD_extract_int64_arg("2,-4,245", 8, 0, &output_val));
    TEST_ASSERT_TRUE(output_val == 2);
    TEST_ASSERT_FALSE(TCMD_extract_int64_arg("2,-4,245", 8, 1, &output_val));
    TEST_ASSERT_TRUE(output_val == -4);
    TEST_ASSERT_FALSE(TCMD_extract_int64_arg("2,-4,245", 8, 2, &output_val));
    TEST_ASSERT_TRUE(output_val == 245);

    // Go past the end
    TEST_ASSERT_TRUE(TCMD_extract_int64_arg("2,-4,245", 8, 3, &output_val));

    // Non-int characters
    TEST_ASSERT_FALSE(TCMD_extract_int64_arg("2,-4,24a5", 9, 0, &output_val));
    TEST_ASSERT_TRUE(output_val == 2);
    TEST_ASSERT_FALSE(TCMD_extract_int64_arg("2,-4,24a5", 9, 1, &output_val));
    TEST_ASSERT_TRUE(output_val == -4);
    TEST_ASSERT_TRUE(TCMD_extract_int64_arg("2,-4,24a5", 9, 2, &output_val)); // should error here

    // More errors
    TEST_ASSERT_TRUE(TCMD_extract_int64_arg("2,-4,24.5", 9, 2, &output_val));
    TEST_ASSERT_TRUE(TCMD_extract_int64_arg("2,-4,--245", 9, 2, &output_val));
    TEST_ASSERT_TRUE(TCMD_extract_int64_arg("2,-4,24/5", 9, 2, &output_val));
    TEST_ASSERT_TRUE(TCMD_extract_int64_arg("2,-4a,245", 9, 1, &output_val));

    // Wrong length
    TEST_ASSERT_TRUE(TCMD_extract_int64_arg("2,-4,245", 4, 2, &output_val));

    // Whitespace
    TEST_ASSERT_TRUE(TCMD_extract_int64_arg(" , , ", 5, 2, &output_val));
    TEST_ASSERT_TRUE(TCMD_extract_int64_arg("", 0, 0, &output_val));

    return 0;
}