#include "telecommands/telecommand_args_helpers.h"
#include "unit_tests/unit_test_helpers.h"


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
    TEST_ASSERT_FALSE(TCMD_ascii_to_int64("6", 3, &output_val));
    TEST_ASSERT_TRUE(output_val == 6);
    // assert: output_val approx equals 6

    // Nominal test (negative).
    TEST_ASSERT_FALSE(TCMD_ascii_to_int64("-23", 5, &output_val));
    TEST_ASSERT_TRUE(output_val == -23);
    // assert: output_val approx equals -23

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

    // More assorted tests
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
    TEST_ASSERT_TRUE(TCMD_ascii_to_int64("7-7", 2, &output_val) != 0); // can't have minus sign in the middle
    TEST_ASSERT_TRUE(TCMD_ascii_to_int64("1.5-", 2, &output_val) != 0); // can't have minus sign at the end

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