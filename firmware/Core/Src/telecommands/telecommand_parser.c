#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_parser.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/// @brief Returns whether a character is alphanumeric (0-9, A-Z, a-z).
/// @param c The character to check.
/// @return 1 if the character is alphanumeric, 0 otherwise.
uint8_t TCMD_is_char_alphanumeric(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
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
    int32_t tcmd_name_len = tcmd_str_len - TCMD_PREFIX_STR_LEN;
    for (int32_t i = TCMD_PREFIX_STR_LEN; i < tcmd_str_len; i++) {
        if (!TCMD_is_char_valid_telecommand_name_char(tcmd_str[i])) {
            tcmd_name_len = i - TCMD_PREFIX_STR_LEN;
            break;
        }
    }

    // Iterate through the table of telecommands, and see if any match the given telecommand string
    for (int32_t check_cmd_idx = 0; check_cmd_idx < TCMD_NUM_TELECOMMANDS; check_cmd_idx++) {
        if (tcmd_name_len != strlen(TCMD_telecommand_definitions[check_cmd_idx].tcmd_name)) {
            continue;
        }

        for (int32_t str_idx = 0; str_idx < tcmd_name_len; str_idx++) {
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
