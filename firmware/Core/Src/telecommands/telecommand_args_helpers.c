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
uint8_t TCMD_ascii_to_uint64(const char *str, uint32_t str_len, uint64_t *result)
{
    if (str_len == 0)
    {
        return 1;
    }

    uint64_t temp_result = 0;
    uint32_t i = 0;
    for (; i < str_len; i++)
    {
        if (str[i] < '0' || str[i] > '9')
        {
            break;
        }

        temp_result = temp_result * 10 + (str[i] - '0');
    }

    if (i == 0)
    {
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
uint8_t TCMD_extract_uint64_arg(const char *str, uint32_t str_len, uint8_t arg_index, uint64_t *result)
{
    if (str_len == 0)
    {
        return 1;
    }

    uint32_t arg_count = 0;
    uint32_t i = 0;
    uint32_t start_index = 0;
    for (; i < str_len; i++)
    {
        if (str[i] == ',')
        {
            if (arg_count == arg_index)
            {
                break;
            }

            arg_count++;
            start_index = i + 1;
        }
    }

    if (arg_count < arg_index)
    {
        return 2;
    }

    uint8_t parse_result = TCMD_ascii_to_uint64(&str[start_index], i - start_index, result);
    if (parse_result == 2)
    {
        // The argument is not an integer
        return 3;
    }
    else if (parse_result > 0)
    {
        // Other error
        return 4;
    }
    return 0;
}

// Function to parse a hex string into raw bytes
/**
 * @brief Parses hex string argument into raw bytes
 *
 * @param args_str Input string
 * @param str_len Length of the input string
 * @param arg_index Index of the argument to extract (0-based)
 * @param result Pointer to the result
 * @return 0 if successful, 1 if the string is empty, 2 if invalid input length, 3 if the string does not contain enough arguments
 *           4 if the argument is not an integer, 5 for other error
 */
int TCMD_hex_string_to_bytes(const uint8_t *args_str, uint32_t str_len, uint8_t arg_index, uint8_t *result)
{
    // Enpty argument
    if (str_len == 0)
    {
        return 1;
    }

    // The length of the hex string must be even
    if (str_len % 2 != 0)
    {
        return 2; // Invalid input length
    }

    uint32_t arg_count = 0;
    uint32_t i = 0;
    uint32_t start_index = 0;

    // Parse each pair of hex characters
    for (; i < str_len / 2; i++)
    {
        // Convert the current pair of hex characters to a byte
        char hex_pair[3] = {args_str[start_index], args_str[start_index + 1], '\0'};
        char *endptr;
        unsigned long byte_value = strtoul(hex_pair, &endptr, 16);

        // Check if the conversion was successful
        if (*endptr != '\0')
        {
            return 4; // Argument is not an integer
        }

        // Store the byte value in the result array
        if (arg_count == arg_index)
        {
            result = (uint8_t)byte_value;
            return 0; // Successful parsing
        }

        // Move to the next pair of hex characters
        start_index += 2;
        arg_count++;
    }

    // Check if the argument index is out of bounds
    if (arg_index >= arg_count)
    {
        return 3; // Not enough arguments
    }

    return 5; // Other error
}
