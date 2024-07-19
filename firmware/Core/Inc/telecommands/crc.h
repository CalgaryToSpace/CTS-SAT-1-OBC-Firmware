#ifndef __CRC_H
#define __CRC_H

#include <stdint.h>

// Function prototypes
void GEN_crc32_checksum(uint8_t *input_arr, uint32_t input_arr_len, uint32_t *output_arr);

#endif /* __CRC_H */