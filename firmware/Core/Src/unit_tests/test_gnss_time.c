#include "gnss_receiver/gnss_internal_drivers.h"
#include "gnss_receiver/gnss_time.h"
#include "timekeeping/timekeeping.h"
#include "log/log.h"
#include "unit_tests/unit_test_helpers.h"

#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

/// @brief Unit test for GNSS_parse_timea_response_and_convert_to_unix_time_ms
uint8_t TEST_EXEC__GNSS_parse_timea_response_and_convert_to_unix_time_ms() {
    // Example valid GNSS TIMEA message
    char test_str_valid_1[] = "<OK[COM1]#TIMEA,COM1,0,80.0,FINESTEERING,2367,411646.000,02040000,9924,17402;VALID,-4.474795457e-08,9.384349250e-09,-18.00000000000,2025,5,22,9,0,28526,VALID*a2ff113a";
    char test_str_valid_2[] = "<OK[COM1]#TIMEA,COM1,0,80.0,FINESTEERING,2367,411646.000,02040000,9924,17402;VALID,-4.474795457e-08,9.384349250e-09,-18.00000000000,2027,10,10,11,9,19000,VALID*a2ff113a";

    const uint64_t expected_ms_1 = 1747947628526;
    const uint64_t expected_ms_2 = 1823166559000;

    // Invalid/partially invalid GNSS TIMEA messages
    char test_str_invalid_1[] = "<OK[COM1]#TIMEA,COM1,0,80.0,FINESTEERING,2367,411646.000,02040000,9924,17402;VALID,-4.474795457e-08,9.384349250e-09,-18.00000000000,2025,5,22,9,0,28526,INVALID*a2ff113a";
    char test_str_invalid_2[] = "<OK[COM1]#TIMEA,COM1,0,80.0,FINESTEERING,2367,411646.000,02040000,9924,17402;INVALID,-4.474795457e-08,9.384349250e-09,-18.00000000000,2025,5,22,9,0,28526,VALID*a2ff113a";
    char test_str_invalid_3[] = "<OK[COM1]n/#TIMEA,COM1,0,80.0,HI,0,2367,411646.000,02040000,9924,17402;INVALID,-4.474795457e-08,9.384349250e-09,-18.00000000000,2025,5,22,9,0,28526,VALID*a2ff113a";

    // Expected error code
    const uint8_t error_response = 1;

    // Test valid case 1
    uint64_t result_1 = 0;
    uint8_t status_1 = GNSS_parse_timea_response_and_convert_to_unix_time_ms(test_str_valid_1, &result_1);
    TEST_ASSERT_TRUE(status_1 == 0);
    TEST_ASSERT_TRUE(result_1 == expected_ms_1);

    // Test valid case 2
    uint64_t result_2 = 0;
    uint8_t status_2 = GNSS_parse_timea_response_and_convert_to_unix_time_ms(test_str_valid_2, &result_2);
    TEST_ASSERT_TRUE(status_2 == 0);
    TEST_ASSERT_TRUE(result_2 == expected_ms_2);

    // Test invalid case 1
    uint64_t result_invalid_1 = 0;
    uint8_t status_invalid_1 = GNSS_parse_timea_response_and_convert_to_unix_time_ms(test_str_invalid_1, &result_invalid_1);
    TEST_ASSERT_TRUE(status_invalid_1 == error_response);

    // Test invalid case 2 (invalid UTC status early, should parse as valid)
    uint64_t result_invalid_2 = 0;
    uint8_t status_invalid_2 = GNSS_parse_timea_response_and_convert_to_unix_time_ms(test_str_invalid_2, &result_invalid_2);
    TEST_ASSERT_TRUE(status_invalid_2 == 0);
    TEST_ASSERT_TRUE(result_invalid_2 == expected_ms_1);

    // Test invalid case 3 (malformed message)
    uint64_t result_invalid_3 = 0;
    uint8_t status_invalid_3 = GNSS_parse_timea_response_and_convert_to_unix_time_ms(test_str_invalid_3, &result_invalid_3);
    TEST_ASSERT_TRUE(status_invalid_3 == error_response);

    return 0;  // Indicate test passed
}
