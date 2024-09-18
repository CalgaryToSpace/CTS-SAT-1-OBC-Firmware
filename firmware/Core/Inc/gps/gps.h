#ifndef __INCLUDE_GUARD__GPS_H__
#define __INCLUDE_GUARD__GPS_H__

#include "gps/gps_types.h"

uint32_t GPS_crc32_value(uint8_t i);
uint32_t calculate_block_crc32( uint32_t ulCount, uint8_t *ucBuffer );

uint8_t GPS_reference_time_status_str_to_enum(const char *status_str, GPS_reference_time_status_t *status);
uint8_t assign_gps_solution_status(const char *status_str, GPS_solution_status_enum_t *status);
uint8_t assign_gps_position_velocity_type(const char *type_str, GPS_position_type_enum_t *type);

uint8_t parse_gps_header(const char* data_received, gps_response_header *result);
uint8_t parse_bestxyza_data(const char* data_received, gps_bestxyza_response *result);
uint8_t parse_timea_data(const char* data_received, gps_timea_response *result);

#endif // __INCLUDE_GUARD__GPS_H__
