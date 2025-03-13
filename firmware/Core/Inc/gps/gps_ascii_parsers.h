#ifndef __INCLUDE_GUARD__GPS_ASCII_PARSERS_H__
#define __INCLUDE_GUARD__GPS_ASCII_PARSERS_H__

#include "gps/gps_types.h"

uint8_t GPS_header_response_parser(const char* data_received, GPS_header_response_t *result);
uint8_t GPS_bestxyza_data_parser(const char* data_received, GPS_bestxyza_response_t *result);
uint8_t GPS_timea_data_parser(const char* data_received, GPS_timea_response_t *result);

#endif // __INCLUDE_GUARD__GPS_ASCII_PARSERS_H__