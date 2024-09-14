#ifndef __INCLUDE_GUARD__TRANSFORMS_ARRAYS_H__
#define __INCLUDE_GUARD__TRANSFORMS_ARRAYS_H__

#include <stdint.h>

int16_t GEN_get_index_of_substring_in_array(const char *haystack_arr, int16_t haystack_arr_len, const char *needle_str);

void GEN_uint64_to_str(uint64_t value, char *buffer);
void GEN_int64_to_str(int64_t value, char *buffer);

uint8_t GEN_hex_str_to_byte_array(const char *hex_str, uint8_t output_byte_array[],
    uint16_t output_byte_array_size, uint16_t *output_byte_array_len);

#endif // __INCLUDE_GUARD__TRANSFORMS_ARRAYS_H__
