#include "telecommands/telecommand_args_helpers.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


/// @brief Extracts the longest substring of integer characters, starting from the beginning of the
///     string, to a maximum length or until the first non-integer character is found.
/// @param str Input string, starting with an integer
/// @param str_len Max length of the input string
/// @param result Pointer to the result
/// @return 0 if successful, 1 if the string is empty, 2 if the string does not start with an integer
uint8_t TCMD_ascii_to_uint64(const char *str, uint32_t str_len, uint64_t *result) {
    if (str_len == 0) {
        return 1;
    }

    uint64_t temp_result = 0;
    uint32_t i = 0;
    for (; i < str_len; i++) {
        if (str[i] < '0' || str[i] > '9') {
            break;
        }

        temp_result = temp_result * 10 + (str[i] - '0');
    }

    if (i == 0) {
        return 2;
    }

    *result = temp_result;
    return 0;
}

/// @brief Extracts the nth comma-separated argument from the input string, assuming it's a uint64
/// @param str Input string
/// @param str_len Length of the input string
/// @param arg_index Index of the argument to extract (0-based)
/// @param result Pointer to the result
/// @return 0 if successful, 1 if the string is empty, 2 if the string does not contain enough arguments
///        3 if the argument is not an integer, 4 for other error
uint8_t TCMD_extract_uint64_arg(const char *str, uint32_t str_len, uint8_t arg_index, uint64_t *result) {
    if (str_len == 0) {
        return 1;
    }

    uint32_t arg_count = 0;
    uint32_t i = 0;
    uint32_t start_index = 0;
    for (; i < str_len; i++) {
        if (str[i] == ',') {
            if (arg_count == arg_index) {
                break;
            }

            arg_count++;
            start_index = i + 1;
        }
    }

    if (arg_count < arg_index) {
        return 2;
    }

    uint8_t parse_result = TCMD_ascii_to_uint64(&str[start_index], i - start_index, result);
    if (parse_result == 2) {
        // The argument is not an integer
        return 3;
    }
    else if (parse_result > 0) {
        // Other error
        return 4;
    }
    return 0;
}

/// @brief Extracts the nth comma-separated argument from the input string, assuming it's a string
/// @param str Input string (null-terminated)
/// @param arg_index Index of the argument to extract (0-based)
/// @param result Pointer to the result, to be filled with the extracted string (null-terminated)
/// @param result_max_len Maximum length of the result
/// @return 0 if successful, 1 if the string is empty, 2 if the string does not contain enough arguments
///         3 for other error
uint8_t TCMD_extract_string_arg(const char *str, uint8_t arg_index, char *result, uint16_t result_max_len) {
    const uint16_t str_len = strlen(str);
    if (str_len == 0) {
        return 1;
    }

    uint32_t arg_count = 0;
    uint32_t i = 0;
    uint32_t start_index = 0;
    for (; i < str_len; i++) {
        if (str[i] == ',') {
            if (arg_count == arg_index) {
                break;
            }

            arg_count++;
            start_index = i + 1;
        }
    }

    if (arg_count < arg_index) {
        return 2;
    }
    
    uint32_t end_index = i;
    if (end_index - start_index >= result_max_len) {
        return 3;
    }

    // Trim leading whitespace
    while (start_index < end_index && (str[start_index] == ' ' || str[start_index] == '\t')) {
        start_index++;
    }

    // Trim trailing whitespace
    while (end_index > start_index && (str[end_index - 1] == ' ' || str[end_index - 1] == '\t')) {
        end_index--;
    }

    uint32_t token_len = end_index - start_index;

    if (token_len >= result_max_len) {
        // The argument is too long for the result buffer.
        return 3;
    }

    strncpy(result, &str[start_index], token_len);
    result[token_len] = '\0';

    return 0; // Successful extraction
}

/// @brief Extracts the longest substring of double characters, starting from the beginning of the
///     string, to a maximum length or until the first non-double character is found.
/// @param str Input string, starting with a double
/// @param str_len Max length of the input string
/// @param result Pointer to the result
/// @return 0 if successful, 1 if the string is empty, 2 if the string does not start with a double, 3 for other issues
uint8_t TCMD_ascii_to_double(const char *str, uint32_t str_len, double *result) {
    if (str_len == 0) {
        return 1;
    }

    for (uint32_t i = 0; i < str_len; i++) {
        // iterate through the string to find the first non-whitespace character
        if (isdigit(str[i])) {
            // so long as the first character is a digit, atof can handle it
            double temp_result = atof(str);
            *result = temp_result;
            return 0;
        }
        else if (!isdigit(str[i]) && !isspace(str[i])) {
            // atof removes whitespace, so we only need to check for other characters
            // atof also ignores all subsequent non-double characters following the double
            return 2;
        }
    }

    // if this return is reached, there has been another issue in the input
    return 3;

}

/// @brief Extracts the nth comma-separated argument from the input string, assuming it's a double
/// @param str Input string
/// @param str_len Length of the input string
/// @param arg_index Index of the argument to extract (0-based)
/// @param result Pointer to the result
/// @return 0 if successful, 1 if the string is empty, 2 if the string does not contain enough arguments
///        3 if the argument is not a double, 4 for other error
uint8_t TCMD_extract_double_arg(const char *str, uint32_t str_len, uint8_t arg_index, double *result) {
    if (str_len == 0) {
        return 1;
    }

    uint32_t arg_count = 0;
    uint32_t i = 0;
    uint32_t start_index = 0;
    for (; i < str_len; i++) {
        if (str[i] == ',') {
            if (arg_count == arg_index) {
                break;
            }

            arg_count++;
            start_index = i + 1;
        }
    }

    if (arg_count < arg_index) {
        return 2;
    }

    uint8_t parse_result = TCMD_ascii_to_double(&str[start_index], i - start_index, result);
    if (parse_result == 2) {
        // The argument is not a double
        return 3;
    }
    else if (parse_result > 0) {
        // Other error
        return 4;
    }
    return 0;
}
