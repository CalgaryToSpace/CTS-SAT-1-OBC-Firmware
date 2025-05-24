#ifndef __INCLUDE_GUARD__UNIT_TEST_GNSS_H__
#define __INCLUDE_GUARD__UNIT_TEST_GNSS_H__

#include <stdint.h>

uint8_t TEST_EXEC__GNSS_reference_time_status_str_to_enum();
uint8_t TEST_EXEC__GNSS_solution_status_str_to_enum();
uint8_t TEST_EXEC__GNSS_position_type_str_to_enum();

uint8_t TEST_EXEC__GNSS_header_response_parser();
uint8_t TEST_EXEC__GNSS_bestxyza_data_parser();
uint8_t TEST_EXEC__GNSS_timea_data_parser();

#endif // __INCLUDE_GUARD__UNIT_TEST_GNSS_H__