#include "telecommands/telecommand_args_helpers.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>


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

/// @brief Provides the index into the command string corresponding to the start of the requested argument number
/// @param str Input string
/// @param str_len Length of the input string
/// @param arg_index Index of the argument to extract (0-based)
/// @param result Pointer to the result
/// @return 0 if successful, 1 if the string is empty, 2 if the string does not contain enough arguments
// TODO unit tests for this
uint8_t TCMD_get_arg_info(const char *str, uint32_t str_len, uint8_t arg_index, uint32_t *first_index, uint32_t *last_index, uint32_t *arg_length) {
    if (str_len == 0) {
        return 1;
    }

    uint32_t arg_count = 0;
    uint32_t i = 0;
    uint32_t start_index = 0;
    uint32_t stop_index = 0;
    for (; i < str_len; i++) {
        // Only one ( is allowed per command
        if (str[i] == '(') {
            start_index = i + 1;
        }
        else if (str[i] == ')') {
            stop_index = i - 1;
            break;
        }
        else if (str[i] == ',' ) {
            stop_index = i - 1;
            if (arg_count == arg_index ) {
                break;
            }
            arg_count++;
            start_index = i + 1;
        }
    }

    if (arg_count < arg_index) {
        return 2;
    }

    if (first_index != NULL) {
        *first_index = start_index;
    }
    
    if (last_index != NULL) {
        *last_index = stop_index;
    }
    
    if (arg_length != NULL) {
        *arg_length = stop_index - start_index + 1;
    }

    return 0;
}

/// @brief Extract argument as a character string
/// @param str Input string
/// @param str_len Length of the input string
/// @param arg_index Index of the argument to extract (0-based)
/// @param result Pointer to the result, which must be a pre-allocated array of length TCMD_MAX_STRING_LEN
/// @return 0 if successful, 1 if the string is empty, 2 if the string does not contain enough arguments
// TODO unit tests for this
uint8_t TCMD_arg_as_string(const char *str, uint32_t str_len, uint8_t arg_index, char *result) {

    uint32_t first_index = 0;
    uint32_t arg_length = 0;
    uint8_t parse_result = TCMD_get_arg_info(str, str_len, arg_index, &first_index, NULL, &arg_length);
    if (parse_result > 0) {
        return parse_result;
    }
    if (arg_length >= TCMD_MAX_STRING_LEN) {
        arg_length = TCMD_MAX_STRING_LEN - 1;
    }

    memcpy(result, str + first_index, arg_length);
    result[arg_length] = '\0';

    return 0;
}

uint8_t TCMD_arg_base64_decode(const char *str, uint32_t str_len, uint8_t arg_index, uint8_t *result, uint32_t *result_len) {

    char base64[TCMD_MAX_STRING_LEN] = {0};
    uint8_t parse_result = TCMD_arg_as_string(str, str_len, arg_index, base64);
    if (parse_result > 0) {
        return parse_result;
    }

    uint32_t base64_len = strlen(base64);
    uint32_t byte_counter = 0;
    char chars[4] = {0};

    for (int i = 0; i < base64_len; i+=4) {
        for (int j = 0; j < 4; j++) {
            chars[j] = TCMD_base64_decode_character(base64[i+j]);
        }

        if (byte_counter + 2 >= *result_len) {
            // Ran out of array to store bytes in
            return 1;
        }
        result[byte_counter] = (chars[0] << 2) | (chars[1] >> 4);
        result[byte_counter + 1] = (chars[1] << 4) | (chars[2] >> 2);
        result[byte_counter + 2] = (chars[2] << 6) | chars[3];
        byte_counter += 3;
    }

    *result_len = byte_counter;

    return 0;
}

uint8_t TCMD_base64_decode_character(char c) {
    if (c == '/') 
        return 63;
    else if (c == '+')
        return 62;
    else if (c >= '0' && c <= '9')
        return c + 4;
    else if (c >= 'a' && c <= 'z')
        return c - 71;
    else if (c >= 'A' && c <= 'Z')
        return c - 65;
    else 
        return 0; // '=' and any erroneous characters are interpreted as padding
                    
}
