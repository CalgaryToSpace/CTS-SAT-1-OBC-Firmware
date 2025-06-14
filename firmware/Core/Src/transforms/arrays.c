#include "transforms/arrays.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>

/// @brief Returns the index of the first character of the first occurrence of a substring in an array.
/// @param haystack_arr The array to search within.
/// @param haystack_arr_len The length of the array to search within.
/// @param needle_str The substring to search for. Must be a null-terminated C-string.
/// @return The index of the start of the first occurrence of the needle_str in the array, or -1 if not found
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

/// @brief Returns the index of the first element of the first occurrence of a sub-array in an array.
/// @param haystack_arr The array to search within.
/// @param haystack_arr_len The length of the array to search within.
/// @param needle_arr The substring/sub-array to search for.
/// @param needle_arr_len The length of the substring/sub-array to search for.
/// @return The index of the start of the first occurrence of the substring in the array, or -1 if not found
int16_t GEN_get_index_of_subarray_in_array(
    const uint8_t *haystack_arr, int16_t haystack_arr_len,
    const uint8_t *needle_arr, int16_t needle_arr_len
) {    
    // Iterate through the haystack array
    for (int16_t i = 0; i <= haystack_arr_len - needle_arr_len; i++) {
        // Check if the substring matches at position i
        int16_t j;
        for (j = 0; j < needle_arr_len; j++) {
            if (haystack_arr[i + j] != needle_arr[j]) {
                break;
            }
        }
        // If we completed the inner loop, we found the substring
        if (j == needle_arr_len) {
            return i;
        }
    }

    // If the substring was not found, return -1
    return -1;
}

/// @brief Converts a uint64_t to a string.
/// @param value The input value to convert.
/// @param buffer The output buffer to write the string to. MUST BE AT LEAST 21 BYTES LONG.
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


/// @brief Converts a uint64_t to a string, with leading zeros.
/// @param value The input value to convert.
/// @param padding_len The expected length of the output string.
/// @param buffer The output buffer to write the string to. Must be `padding_len` bytes long.
void GEN_uint64_to_padded_str(uint64_t value, uint8_t padding_len, char *buffer) {
    // Ensure the buffer is valid and large enough
    if (buffer == NULL) {
        return;
    }

    char temp[32];
    int index = 0;

    // Handle the case where the value is 0
    if (value == 0) {
        temp[index++] = '0';
    } else {
        // Convert the uint64_t value to a string in reverse order
        while (value > 0) {
            temp[index++] = '0' + (value % 10);
            value /= 10;
        }
    }

    // Add padding if necessary
    while (index < padding_len) {
        temp[index++] = '0';
    }

    // Reverse the string to correct the order and copy to buffer
    for (int i = 0; i < index; ++i) {
        buffer[i] = temp[index - i - 1];
    }
    buffer[index] = '\0';  // Null-terminate the buffer
}


/// @brief Converts an int64_t to a string.
/// @param value The input value to convert.
/// @param buffer The output buffer to write the string to. MUST BE AT LEAST 21 BYTES LONG.
void GEN_int64_to_str(int64_t value, char *buffer) {
    // Ensure the buffer is valid and large enough
    if (buffer == NULL) {
        return;
    }

    uint64_t absolute_value;
    
    // Handle negative numbers (we can't use abs() for fixed-length integers)
    if (value < 0) {
        buffer[0] = '-';
        absolute_value = -value; 
    } else {
        absolute_value = value;
    }

    GEN_uint64_to_str(absolute_value, &buffer[(value < 0)]); // leave room for the '-' if it's negative
    
}

/// @brief Converts a hex string to a byte array
/// @param hex_str  The input hex string. Can be upper or lower case. Can contain spaces and underscores between bytes.
/// @param output_byte_array Pointer to the output destination byte array.
/// @param output_byte_array_size Maximum size of the output byte array.
/// @param output_byte_array_len Pointer to the output variable that will be set to the length of the output byte array.
/// @return 0 if successful, >0 for error
/// @note Delimiters between bytes are ignored, but delimiters within a byte are not allowed.
uint8_t GEN_hex_str_to_byte_array(const char *hex_str, uint8_t output_byte_array[],
    uint16_t output_byte_array_size, uint16_t *output_byte_array_len)
{
    const size_t hex_str_len = strlen(hex_str);
    if (hex_str_len == 0) {
        // String is empty
        return 1;
    }

    uint16_t byte_index = 0;
    uint8_t current_byte = 0;
    uint8_t nibble_count = 0;

    for (size_t i = 0; i < hex_str_len; i++) {
        char current_char = hex_str[i];

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
            if (byte_index >= output_byte_array_size) {
                // Output array size exceeded
                return 3;
            }
            output_byte_array[byte_index++] = current_byte;
            current_byte = 0;
            nibble_count = 0;
        }
    }

    if (nibble_count != 0) {
        // Odd number of nibbles (half a byte)
        return 4;
    }

    *output_byte_array_len = byte_index;
    return 0;
}

/// @brief Writes a byte array to a hex string (no spaces between bytes).
/// @param byte_array Input byte array.
/// @param byte_array_len Length of input `byte_array`.
/// @param dest_str The destination to write a C-string to.
/// @param dest_str_size The size of the `dest_str` array, allocated before calling. Must be at least
///     `byte_array_len * 3 + 1` to fit the entire string.
void GEN_byte_array_to_hex_str(
    const uint8_t *byte_array, uint32_t byte_array_len, char *dest_str, uint32_t dest_str_size
) {
    if (dest_str_size == 0) return; // no space at all
    char *ptr = dest_str;
    uint32_t remaining = dest_str_size;

    for (uint32_t i = 0; i < byte_array_len; ++i) {
        if (remaining < 3) break;  // not enough space for 2 chars + null terminator
        int written = snprintf(ptr, remaining, "%02X", byte_array[i]);
        if (written != 2) break;   // safety check (should always write 2 characters)
        ptr += 2;
        remaining -= 2;
    }

    *ptr = '\0'; // null-terminate even if truncated
}
