#ifndef __INCLUDE_GUARD__UNIT_TEST_GPS_H__
#define __INCLUDE_GUARD__UNIT_TEST_GPS_H__

#include <stdint.h>

uint8_t TEST_EXEC__GPS_assign_gps_time_status();
uint8_t TEST_EXEC__GPS_assign_gps_solution_status();
uint8_t TEST_EXEC__GPS_position_velocity_type();

uint8_t TEST_EXEC__GPS_Parse_header();
uint8_t TEST_EXEC__GPS_Parse_bestxyza();
uint8_t TEST_EXEC__GPS_Parse_timea();

#endif // __INCLUDE_GUARD__UNIT_TEST_GPS_H__