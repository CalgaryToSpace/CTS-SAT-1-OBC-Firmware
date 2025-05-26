#ifndef __INCLUDE_GUARD__GNSS_ASCII_PARSERS_H__
#define __INCLUDE_GUARD__GNSS_ASCII_PARSERS_H__

#include "gnss_receiver/gnss_types.h"

uint8_t GNSS_header_response_parser(const char* data_received, GNSS_header_response_t *result);
uint8_t GNSS_bestxyza_data_parser(const char* data_received, GNSS_bestxyza_response_t *result);
uint8_t GNSS_timea_data_parser(const char* data_received, GNSS_timea_response_t *result);

#endif // __INCLUDE_GUARD__GNSS_ASCII_PARSERS_H__