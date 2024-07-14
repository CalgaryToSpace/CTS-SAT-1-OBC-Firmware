#ifndef __INCLUDE_GUARD__TRANSFORMS_ARRAYS_H__
#define __INCLUDE_GUARD__TRANSFORMS_ARRAYS_H__

#include <stdint.h>

int16_t GEN_get_index_of_substring_in_array(const char *haystack_arr, int16_t haystack_arr_len, const char *needle_str);

void GEN_uint64_to_str(uint64_t value, char *buffer);


#endif // __INCLUDE_GUARD__TRANSFORMS_ARRAYS_H__
