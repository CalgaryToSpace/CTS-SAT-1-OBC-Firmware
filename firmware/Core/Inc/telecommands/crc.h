#ifndef __CRC_H
#define __CRC_H

#include <stdint.h>
#include "stm32l4xx_hal.h"

// Function prototypes
uint32_t GEN_crc32_checksum(CRC_HandleTypeDef *hcrc,uint8_t *input_arr, uint32_t input_arr_len);

#endif /* __CRC_H */