#ifndef INCLUDE_GUARD__GPS_TIME_H__
#define INCLUDE_GUARD__GPS_TIME_H__

#include <stdint.h>

uint64_t GPS_format_and_convert_to_unix_epoch(char* input_str);

uint8_t GPS_set_obc_time_based_on_gps_time();

#endif // INCLUDE_GUARD__GPS_TIME_H__


