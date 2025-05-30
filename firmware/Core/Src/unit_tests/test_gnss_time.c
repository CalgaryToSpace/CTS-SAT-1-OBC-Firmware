#define _GNU_SOURCE
#include "gnss_receiver/gnss_internal_drivers.h"
#include "gnss_receiver/gnss_time.h"
#include "timekeeping/timekeeping.h"
#include "log/log.h"
#include "unit_tests/unit_test_helpers.h"

#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief Unit test for GNSS_format_and_convert_to_unix_epoch
 */
uint8_t TEST_EXEC__GNSS_format_and_convert_to_unix_epoch() {
    // Example valid GNSS TIMEA message
    char test_str_valid[] = "<OK[COM1]#TIMEA,COM1,0,80.0,FINESTEERING,2367,411646.000,02040000,9924,17402;VALID,-4.474795457e-08,9.384349250e-09,-18.00000000000,2025,5,22,18,20,28526,VALID*a2ff113a";

    // Expected epoch milliseconds for 2025-05-22 18:20:28.526 UTC
    struct tm t = {0};
    t.tm_year = 2025 - 1900;
    t.tm_mon  = 5 - 1;
    t.tm_mday = 22;
    t.tm_hour = 18;
    t.tm_min  = 20;
    t.tm_sec  = 28;

    time_t expected_seconds = portable_timegm(&t);
    uint64_t expected_ms = (uint64_t)expected_seconds * 1000 + 526;

    // Test valid case
    uint64_t result = GNSS_format_and_convert_to_unix_epoch(test_str_valid);
    TEST_ASSERT_TRUE(result == expected_ms);

    return 0;  // Indicate test passed
}
