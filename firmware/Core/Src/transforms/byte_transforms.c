#include "transforms/byte_transforms.h"

uint32_t GEN_convert_big_endian_four_byte_array_to_uint32(uint8_t four_byte_array[]) {
    // recall: big endian means the Most Significant Bit (MSB) is at index 0 (left side)
    return (four_byte_array[0] << 24) | (four_byte_array[1] << 16) |
            (four_byte_array[2] << 8)  | four_byte_array[3];
}

uint32_t convert_little_endian_four_byte_array_to_uint32(uint8_t four_byte_array[]) {
    // recall: little endian means the Least Significant Bit (LSB) is at index 0 (left side)
    return (four_byte_array[3] << 24) | (four_byte_array[2] << 16) |
            (four_byte_array[1] << 8)  | four_byte_array[0];
}
