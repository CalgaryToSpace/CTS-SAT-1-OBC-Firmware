#ifndef __INCLUDE_GUARD__GPS_H__
#define __INCLUDE_GUARD__GPS_H__

#include "gps/gps_types.h"

uint32_t crc32_value(uint8_t i);
uint32_t calculate_block_crc32( uint32_t ulCount, uint8_t *ucBuffer );

uint8_t parse_gps_header(const char* data_received, gps_response_header *result);
uint8_t parse_bestxyza_data(const char* data_received, gps_bestxyza_response *result);
uint8_t parse_timea_data(const char* data_received, gps_timea_response *result);

#endif // __INCLUDE_GUARD__GPS_H__
