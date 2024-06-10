
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "helpers/helpers.h"

/// @brief Populates an array of strings by splitting a string by a delimiter
/// @param input_str Input String
/// @param input_str_len length of input string
/// @param delim any character
/// @param result 2d array of strings to be populated, enusre that the array is clearerd before passing in,
/// or undefined behavior may occur
/// @param max_segments Max number of strings to be populated
/// @return Number of strings populated
/// @note By default, each string inside the result array is 20 characters long
/// if this is not enough for the string, need to update approach of increase size of array
// TODO: Update approach of increase size of array
// TODO: Update input_str_len so that it is not needed
uint8_t split_string_by_delimiter(const uint8_t *input_str, const uint8_t input_str_len, const uint8_t delim, uint8_t result[][20], const uint8_t max_segments)
{
    if (input_str == NULL || result == NULL || max_segments == 0 || input_str_len == 0)
    {
        return 0;
    }

    uint8_t count = 0;
    uint8_t start_of_word = 0;

    for (uint8_t pos_in_str = 0; pos_in_str <= input_str_len; pos_in_str++)
    {
        if (pos_in_str != input_str_len && input_str[pos_in_str] != delim)
        {
            continue;
        }

        if (count >= max_segments)
        {
            break;
        }

        // copy over word
        const uint8_t word_len = pos_in_str - start_of_word;
        if (word_len == 0)
        {
            start_of_word++;
            continue;
        }
        uint8_t copy[word_len + 1]; // account for null terminator
        strncpy((char *)copy, (char *)(input_str + start_of_word), word_len);
        copy[word_len] = '\0';

        // add word to result
        strncpy((char *)result[count], (char *)copy, word_len);

        count++;

        start_of_word = pos_in_str + 1;
    }

    return count;
}