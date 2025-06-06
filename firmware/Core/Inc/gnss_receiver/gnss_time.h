#ifndef INCLUDE_GUARD__GNSS_TIME_H__
#define INCLUDE_GUARD__GNSS_TIME_H__

#include <stdint.h>
#include <time.h>

uint64_t GNSS_parse_timea_response_and_convert_to_unix_time_ms(char* input_str, uint64_t* unix_time_ms);

uint8_t GNSS_set_obc_time_based_on_gnss_time();

#endif // INCLUDE_GUARD__GNSS_TIME_H__


