#include "unit_tests/unit_test_helpers.h"
#include "telecommand_exec/telecommand_parser.h"
#include "telecommand_exec/telecommand_args_helpers.h"
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

uint8_t TEST_EXEC__TCMD_get_suffix_tag_str() {
    uint8_t result_err = 0;
    char result_val[TCMD_MAX_LOG_FILENAME_LEN] = {0};
    uint16_t result_val_len = sizeof(result_val);

    // -------------------------------
    // Test 1: Valid tag and value
    result_err = TCMD_get_suffix_tag_str("@log_filename=log.txt", "@log_filename=", result_val, result_val_len);
    TEST_ASSERT(result_err == 0);
    TEST_ASSERT(strcmp(result_val, "log.txt") == 0);

    memset(result_val, 0, result_val_len);

    // -------------------------------
    // Test 2: Missing tag
    result_err = TCMD_get_suffix_tag_str("@log_filename=log.txt", "@missing_tag=", result_val, result_val_len);
    TEST_ASSERT(result_err == 1);
    TEST_ASSERT(strcmp(result_val, "") == 0); // should stay empty

    memset(result_val, 0, result_val_len);

    // -------------------------------
    // Test 3: Empty value after tag
    result_err = TCMD_get_suffix_tag_str("@log_filename=", "@log_filename=", result_val, result_val_len);
    TEST_ASSERT(result_err == 2); // because no value after =
    TEST_ASSERT(strcmp(result_val, "") == 0);

    memset(result_val, 0, result_val_len);

    // -------------------------------
    // Test 4: Buffer too small
    result_err = TCMD_get_suffix_tag_str("@log_filename=log.txt", "@log_filename=", result_val, 5);
    TEST_ASSERT(result_err == 4); // value_len (7) > 5-1
    TEST_ASSERT(strcmp(result_val, "") == 0);

    memset(result_val, 0, result_val_len);

    // -------------------------------
    // Test 5: Invalid terminator after value
    result_err = TCMD_get_suffix_tag_str("@log_filename=log.txt$", "@log_filename=", result_val, result_val_len);
    TEST_ASSERT(result_err == 5); // '$' is invalid terminator
    TEST_ASSERT(strcmp(result_val, "") == 0);

    memset(result_val, 0, result_val_len);

    // -------------------------------
    // Test 6: Special characters '.' and '-' allowed
    result_err = TCMD_get_suffix_tag_str("@log_filename=log-file.v1", "@log_filename=", result_val, result_val_len);
    TEST_ASSERT(result_err == 0);
    TEST_ASSERT(strcmp(result_val, "log-file.v1") == 0);

    memset(result_val, 0, result_val_len);

    // -------------------------------
    // Test 7: NULL input checks
    result_err = TCMD_get_suffix_tag_str(NULL, "@log_filename=", result_val, result_val_len);
    TEST_ASSERT(result_err == 10);

    result_err = TCMD_get_suffix_tag_str("@log_filename=log.txt", NULL, result_val, result_val_len);
    TEST_ASSERT(result_err == 10);

    result_err = TCMD_get_suffix_tag_str("@log_filename=log.txt", "@log_filename=", NULL, result_val_len);
    TEST_ASSERT(result_err == 10);

    result_err = TCMD_get_suffix_tag_str("@log_filename=log.txt", "@log_filename=", result_val, 0);
    TEST_ASSERT(result_err == 10);

    memset(result_val, 0, result_val_len);

    // -------------------------------
    // Test 8: Value exactly fits buffer (edge case)
    // value_len == value_dest_max_len - 1
    char buffer[10] = {0}; // "log.txt" is 7 chars
    result_err = TCMD_get_suffix_tag_str("@log_filename=log.txt", "@log_filename=", buffer, 8); // 7+1 null
    TEST_ASSERT(result_err == 0);
    TEST_ASSERT(strcmp(buffer, "log.txt") == 0);

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

uint8_t TEST_EXEC__TCMD_process_suffix_tag_tssent() {
    uint64_t tssent_time_ms = 0;

    // Test case: Valid @tssent tag
    TEST_ASSERT(TCMD_process_suffix_tag_tssent("@tssent=1234567890", strlen("@tssent=1234567890"), &tssent_time_ms) == 0);
    TEST_ASSERT(tssent_time_ms == 1234567890);

    // Test case: Missing @tssent tag
    TEST_ASSERT(TCMD_process_suffix_tag_tssent("@tsexec=1234567890", strlen("@tsexec=1234567890"), &tssent_time_ms) == 0);

    // Test case: Invalid @tssent tag format
    TEST_ASSERT(TCMD_process_suffix_tag_tssent("@tssent=abcd", strlen("@tssent=abcd"), &tssent_time_ms) == 1);

    // Test case: Empty string
    TEST_ASSERT(TCMD_process_suffix_tag_tssent("", 0, &tssent_time_ms) == 0);

    return 0;
}

uint8_t TEST_EXEC__TCMD_process_suffix_tag_tsexec() {
    uint64_t tsexec_time_ms = 0;

    // Test case: Valid @tsexec tag
    TEST_ASSERT(TCMD_process_suffix_tag_tsexec("@tsexec=9876543210", strlen("@tsexec=9876543210"), &tsexec_time_ms) == 0);
    TEST_ASSERT(tsexec_time_ms == 9876543210);

    // Test case: Missing @tsexec tag
    TEST_ASSERT(TCMD_process_suffix_tag_tsexec("@tssent=9876543210", strlen("@tssent=9876543210"), &tsexec_time_ms) == 0);

    // Test case: Invalid @tsexec tag format
    TEST_ASSERT(TCMD_process_suffix_tag_tsexec("@tsexec=abcd", strlen("@tsexec=abcd"), &tsexec_time_ms) == 1);

    // Test case: Empty string
    TEST_ASSERT(TCMD_process_suffix_tag_tsexec("", 0, &tsexec_time_ms) == 0);

    return 0;
}

uint8_t TEST_EXEC__TCMD_process_suffix_tag_sha256() {
    const char *tcmd_str = "CTS1+hello_world()!";
    const int32_t end_of_args_idx = strlen("CTS1+hello_world()") - 1;

    // Test case: Valid @sha256 tag
    // Used the following website to compute: https://emn178.github.io/online-tools/sha256.html
    const char *valid_suffix = "@sha256=9f2c356aee31c00991e024189ec6c602aaee9358dc5cc2d26182e55f98dce181";
    TEST_ASSERT(TCMD_process_suffix_tag_sha256(valid_suffix, strlen(valid_suffix), tcmd_str, end_of_args_idx) == 0);

    // Test case: Missing @sha256 tag (not required)
    const char *missing_suffix = "@tssent=1234567890";
    TEST_ASSERT(TCMD_process_suffix_tag_sha256(missing_suffix, strlen(missing_suffix), tcmd_str, end_of_args_idx) == 0);

    // Test case: Invalid @sha256 tag format
    const char *invalid_suffix = "@sha256=invalidhash";
    TEST_ASSERT(TCMD_process_suffix_tag_sha256(invalid_suffix, strlen(invalid_suffix), tcmd_str, end_of_args_idx) == 2);

    // Test case: Mismatched SHA256 hash
    const char *mismatched_suffix = "@sha256=ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff";
    TEST_ASSERT(TCMD_process_suffix_tag_sha256(mismatched_suffix, strlen(mismatched_suffix), tcmd_str, end_of_args_idx) == 3);

    return 0;
}

uint8_t TEST_EXEC__TCMD_process_suffix_tag_log_filename()
{
    char log_filename[TCMD_MAX_LOG_FILENAME_LEN] = {0};
    uint8_t log_filename_len = sizeof(log_filename);

    // -------------------------------
    // Case: Correct log filename
    const char *valid_suffix = "@log_filename=log.txt";
    TEST_ASSERT(TCMD_process_suffix_tag_log_filename(valid_suffix, strlen(valid_suffix), log_filename, log_filename_len) == 0);
    TEST_ASSERT(strcmp(log_filename, "log.txt") == 0);

    memset(log_filename, 0, log_filename_len); 

    // -------------------------------
    // Case: Log filename with _ and numbers
    const char *valid_suffix_2 = "@log_filename=log_123.file";
    TEST_ASSERT(TCMD_process_suffix_tag_log_filename(valid_suffix_2, strlen(valid_suffix_2), log_filename, log_filename_len) == 0);
    TEST_ASSERT(strcmp(log_filename, "log_123.file") == 0);

    memset(log_filename, 0, log_filename_len); 

    // -------------------------------
    // Case: Missing @log_filename tag
    const char *no_suffix = "@tssent=1234567890";
    TEST_ASSERT(TCMD_process_suffix_tag_log_filename(no_suffix, strlen(no_suffix), log_filename, log_filename_len) == 0);
    TEST_ASSERT(log_filename[0] == '\0'); // empty string

    memset(log_filename, 0, log_filename_len); 

    // -------------------------------
    // Case: Empty log filename
    const char *empty_suffix = "@log_filename=";
    TEST_ASSERT(TCMD_process_suffix_tag_log_filename(empty_suffix, strlen(empty_suffix), log_filename, log_filename_len) == 1);
    TEST_ASSERT(log_filename[0] == '\0'); // empty string

    memset(log_filename, 0, log_filename_len); 

    // -------------------------------
    // Case: Tag not at position 0
    const char *offset_tag = "@other=1@log_filename=test.log";
    TEST_ASSERT(TCMD_process_suffix_tag_log_filename(offset_tag, strlen(offset_tag), log_filename, log_filename_len) == 0);
    TEST_ASSERT(strcmp(log_filename, "test.log") == 0);

    return 0;
}
