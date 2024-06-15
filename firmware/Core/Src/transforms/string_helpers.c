
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "transforms/string_helpers.h"

/// @brief Populates result array with the first string in input_str separated by the delimiter. Input string is modified
///     to point to the remaining string.
/// @param input_str Input string, modified
/// @param input_str_len Length of the input string
/// @param result Pointer to the result
/// @return 0 if successful, 1 if the string is empty
uint8_t split_string_by_delimiter(char **input_str, const uint8_t input_str_len, const char delim, char *result)
{
    if (input_str_len == 0)
    {
        return 1;
    }
    uint8_t i = 0;
    for (; i < input_str_len; i++)
    {
        const char current_char = (*input_str)[i];
        if (current_char == delim)
        {
            break;
        }
        result[i] = current_char;
    }
    result[i] = '\0';
    *input_str = (*input_str + i + 1);
    return 0;
};