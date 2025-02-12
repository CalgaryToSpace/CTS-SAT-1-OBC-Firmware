#ifndef __CRC_H
#define __CRC_H

/*
Online CRC calculator for reference: https://www.sunshine2k.de/coding/javascript/crc/crc_js.html

Select CRC32, custom parameters:
Input reflected: false
Outout reflected: false
Initial value: 0xFFFFFFFF
Final Xor value: 0
*/

#include <stdint.h>
#include "stm32l4xx_hal.h"

uint32_t GEN_crc32_checksum(uint8_t *input_message, uint32_t input_message_len);
#endif /* __CRC_H */