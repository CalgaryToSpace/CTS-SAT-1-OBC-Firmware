#include "transforms/arrays.h"

#include <string.h>


/// @brief Returns the index of the first character of the first occurrence of a substring in an array.
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

/// @brief Converts a uint64_t to a string.
/// @param value The input value to convert.
/// @param buffer The output buffer to write the string to. MUST BE 32 BYTES LONG.
void GEN_uint64_to_str(uint64_t value, char *buffer) {
    // Ensure the buffer is valid and large enough
    if (buffer == NULL) {
        return;
    }

    char temp[32];
    int index = 0;

    // Handle the case where the value is 0
    if (value == 0) {
        buffer[index++] = '0';
        buffer[index] = '\0';
        return;
    }

    // Convert the uint64_t value to a string in reverse order
    while (value > 0) {
        temp[index++] = '0' + (value % 10);
        value /= 10;
    }

    // Reverse the string to correct the order
    for (int i = 0; i < index; ++i) {
        buffer[i] = temp[index - i - 1];
    }
    buffer[index] = '\0';
}
