#include "unit_tests/unit_test_helpers.h"
#include "telecommands/telecommand_args_helpers.h"

uint8_t TEST_EXEC__TCMD_hex_string_to_byte_array(){
    char hex_strings[3][8] ={
        "0xFFFFFF",
        "0x000000",
        "0x123BDE"
    };
    unsigned char expected_answer[3][3] = {
        {0xFF,0xFF,0xFF},
        {0x00,0x00,0x00},
        {0x12,0x3B,0xDE}
    };
    unsigned char result[3];
    TCMD_hex_string_to_byte_array(hex_strings[0],strlen(hex_strings[0]),result);

    TEST_ASSERT(TCMD_hex_string_to_byte_array(hex_strings[0],strlen(hex_strings[0]),result) ==0);
    TEST_ASSERT_EQUAL(expected_answer[0], result);

    TCMD_hex_string_to_byte_array(hex_strings[1],strlen(hex_strings[1]),result);

    TEST_ASSERT(TCMD_hex_string_to_byte_array(hex_strings[1],strlen(hex_strings[1]),result) ==0);
    TEST_ASSERT_EQUAL(expected_answer[1], result);

    TCMD_hex_string_to_byte_array(hex_strings[2],strlen(hex_strings[2]),result);

    TEST_ASSERT(TCMD_hex_string_to_byte_array(hex_strings[2],strlen(hex_strings[2]),result) ==0);
    TEST_ASSERT_EQUAL(expected_answer[2], result);
    return 0;
}