#ifndef __INCLUDE_GUARD__UNIT_TEST_GPS_H__
#define __INCLUDE_GUARD__UNIT_TEST_GPS_H__

#include <stdint.h>

uint8_t TEST_EXEC__GPS_reference_time_status_str_to_enum();
uint8_t TEST_EXEC__GPS_solution_status_str_to_enum();
uint8_t TEST_EXEC__GPS_position_type_str_to_enum();

uint8_t TEST_EXEC__GPS_header_response_parser();
uint8_t TEST_EXEC__GPS_bestxyza_data_parser();
uint8_t TEST_EXEC__GPS_timea_data_parser();

uint8_t TEST_EXEC__GPS_check_starts_with_log_prefix();

#endif // __INCLUDE_GUARD__UNIT_TEST_GPS_H__