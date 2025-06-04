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
 * @brief Unit test for GNSS_parse_timea_response_and_convert_to_unix_time_ms
 */
uint8_t TEST_EXEC__GNSS_parse_timea_response_and_convert_to_unix_time_ms() {
    // Example valid GNSS TIMEA message
    char test_str_valid[] = "<OK[COM1]#TIMEA,COM1,0,80.0,FINESTEERING,2367,411646.000,02040000,9924,17402;VALID,-4.474795457e-08,9.384349250e-09,-18.00000000000,2025,5,22,18,20,28526,VALID*a2ff113a";

    const uint64_t expected_ms = 1747947628526;

    // Test valid case
    uint64_t result = GNSS_parse_timea_response_and_convert_to_unix_time_ms(test_str_valid);
    TEST_ASSERT_TRUE(result == expected_ms);

    return 0;  // Indicate test passed
}
