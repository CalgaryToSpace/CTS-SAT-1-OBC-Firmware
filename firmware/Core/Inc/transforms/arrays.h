#ifndef INCLUDE_GUARD__TRANSFORMS_ARRAYS_H__
#define INCLUDE_GUARD__TRANSFORMS_ARRAYS_H__

#include <stdint.h>

int16_t GEN_get_index_of_substring_in_array(const char *haystack_arr, int16_t haystack_arr_len, const char *needle_str);

void GEN_uint64_to_str(uint64_t value, char *buffer);
void GEN_uint64_to_padded_str(uint64_t value, uint8_t padding_len, char *buffer);

void GEN_int64_to_str(int64_t value, char *buffer);

uint8_t GEN_hex_str_to_byte_array(const char *hex_str, uint8_t output_byte_array[],
    uint16_t output_byte_array_size, uint16_t *output_byte_array_len);

void GEN_byte_array_to_hex_str(
    const uint8_t *byte_array, uint32_t byte_array_len, char *dest_str, uint32_t dest_str_size
);

#endif // INCLUDE_GUARD__TRANSFORMS_ARRAYS_H__
