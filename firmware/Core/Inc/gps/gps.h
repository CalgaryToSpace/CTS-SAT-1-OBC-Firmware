#ifndef __INCLUDE_GUARD__GPS_H__
#define __INCLUDE_GUARD__GPS_H__

#include <stdint.h>

uint32_t GPS_crc32_value(uint8_t i);
uint32_t GPS_calculate_block_crc32( uint32_t ulCount, uint8_t *ucBuffer );

#endif // __INCLUDE_GUARD__GPS_H__
