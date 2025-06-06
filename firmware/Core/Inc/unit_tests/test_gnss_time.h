#ifndef INCLUDE_GUARD__TEST_GNSS_TIME_H
#define INCLUDE_GUARD__TEST_GNSS_TIME_H

#include <stdint.h>

uint8_t TEST_EXEC__GNSS_timea_valid_case_1();
uint8_t TEST_EXEC__GNSS_timea_valid_case_2();
uint8_t TEST_EXEC__GNSS_timea_invalid_utc_status();
uint8_t TEST_EXEC__GNSS_timea_invalid_status_early();
uint8_t TEST_EXEC__GNSS_timea_malformed();

#endif
