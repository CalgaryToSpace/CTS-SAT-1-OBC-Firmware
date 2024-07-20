#include "transforms/arrays.h"
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
    // FIXME: return error if the string is too long/number is too large
    // FIXME: return error if the number doesn't occupy the whole string (e.g., "123abc" with str_len=6 should error)
    // TODO: write unit tests for this function
    // TODO: consider removing the str_len parameter and using strlen(str) instead (requires refactor in caller)
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



/// @brief Extracts the nth comma-separated argument from the input string, assuming it's a hex string.
/// @param args_str Input string containing comma-separated arguments (null-terminated)
/// @param arg_index Index of the argument to extract (0-based)
/// @param result Pointer to the result; a byte array containing the values of the hex string 
/// @param result_array_size Size of the result array
/// @param result_length Pointer to variable that will contain the length of the result after converting
/// @return 0 if successful, >0 for error
/// @note Delimiters between bytes are ignored, but delimiters within a byte are not allowed.
uint8_t TCMD_extract_hex_array_arg(const char *args_str, uint8_t arg_index, uint8_t result_array[],
    uint16_t result_array_size, uint16_t *result_length )
{
    const size_t args_str_len = strlen(args_str);
    if (args_str_len == 0) {
        // String is empty
        return 10;
    }

    // Find the start index of the argument.
    uint32_t arg_count = 0;
    uint32_t start_index = 0;
    for (uint32_t i = 0; i < args_str_len; i++) {
        if (args_str[i] == ',') {
            if (arg_count == arg_index) {
                break;
            }
            arg_count++;
            start_index = i + 1;
        }
    }
    
    if (arg_count < arg_index) {
        // Not enough arguments
        return 11;
    }

    if (strlen(&args_str[start_index]) < 2) {
        // Empty argument, or not enough characters to form a byte
        return 12; 
    }

    // Find the end index of the argument.
    // `end_index` will point to the character after the last character of the argument.
    uint32_t end_index = start_index;
    while (args_str[end_index] != ',' && args_str[end_index] != '\0') {
        end_index++;
    }
    
    // Parse the hex string into a byte array.
    uint16_t byte_index = 0;
    uint8_t current_byte = 0;
    uint8_t nibble_count = 0;

    for (size_t i = start_index; i < end_index; i++) {
        char current_char = args_str[i];

        if (current_char == ' ' || current_char == '_') {
            if (nibble_count % 2 != 0) {
                // Separator found in the middle of a byte.
                return 4;
            }
            // Skip spaces and underscores
            continue;
        }

        if (!isxdigit(current_char)) {
            // Invalid character found
            return 2;
        }

        current_char = tolower(current_char);

        // Incantation to convert a hex character to a nibble.
        uint8_t nibble = (uint8_t)((current_char >= '0' && current_char <= '9') ? (current_char - '0') : (current_char - 'a' + 10));

        current_byte = (current_byte << 4) | nibble;
        nibble_count++;

        if (nibble_count == 2) {
            if (byte_index >= result_array_size) {
                // Output array size exceeded
                return 3;
            }
            result_array[byte_index++] = current_byte;
            current_byte = 0;
            nibble_count = 0;
        }
    }

    if (nibble_count != 0) {
        // Odd number of nibbles (half a byte)
        return 4;
    }

    *result_length = byte_index;
    return 0;
}
/// @brief Extracts the longest substring of double characters, starting from the beginning of the
///     string, to a maximum length or until the first non-double character is found.
/// @param str Input string, starting with a double
/// @param str_len Max length of the input string
/// @param result Pointer to the result
/// @return 0 if successful, 1 if the string is empty, 2 if the string does not start with a double
uint8_t TCMD_ascii_to_double(const char *str, uint32_t str_len, double *result) {
    
    if (str_len == 0) {
        // check for an empty string
        return 1;
    }

    uint8_t number_of_decimal_points_found = 0;
    for (uint8_t i = 0; i < str_len; i++) { // main loop
        const char iter_char = str[i];
        // negative sign at start is allowed
        if (i==0 && iter_char == '-') { 
            continue; 
        }

        // decimal sign is allowed once, but not at the start/end
        if ( (iter_char == '.') && (number_of_decimal_points_found == 0) && (i != 0 && i != str_len-1) ) {
            number_of_decimal_points_found += 1;
            continue;
        }

        // if it's a number, continue
        if (isdigit(iter_char)) { continue; }

        // if we've reached the end of the valid conditions for this char, it's invalid
        return 2; // invalid char
    }

    // now that we know they're valid chars, use atof
    const double temp_result = atof(str);
    *result = temp_result;

    return 0;

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
