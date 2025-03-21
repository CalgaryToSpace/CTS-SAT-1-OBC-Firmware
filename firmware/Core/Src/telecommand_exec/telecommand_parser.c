#include "telecommand_exec/telecommand_definitions.h"
#include "telecommand_exec/telecommand_parser.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "debug_tools/debug_uart.h"
#include "transforms/arrays.h"
#include "crypto/sha256.h"
#include "log/log.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/// @brief Bool. When 1, the telecommand parser will require a valid SHA256 hash in the telecommand string.
uint32_t TCMD_require_valid_sha256 = 0;

/// @brief Returns whether a character is alphanumeric (0-9, A-Z, a-z).
/// @param c The character to check.
/// @return 1 if the character is alphanumeric, 0 otherwise.
uint8_t TCMD_is_char_alphanumeric(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

/// @brief Returns whether a character is numeric (0-9).
/// @param c The character to check.
/// @return 1 if the character is a numerical digit (0-9), 0 otherwise.
uint8_t TCMD_is_char_numeric(char c) {
    return (c >= '0' && c <= '9');
}

/// @brief Returns whether a character is a valid character in a telecommand name.
/// @param c The character to check.
/// @return 1 if the character is valid, 0 otherwise.
uint8_t TCMD_is_char_valid_telecommand_name_char(char c) {
    return TCMD_is_char_alphanumeric(c) || c == '_';
}


/// @brief Returns whether tcmd_str starts with `CTS1+` (TCMD_PREFIX_STR).
/// @param tcmd_str The telecommand string to check.
/// @param tcmd_str_len The length of the telecommand string.
/// @return 1 if tcmd_str starts with `CTS1+`, 0 otherwise.
uint8_t TCMD_check_starts_with_device_id(const char *tcmd_str, uint32_t tcmd_str_len) {
    if (tcmd_str_len <= TCMD_PREFIX_STR_LEN) {
        return 0;
    }
    for (int32_t i = 0; i < TCMD_PREFIX_STR_LEN; i++) {
        if (tcmd_str[i] != TCMD_PREFIX_STR[i]) {
            return 0;
        }
    }
    return 1;
}

/// @brief Finds an index into TCMD_telecommand_definitions for the given telecommand string.
/// @details This function searches for the longest matching telecommand string in TCMD_telecommand_definitions, where all
///          alpha-numeric characters at the start of the to-be-parsed telecommand match the telecommand string in
///          TCMD_telecommand_definitions. This function is case-sensitive.
/// @param tcmd_str The telecommand string to search within, starting with prefix, optionally including args.
/// @return The index into TCMD_telecommand_definitions for the given telecommand string. <0 if error.
///       Returns -1 if telecommand not found.
///       Returns -2 if strlen of tcmd_str is less than TCMD_PREFIX_STR_LEN.
///       Returns -3 if tcmd_str does not start with TCMD_PREFIX_STR.
int32_t TCMD_parse_telecommand_get_index(const char *tcmd_str, uint32_t tcmd_str_len) {
    if (tcmd_str_len <= TCMD_PREFIX_STR_LEN) {
        return (-2);
    }
    if (!TCMD_check_starts_with_device_id(tcmd_str, tcmd_str_len)) {
        return (-3);
    }

    // Find the length of the telecommand name part of the "tcmd_str"
    // tcmd_name_len is the length of the telecommand name part of the "tcmd_str"
    //   and its default value is the remainder of the string after the prefix
    uint16_t tcmd_name_len = tcmd_str_len - TCMD_PREFIX_STR_LEN;
    for (uint16_t i = TCMD_PREFIX_STR_LEN; i < tcmd_str_len; i++) {
        if (!TCMD_is_char_valid_telecommand_name_char(tcmd_str[i])) {
            tcmd_name_len = i - TCMD_PREFIX_STR_LEN;
            break;
        }
    }

    // Iterate through the table of telecommands, and see if any match the given telecommand string
    for (uint16_t check_cmd_idx = 0; check_cmd_idx < TCMD_NUM_TELECOMMANDS; check_cmd_idx++) {
        if (tcmd_name_len != strlen(TCMD_telecommand_definitions[check_cmd_idx].tcmd_name)) {
            continue;
        }

        for (uint16_t str_idx = 0; str_idx < tcmd_name_len; str_idx++) {
            char rxd_char = tcmd_str[str_idx + TCMD_PREFIX_STR_LEN];
            char real_tcmd_name_char = TCMD_telecommand_definitions[check_cmd_idx].tcmd_name[str_idx];
            if (rxd_char != real_tcmd_name_char) {
                break;
            }

            // if we've checked against the whole telecommand name
            if (str_idx == (strlen(TCMD_telecommand_definitions[check_cmd_idx].tcmd_name) - 1)) {
                return check_cmd_idx;
            }
        }
    }
    return (-1);
}

/// @brief Searches for a `str` like `\@tag_name=xxxx`, and sets `uint64_t xxxx` into `out_value`.
/// @param str The string (haystack) to search with for the tag.
/// @param tag_name The tag to search for, including the '@' and '='.
/// @param value_dest The destination for the value. `*value_dest` will be set.
/// @return 0 if the tag was found successfully. >0 if the tag was not found, or there was an error.
/// @note This function will return an error if the character after the number is not in: `#\@\0!`
uint8_t TCMD_get_suffix_tag_uint64(const char *str, const char *tag_name, uint64_t *value_dest) {
    // Find the tag in the string
    int16_t tag_index = GEN_get_index_of_substring_in_array(str, strlen(str), tag_name);
    if (tag_index < 0) {
        return 1;
    }

    // Find the start of the value, then do safety check.
    uint16_t value_start_index = tag_index + strlen(tag_name);
    if (value_start_index >= strlen(str)) {
        return 2;
    }

    // Find the end of the value
    uint16_t value_end_index = value_start_index;
    while (TCMD_is_char_numeric(str[value_end_index])) {
        value_end_index++;
    }

    // Check that there was at least one digit
    if (value_end_index <= value_start_index) {
        return 3;
    }

    // Check that the character after the number is any of '#', '@', '!', or '\0'
    if (str[value_end_index] != '#' && str[value_end_index] != '@' && str[value_end_index] != '\0' && str[value_end_index] != '!') {
        return 4;
    }

    // Copy the value into a buffer
    char value_str[32]; // uint64 needs 20 chars max
    memset(value_str, 0, sizeof(value_str));
    if ((uint32_t)(value_end_index - value_start_index) >= sizeof(value_str) - 1) {
        // Failure: digit string too long
        return 5;
    }
    strncpy(value_str, str + value_start_index, value_end_index - value_start_index);

    // Convert the value to a uint64_t
    uint64_t value;
    if (TCMD_ascii_to_uint64(value_str, strlen(value_str), &value) != 0) {
        return 6;
    }

    // Set the value
    *value_dest = value;
    
    // Success
    return 0;
}

/// @brief Searches for a `str` like `\@tag_name=xxxx`, and sets uint8_t array `xxxx` into `value_dest`.
/// @param str The string to search within for the tag. Excluding the tag name, the length of the string must be 256 characters.
/// @param tag_name The tag to search for, including the '@' and '='.
/// @param value_dest The destination for the value. `*value_dest` will be set.
/// @return 0 if the tag was found successfully. >0 if the tag was not found, or there was an error.
/// @note This function will return an error if the character after the string is not one of the following: {'#', '@', '\0', '!'}.
uint8_t TCMD_get_suffix_tag_hex_array(const char *str, const char *tag_name, uint8_t *value_dest) {
    // Find the tag in the string
    const int16_t tag_index = GEN_get_index_of_substring_in_array(str, strlen(str), tag_name);
    if (tag_index < 0) {
        return 1;
    }

    // Find the start of the value, then do safety check.
    const uint16_t value_start_index = tag_index + strlen(tag_name);
    if (value_start_index >= strlen(str)) {
        return 2;
    }

    // Find the end of the value
    uint16_t value_end_index = value_start_index;
    while (TCMD_is_char_valid_telecommand_name_char(str[value_end_index])) {
        value_end_index++;
    }

    // Check that there was at least one digit
    if (value_end_index <= value_start_index) {
        return 3;
    }

    // Check that the character after the number is any of '#', '@', '!', or '\0'
    if (str[value_end_index] != '#' && str[value_end_index] != '@' && str[value_end_index] != '\0' && str[value_end_index] != '!') {
        return 4;
    }

    // Copy the value into a buffer
    char value_str[65]; // sha256 is 32 bytes long. String must be 64 chars long + null terminator
    memset(value_str, 0, sizeof(value_str));
    if ((uint32_t)(value_end_index - value_start_index) > sizeof(value_str)) {
        // Failure: string too long
        return 5;
    }
    strncpy(value_str, str + value_start_index, value_end_index - value_start_index);
    strncpy(value_str + (value_end_index - value_start_index), "\0", 1); // Ensure string is null-terminated

    // Convert the value to a hex array
    uint16_t value_len = 0;
    if (TCMD_extract_hex_array_arg(value_str, 0, value_dest, 32, &value_len) != 0) {
        return 6;
    }

    // Ensure that result array is 32 bytes long
    if (value_len != 32) {
        return 7;
    }
    
    // Success
    return 0;
}

/// @brief Parses a telecommand into everything needed to execute it.
/// @param tcmd_str The telecommand string to parse. Must be null-terminated.
/// @param parsed_tcmd_output Pointer to the output struct, which is modified by this function.
///     Not modified if an error occurs.
/// @return 0 on success, >0 on error.
uint8_t TCMD_parse_full_telecommand(
    const char tcmd_str[], TCMD_TelecommandChannel_enum_t tcmd_channel,
    TCMD_parsed_tcmd_to_execute_t *parsed_tcmd_output
) {
    size_t tcmd_str_len = strlen(tcmd_str);

    if (parsed_tcmd_output == NULL) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error: TCMD_parse_full_telecommand: parsed_tcmd_output is NULL."
        );
        return 1;
    }

    if (tcmd_str_len == 0) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "TCMD_parse_full_telecommand: called with empty string."
        );
        return 10;
    }

    // Optionally, echo back the command.
    LOG_message(
        LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "Parsed telecommand (len=%u): '%s'", tcmd_str_len, tcmd_str
    );

    // Check that the telecommand starts with the correct prefix.
    if (!TCMD_check_starts_with_device_id(tcmd_str, tcmd_str_len)) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "TCMD_parse_full_telecommand: str does not start with the correct prefix."
        );
        return 20;
    }

    // Check that the telecommand ends with "!" (but can have whitespaces between '!' and the end of the string).
    // Also, check that the _only_ '!' is at the end of the string.
    // Important to ensure we're not accidentally parsing 2 telecommands in the same string.
    uint16_t end_of_tcmd_char_str_count = 0;
    for (int32_t i = tcmd_str_len - 1; i >= TCMD_PREFIX_STR_LEN; i--) {
        if (tcmd_str[i] == ' ' || tcmd_str[i] == '\r' || tcmd_str[i] == '\n') {
            // Skip whitespace.
            continue;
        }
        else if (tcmd_str[i] == '!') {
            if (end_of_tcmd_char_str_count == 0) {
                end_of_tcmd_char_str_count++;
            }
            else {
                // Found a second '!', which is not allowed.
                LOG_message(
                    LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "TCMD_parse_full_telecommand: found >1 '!' in the string."
                );
                return 25;
            }
        }
    }
    if (end_of_tcmd_char_str_count == 0) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "TCMD_parse_full_telecommand: no '!' found at the end of the string."
        );
        return 26;
    }

    // Process the telecommand name.
    int32_t tcmd_idx = TCMD_parse_telecommand_get_index(tcmd_str, tcmd_str_len);
    if (tcmd_idx < 0) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "TCMD_parse_full_telecommand: telecommand not found in the list."
        );
        return 30;
    }

    // Get the telecommand definition.
    TCMD_TelecommandDefinition_t tcmd_def = TCMD_telecommand_definitions[tcmd_idx];
    
    // Args: Check opening parenthesis index.
    uint32_t start_of_args_idx = TCMD_PREFIX_STR_LEN + strlen(tcmd_def.tcmd_name);
    if (tcmd_str_len < start_of_args_idx + 1) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "TCMD_parse_full_telecommand: You must have parenthesis for the args."
        );
        return 40;
    }
    if (tcmd_str[start_of_args_idx] != '(') {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "TCMD_parse_full_telecommand: You must have parenthesis for the args. You need an opening paren."
        );
        return 50;
    }
    
    // Args: Check closing parenthesis index.
    int32_t end_of_args_idx = GEN_get_index_of_substring_in_array(tcmd_str, tcmd_str_len, ")");
    if (end_of_args_idx < 0) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "TCMD_parse_full_telecommand: You must have parenthesis for the args. No closing paren found."
        );
        return 60;
    }

    // Extract the args string.
    // arg_len is the number of characters (excluding null terminator) in `args_str_no_parens`
    const uint16_t arg_len = end_of_args_idx - start_of_args_idx - 1;
    char args_str_no_parens[arg_len + 1];
    memcpy(args_str_no_parens, &tcmd_str[start_of_args_idx + 1], arg_len);
    args_str_no_parens[arg_len] = '\0';

    // Extract suffix tags.
    const char *tcmd_suffix_tag_str = &tcmd_str[end_of_args_idx];
    const uint16_t tcmd_suffix_tag_str_len = strlen(tcmd_suffix_tag_str);

    // Extract @tssent=xxxx from the telecommand string, starting at &tcmd_str[end_of_args_idx]
    uint64_t timestamp_sent = 0; // default value
    if (GEN_get_index_of_substring_in_array(tcmd_suffix_tag_str, tcmd_suffix_tag_str_len, "@tssent=") >= 0) {
        // The "@tssent=" tag was found, so parse it.
        if (TCMD_get_suffix_tag_uint64(tcmd_suffix_tag_str, "@tssent=", &timestamp_sent) != 0) {
            LOG_message(
                LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                "TCMD_parse_full_telecommand: failed to parse present @tssent=xxxx."
            );
            return 70;
        }
    }

    // Extract @tsexec=xxxx from the telecommand string, starting at &tcmd_str[end_of_args_idx]
    uint64_t timestamp_to_execute = 0; // default value: execute immediately
    if (GEN_get_index_of_substring_in_array(tcmd_suffix_tag_str, tcmd_suffix_tag_str_len, "@tsexec=") >= 0) {
        // The "@tsexec=" tag was found, so parse it.
        if (TCMD_get_suffix_tag_uint64(tcmd_suffix_tag_str, "@tsexec=", &timestamp_to_execute) != 0) {
            LOG_message(
                LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                "TCMD_parse_full_telecommand: failed to parse present @tsexec=xxxx."
            );
            return 80;
        }
    }

    // Extract @sha256=xxxx from the telecommand string, starting at &tcmd_str[end_of_args_idx]
    uint8_t tag_is_present = 0;
    if (GEN_get_index_of_substring_in_array(tcmd_suffix_tag_str, tcmd_suffix_tag_str_len, "@sha256=") >= 0) { 
        tag_is_present = 1;
    }

    if (TCMD_require_valid_sha256 && !tag_is_present) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error: TCMD_parse_full_telecommand: @sha256=xxxx tag is required but not present."
        );
        return 90;
    }

    if(tag_is_present) {
        uint8_t parsed_sha256_hash[32];
        // Parse the sha256 hash from the suffix tag.
        if (TCMD_get_suffix_tag_hex_array(tcmd_suffix_tag_str, "@sha256=", parsed_sha256_hash) != 0) {
            LOG_message(
                LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                "Error: TCMD_parse_full_telecommand: failed to parse present @sha256=xxxx."
            );
            return 100;
        }

        // After successfully parsing sha256, check to see if hash given matches the calculated hash generated by `CRYPT_compute_sha256_hash()`.
        uint8_t calculated_sha256_hash[32];
        CRYPT_compute_sha256_hash((uint8_t *)tcmd_str, end_of_args_idx + 1, calculated_sha256_hash);

        if (memcmp(parsed_sha256_hash, calculated_sha256_hash, TCMD_SHA256_LENGTH_BYTES) != 0) {
            LOG_message(
                LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                "Error: TCMD_parse_full_telecommand: sha256 hash does not match the expected hash."
            );
            return 110;
        }
    }

    // Check that the args_str_no_parens is not too long.
    // Note: `arg_len` does not include the null terminator, but `TCMD_ARGS_STR_NO_PARENS_SIZE` does.
    if (arg_len + 1 > TCMD_ARGS_STR_NO_PARENS_SIZE) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "TCMD_parse_full_telecommand: args_str_no_parens is too long.");
        return 120;
    }

    // Ensure the correct number of args are provided.
    const int8_t num_args_expected = TCMD_telecommand_definitions[tcmd_idx].number_of_args;
    const char* tcmd_name = TCMD_telecommand_definitions[tcmd_idx].tcmd_name;

    int16_t num_commas = 0;
    for (int32_t i = 0; i <= arg_len; i++) {
        if (args_str_no_parens[i] == ',') {
            num_commas++;
        }    
    }
    
    const int8_t correct_number_of_args_provided = (num_args_expected == 0 && arg_len == 0) || (num_commas == (num_args_expected-1) && arg_len != 0);
    if (!correct_number_of_args_provided) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "TCMD_parse_full_telecommand: %s() accepts %d argument(s).\n", tcmd_name, num_args_expected
        );
        return 130;
    }
// Reached the end of the telecommand parsing. Thus, success. Fill the output struct.
    parsed_tcmd_output->tcmd_idx = tcmd_idx;
    memset(parsed_tcmd_output->args_str_no_parens, 0, TCMD_ARGS_STR_NO_PARENS_SIZE); // Safety.
    memcpy(parsed_tcmd_output->args_str_no_parens, args_str_no_parens, arg_len + 1);
    parsed_tcmd_output->timestamp_sent = timestamp_sent;
    parsed_tcmd_output->timestamp_to_execute = timestamp_to_execute;
    parsed_tcmd_output->tcmd_channel = tcmd_channel;
    return 0;
}
