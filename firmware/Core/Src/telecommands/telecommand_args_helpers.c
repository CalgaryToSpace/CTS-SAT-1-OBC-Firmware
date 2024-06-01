#include "telecommands/telecommand_args_helpers.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


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
/// @param str Input string
/// @param str_len Length of the input string
/// @param arg_index Index of the argument to extract (0-based)
/// @param result Pointer to the result
/// @return 0 if successful, 1 if the string is empty, 2 if the string does not contain enough arguments
///         3 for other error
uint8_t TCMD_extract_string_arg(const char *str, uint32_t str_len, uint8_t arg_index, char **result) {
    if (str_len == 0) {
        return 1;
    }

    // FIXME: This function needs to be converted to using the stack. The result should be a single pointer.

    char *copy = strdup(str); // Make a copy of the input string
    if (copy == NULL)
        return 3;

    char *token = strtok(copy, ",");
    int current_index = 0;
    while (token != NULL && current_index < arg_index) {
        token = strtok(NULL, ",");
        current_index++;
    }

    if (token == NULL) {
        free(copy);
        return 2;
    }

    // Trim leading and trailing whitespace from the token
    while (*token == ' ' || *token == '\t') {
        token++;
    }
    size_t token_len = strlen(token);
    while (token_len > 0 && (token[token_len - 1] == ' ' || token[token_len - 1] == '\t')) {
        token_len--;
    }

    *result = (char *)malloc(token_len + 1);
    if (*result == NULL) {
        free(copy);
        return 3;
    }
    
    strncpy(*result, token, token_len);
    (*result)[token_len] = '\0';

    free(copy);
    return 0; // Successful extraction
}