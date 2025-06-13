#include "gnss_receiver/gnss_internal_drivers.h"
#include "gnss_receiver/gnss_time.h"
#include "timekeeping/timekeeping.h"
#include "log/log.h"
#include "unit_tests/unit_test_helpers.h"

#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

/// Expected error code for invalid messages
const uint8_t error_response = 1;

/// @brief Test: Valid TIMEA message, case 1
uint8_t TEST_EXEC__GNSS_timea_valid_case_1() {
    char test_str[] = "<OK[COM1]#TIMEA,COM1,0,80.0,FINESTEERING,2367,411646.000,02040000,9924,17402;VALID,-4.474795457e-08,9.384349250e-09,-18.00000000000,2025,5,22,21,0,28526,VALID*a2ff113a";
    const uint64_t expected_ms = 1747947628526;

    uint64_t result = 0;
    uint8_t status = GNSS_parse_timea_response_and_convert_to_unix_time_ms(test_str, &result);

    TEST_ASSERT_TRUE(status == 0);
    TEST_ASSERT_TRUE(result == expected_ms);

    return 0;
}

/// @brief Test: Valid TIMEA message, case 2
uint8_t TEST_EXEC__GNSS_timea_valid_case_2() {
    char test_str[] = "<OK[COM1]#TIMEA,COM1,0,80.0,FINESTEERING,2367,411646.000,02040000,9924,17402;VALID,-4.474795457e-08,9.384349250e-09,-18.00000000000,2027,10,10,11,9,19000,VALID*a2ff113a";
    const uint64_t expected_ms = 1823166559000;

    uint64_t result = 0;
    uint8_t status = GNSS_parse_timea_response_and_convert_to_unix_time_ms(test_str, &result);

    TEST_ASSERT_TRUE(status == 0);
    TEST_ASSERT_TRUE(result == expected_ms);

    return 0;
}

/// @brief Test: Invalid UTC status at end of TIMEA message
uint8_t TEST_EXEC__GNSS_timea_invalid_utc_status() {
    char test_str[] = "<OK[COM1]#TIMEA,COM1,0,80.0,FINESTEERING,2367,411646.000,02040000,9924,17402;VALID,-4.474795457e-08,9.384349250e-09,-18.00000000000,2025,5,22,21,0,28526,INVALID*a2ff113a";

    uint64_t result = 0;
    uint8_t status = GNSS_parse_timea_response_and_convert_to_unix_time_ms(test_str, &result);

    TEST_ASSERT_TRUE(status == error_response);

    return 0;
}

/// @brief Test: Invalid UTC status early in TIMEA message (should still parse as valid)
uint8_t TEST_EXEC__GNSS_timea_invalid_status_early() {
    char test_str[] = "<OK[COM1]#TIMEA,COM1,0,80.0,FINESTEERING,2367,411646.000,02040000,9924,17402;INVALID,-4.474795457e-08,9.384349250e-09,-18.00000000000,2025,5,22,21,0,28526,VALID*a2ff113a";
    const uint64_t expected_ms = 1747947628526;

    uint64_t result = 0;
    uint8_t status = GNSS_parse_timea_response_and_convert_to_unix_time_ms(test_str, &result);

    TEST_ASSERT_TRUE(status == 0);
    TEST_ASSERT_TRUE(result == expected_ms);

    return 0;
}

/// @brief Test: Malformed TIMEA message
uint8_t TEST_EXEC__GNSS_timea_malformed() {
    char test_str[] = "<OK[COM1]n/#TIMEA,COM1,0,80.0,HI,0,2367,411646.000,02040000,9924,17402;INVALID,-4.474795457e-08,9.384349250e-09,-18.00000000000,2025,5,22,21,0,28526,VALID*a2ff113a";

    uint64_t result = 0;
    uint8_t status = GNSS_parse_timea_response_and_convert_to_unix_time_ms(test_str, &result);

    TEST_ASSERT_TRUE(status == error_response);

    return 0;
}
