#ifndef __CRC_H
#define __CRC_H

#include <stdint.h>
// #include "stm32l4xx.h"
// #include "stm32l4xx_hal_crc.h"
#include "stm32l4xx_hal.h"

extern CRC_HandleTypeDef hcrc1;

// Function prototypes
void GEN_crc32_checksum(CRC_HandleTypeDef *hcrc,uint8_t *input_arr, uint32_t input_arr_len, int64_t *output_arr);

#endif /* __CRC_H */