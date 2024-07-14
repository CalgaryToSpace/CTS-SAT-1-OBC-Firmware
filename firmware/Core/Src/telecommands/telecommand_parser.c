#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_parser.h"
#include "telecommands/telecommand_args_helpers.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

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

/// @brief Returns the index of the first occurrence of a substring in an array.
/// @param haystack_arr The array to search within.
/// @param haystack_arr_len The length of the array to search within.
/// @param substring The substring to search for. Must be a null-terminated C-string.
/// @return The index of the start of the first occurrence of the substring in the array, or -1 if not found
int16_t GEN_get_index_of_substring_in_array(const char *haystack_arr, int16_t haystack_arr_len, const char *needle_str) {
    uint16_t needle_str_len = strlen(needle_str);
    
    // Iterate through the haystack array
    for (int16_t i = 0; i <= haystack_arr_len - needle_str_len; i++) {
        // Check if the substring matches at position i
        int16_t j;
        for (j = 0; j < needle_str_len; j++) {
            if (haystack_arr[i + j] != needle_str[j]) {
                break;
            }
        }
        // If we completed the inner loop, we found the substring
        if (j == needle_str_len) {
            return i;
        }
    }

    // If the substring was not found, return -1
    return -1;
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
/// @note This function will return an error if the character after the number is not in: `#\@\0`
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

    // Check that the character after the number is any of '#', '@', or '\0'
    if (str[value_end_index] != '#' && str[value_end_index] != '@' && str[value_end_index] != '\0') {
        return 4;
    }

    // Copy the value into a buffer
    char value_str[32]; // uint64 needs 20 chars max
    memset(value_str, 0, sizeof(value_str));
    if (value_end_index - value_start_index >= sizeof(value_str) - 1) {
        // Failure: digit string too long
        return 5;
    }
    strncpy(value_str, str + value_start_index, value_end_index - value_start_index);

    // Convert the value to a uint64_t
    uint64_t value;
    if (TCMD_ascii_to_uint64(value_str, strlen(value_str), &value) > 0) {
        return 6;
    }

    // Set the value
    *value_dest = value;
    
    // Success
    return 0;
}
