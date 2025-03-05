#include "unit_tests/unit_test_helpers.h"
#include "telecommands/telecommand_parser.h"
#include "telecommands/telecommand_args_helpers.h"
#include "transforms/number_comparisons.h"
#include "string.h"
#define HELPERS_TEST_EPSILON 1e-6

uint8_t TEST_EXEC__TCMD_is_char_alphanumeric() {
    TEST_ASSERT(TCMD_is_char_alphanumeric('a') == 1);
    TEST_ASSERT(TCMD_is_char_alphanumeric('M') == 1);
    TEST_ASSERT(TCMD_is_char_alphanumeric('A') == 1);
    TEST_ASSERT(TCMD_is_char_alphanumeric('z') == 1);
    TEST_ASSERT(TCMD_is_char_alphanumeric('m') == 1);
    TEST_ASSERT(TCMD_is_char_alphanumeric('Z') == 1);
    TEST_ASSERT(TCMD_is_char_alphanumeric('0') == 1);
    TEST_ASSERT(TCMD_is_char_alphanumeric('9') == 1);
    TEST_ASSERT(TCMD_is_char_alphanumeric(' ') == 0);
    TEST_ASSERT(TCMD_is_char_alphanumeric('!') == 0);
    TEST_ASSERT(TCMD_is_char_alphanumeric('@') == 0);
    TEST_ASSERT(TCMD_is_char_alphanumeric('[') == 0);
    TEST_ASSERT(TCMD_is_char_alphanumeric('+') == 0); // important, on left side of command name
    TEST_ASSERT(TCMD_is_char_alphanumeric('(') == 0); // important, on right side of command name
    return 0;
}

uint8_t TEST_EXEC__TCMD_check_starts_with_device_id() {
    uint8_t result = 0;
    result = TCMD_check_starts_with_device_id("CTS1+hello_world", 16);
    TEST_ASSERT(result == 1);
    
    // this case should fail, as the command needs to be longer than just the prefix
    result = TCMD_check_starts_with_device_id("CTS1+", 5);
    TEST_ASSERT(result == 0);

    // test that CTS-SAT-2 won't break our molten satellite
    result = TCMD_check_starts_with_device_id("CTS2+hello_world", 16);
    TEST_ASSERT(result == 0);

    // test shortest allowable command
    result = TCMD_check_starts_with_device_id("CTS1+a", 6);
    TEST_ASSERT(result == 1);

    return 0;
}

uint8_t TEST_EXEC__TCMD_get_suffix_tag_uint64() {
    uint8_t result_err = 0;
    uint64_t result_val = 0;
    result_err = TCMD_get_suffix_tag_uint64("@tsexec=1234", "@tsexec=", &result_val);
    TEST_ASSERT(result_err == 0);
    TEST_ASSERT(result_val == 1234);

    result_err = TCMD_get_suffix_tag_uint64("@tsexec=1234", "@tssent=", &result_val);
    TEST_ASSERT(result_err > 0);

    result_err = TCMD_get_suffix_tag_uint64("@tsexec=1720939654482", "@tsexec=", &result_val);
    TEST_ASSERT(result_err == 0);
    TEST_ASSERT(result_val == 1720939654482);

    result_err = TCMD_get_suffix_tag_uint64("@tsexec=1720939654482@tssent=3441879389695", "@tssent=", &result_val);
    TEST_ASSERT(result_err == 0);
    TEST_ASSERT(result_val == 3441879389695);

    result_err = TCMD_get_suffix_tag_uint64("@tsexec=1720939654482@tssent=3441879389695", "@tsexec=", &result_val);
    TEST_ASSERT(result_err == 0);
    TEST_ASSERT(result_val == 1720939654482);

    result_err = TCMD_get_suffix_tag_uint64("@tsexec=1720939654482A@tssent=3441879389695", "@tsexec=", &result_val);
    TEST_ASSERT(result_err > 0); // should fail because of the A

    // Case: minimum length
    result_err = TCMD_get_suffix_tag_uint64("@tsexec=0@tssent=3441879389695", "@tsexec=", &result_val);
    TEST_ASSERT(result_err == 0);
    TEST_ASSERT(result_val == 0);

    // Fail case: 0-length value
    result_err = TCMD_get_suffix_tag_uint64("@tsexec=@tssent=3441879389695", "@tsexec=", &result_val);
    TEST_ASSERT(result_err > 0);

    return 0;
}
uint8_t TEST_EXEC__TCMD_ascii_to_double() {
    double output_val;

    // Nominal test.
    TEST_ASSERT_FALSE(TCMD_ascii_to_double("6.7", 3, &output_val));
    TEST_ASSERT_TRUE(GEN_compare_doubles(output_val, 6.7, HELPERS_TEST_EPSILON) == 1);
    // assert: output_val approx equals 6.7

    // Nominal test (negative).
    TEST_ASSERT_FALSE(TCMD_ascii_to_double("-23.9", 5, &output_val));
    TEST_ASSERT_TRUE(GEN_compare_doubles(output_val, -23.9, HELPERS_TEST_EPSILON) == 1);
    // assert: output_val approx equals -23.9

    // Error: empty string
    TEST_ASSERT_TRUE(TCMD_ascii_to_double("", 0, &output_val));

    // Error: just a space
    TEST_ASSERT_TRUE(TCMD_ascii_to_double(" ", 1, &output_val));

    // Error: letters
    TEST_ASSERT_TRUE(TCMD_ascii_to_double("abc", 3, &output_val));

    // Error: trailing letters
    TEST_ASSERT_TRUE(TCMD_ascii_to_double("23.7a", 5, &output_val));

    // Error: numbers in the middle
    TEST_ASSERT_TRUE(TCMD_ascii_to_double("123a123", 7, &output_val));

    // More assorted tests
    TEST_ASSERT_TRUE(TCMD_ascii_to_double("a23.7a", 6, &output_val) != 0);
    TEST_ASSERT_TRUE(TCMD_ascii_to_double("23a7", 4, &output_val) != 0);
    TEST_ASSERT_TRUE(TCMD_ascii_to_double("23ar.7", 6, &output_val) != 0);
    TEST_ASSERT_TRUE(TCMD_ascii_to_double(" 23.7", 5, &output_val) != 0); // has a space
    TEST_ASSERT_TRUE(TCMD_ascii_to_double("23.7 ", 5, &output_val) != 0); // has a space
    TEST_ASSERT_TRUE(TCMD_ascii_to_double("2 3.7", 5, &output_val) != 0); // has a space
    TEST_ASSERT_TRUE(TCMD_ascii_to_double("23.7.1", 6, &output_val) != 0); // two decimal points
    TEST_ASSERT_TRUE(TCMD_ascii_to_double(".7", 2, &output_val) != 0); // can't have decimal point at start
    TEST_ASSERT_TRUE(TCMD_ascii_to_double("7.", 2, &output_val) != 0); // can't have decimal point at end
    TEST_ASSERT_TRUE(TCMD_ascii_to_double("7-7", 2, &output_val) != 0); // can't have minus sign in the middle
    TEST_ASSERT_TRUE(TCMD_ascii_to_double("1.5-", 2, &output_val) != 0); // can't have minus sign at the end

    return 0;
}

uint8_t TEST_EXEC_TCMD_parse_full_telecommand() {
    TCMD_parsed_tcmd_to_execute_t parsed_tcmd;
    // Testing correct usage of 0 argument telecommand 
    TEST_ASSERT_TRUE(TCMD_parse_full_telecommand("CTS1+hello_world()!", TCMD_TelecommandChannel_DEBUG_UART, &parsed_tcmd) == 0);
    TEST_ASSERT_TRUE(strcmp(parsed_tcmd.args_str_no_parens, "") == 0);
    TEST_ASSERT_TRUE(parsed_tcmd.tcmd_channel == TCMD_TelecommandChannel_DEBUG_UART);
    TEST_ASSERT_TRUE(strcmp(TCMD_telecommand_definitions[parsed_tcmd.tcmd_idx].tcmd_name, "hello_world") == 0);

    // Tesing incorrect usage of 0 argument telecommand
    TEST_ASSERT_TRUE(TCMD_parse_full_telecommand("CTS1+hello_world(a)!", TCMD_TelecommandChannel_DEBUG_UART, &parsed_tcmd) == 130);
    TEST_ASSERT_TRUE(TCMD_parse_full_telecommand("CTS1+hello_world(a,1)!", TCMD_TelecommandChannel_DEBUG_UART, &parsed_tcmd) == 130);
    TEST_ASSERT_TRUE(TCMD_parse_full_telecommand("CTS1+hello_world(,)!", TCMD_TelecommandChannel_DEBUG_UART, &parsed_tcmd) == 130);

    // Testing correct usage of 1 argument telecommand 
    TEST_ASSERT_TRUE(TCMD_parse_full_telecommand("CTS1+set_system_time(10)!", TCMD_TelecommandChannel_DEBUG_UART, &parsed_tcmd) == 0); 
    TEST_ASSERT_TRUE(strcmp(parsed_tcmd.args_str_no_parens, "10") == 0);
    TEST_ASSERT_TRUE(parsed_tcmd.tcmd_channel == TCMD_TelecommandChannel_DEBUG_UART);
    TEST_ASSERT_TRUE(strcmp(TCMD_telecommand_definitions[parsed_tcmd.tcmd_idx].tcmd_name, "set_system_time") == 0);


    // Tesing incorrect usage of 1 argument telecommand
    TEST_ASSERT_TRUE(TCMD_parse_full_telecommand("CTS1+set_system_time()!", TCMD_TelecommandChannel_DEBUG_UART, &parsed_tcmd) == 130);
    TEST_ASSERT_TRUE(TCMD_parse_full_telecommand("CTS1+set_system_time(10,1)!", TCMD_TelecommandChannel_DEBUG_UART, &parsed_tcmd) == 130);
    TEST_ASSERT_TRUE(TCMD_parse_full_telecommand("CTS1+set_system_time(10,)!", TCMD_TelecommandChannel_DEBUG_UART, &parsed_tcmd) == 130);
    TEST_ASSERT_TRUE(TCMD_parse_full_telecommand("CTS1+set_system_time(,10)!", TCMD_TelecommandChannel_DEBUG_UART, &parsed_tcmd) == 130);
    TEST_ASSERT_TRUE(TCMD_parse_full_telecommand("CTS1+set_system_time(,)!", TCMD_TelecommandChannel_DEBUG_UART, &parsed_tcmd) == 130);

    // Testing correct usage of 3 3 argument telecommand
    TEST_ASSERT_TRUE(TCMD_parse_full_telecommand("CTS1+flash_read_hex(0,0,0)!", TCMD_TelecommandChannel_DEBUG_UART, &parsed_tcmd) == 0);
    TEST_ASSERT_TRUE(TCMD_parse_full_telecommand("CTS1+flash_read_hex(0,0,0,)!", TCMD_TelecommandChannel_DEBUG_UART, &parsed_tcmd) == 130);
    TEST_ASSERT_TRUE(TCMD_parse_full_telecommand("CTS1+flash_read_hex(,0,0,0)!", TCMD_TelecommandChannel_DEBUG_UART, &parsed_tcmd) == 130);
    TEST_ASSERT_TRUE(TCMD_parse_full_telecommand("CTS1+flash_read_hex(,0,0,0)!", TCMD_TelecommandChannel_DEBUG_UART, &parsed_tcmd) == 130);
    TEST_ASSERT_TRUE(TCMD_parse_full_telecommand("CTS1+flash_read_hex(0,0)!", TCMD_TelecommandChannel_DEBUG_UART, &parsed_tcmd) == 130);

    return 0;
}