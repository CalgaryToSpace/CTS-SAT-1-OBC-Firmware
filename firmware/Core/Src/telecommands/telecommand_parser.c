#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_parser.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/// @brief Returns whether a character is alphanumeric (0-9, A-Z, a-z).
/// @param c The character to check.
/// @return 1 if the character is alphanumeric, 0 otherwise.
uint8_t is_char_alphanumeric(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

/// @brief Returns whether a character is a valid character in a telecommand name.
/// @param c The character to check.
/// @return 1 if the character is valid, 0 otherwise.
uint8_t is_char_valid_telecommand_name_char(char c) {
    return is_char_alphanumeric(c) || c == '_';
}

/// @brief Finds an index into TCMD_telecommand_definitions for the given telecommand string.
/// @details This function searches for the longest matching telecommand string in TCMD_telecommand_definitions, where all
///          alpha-numeric characters at the start of the to-be-parsed telecommand match the telecommand string in
///          TCMD_telecommand_definitions. This function is case-sensitive.
/// @param tcmd_str The telecommand string to search for.
/// @return The index into TCMD_telecommand_definitions for the given telecommand string, or -1 if not found.
int32_t TCMD_parse_telecommand_get_index(const char *tcmd_str, uint32_t tcmd_str_len) {
    // Find the length of the telecommand name part of the "tcmd_str"
    int32_t tcmd_str_telecommand_name_len = tcmd_str_len;
    for (int32_t i = 0; i < tcmd_str_len; i++) {
        if (!is_char_valid_telecommand_name_char(tcmd_str[i])) {
            tcmd_str_telecommand_name_len = i;
            break;
        }
    }

    for (int32_t check_cmd_idx = 0; check_cmd_idx < TCMD_NUM_TELECOMMANDS; check_cmd_idx++) {
        if (tcmd_str_telecommand_name_len != strlen(TCMD_telecommand_definitions[check_cmd_idx].tcmd_name)) {
            continue;
        }
        for (int32_t str_idx = 0; str_idx < tcmd_str_telecommand_name_len; str_idx++) {
            char rxd_char = tcmd_str[str_idx];
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
