#ifndef __INCLUDE_GUARD__GPS_H__
#define __INCLUDE_GUARD__GPS_H__

#include "gps/gps_types.h"

uint32_t GPS_crc32_value(uint8_t i);
uint32_t GPS_calculate_block_crc32( uint32_t ulCount, uint8_t *ucBuffer );

uint8_t GPS_reference_time_status_str_to_enum(const char *status_str, GPS_reference_time_status_t *status);
uint8_t GPS_solution_status_str_to_enum(const char *status_str, GPS_solution_status_enum_t *status);
uint8_t GPS_position_type_str_to_enum(const char *type_str, GPS_position_type_enum_t *type);

uint8_t GPS_header_response_parser(const char* data_received, GPS_header_response_t *result);
uint8_t GPS_bestxyza_data_parser(const char* data_received, GPS_bestxyza_response_t *result);
uint8_t GPS_timea_data_parser(const char* data_received, GPS_timea_response_t *result);

#endif // __INCLUDE_GUARD__GPS_H__
