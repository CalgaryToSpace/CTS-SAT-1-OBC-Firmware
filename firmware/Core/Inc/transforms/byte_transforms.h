
#ifndef INCLUDE_GUARD__BYTE_TRANSFORMS_H__
#define INCLUDE_GUARD__BYTE_TRANSFORMS_H__

#include <stdint.h>

uint32_t GEN_convert_big_endian_four_byte_array_to_uint32(uint8_t four_byte_array[]);
uint32_t GEN_convert_little_endian_four_byte_array_to_uint32(uint8_t four_byte_array[]);


#endif // INCLUDE_GUARD__BYTE_TRANSFORMS_H__
