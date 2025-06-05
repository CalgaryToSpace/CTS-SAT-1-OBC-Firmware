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
    char test_str_valid_1[] = "<OK[COM1]#TIMEA,COM1,0,80.0,FINESTEERING,2367,411646.000,02040000,9924,17402;VALID,-4.474795457e-08,9.384349250e-09,-18.00000000000,2025,5,22,18,20,28526,VALID*a2ff113a";
    char test_str_valid_2[] = "<OK[COM1]#TIMEA,COM1,0,80.0,FINESTEERING,2367,411646.000,02040000,9924,17402;VALID,-4.474795457e-08,9.384349250e-09,-18.00000000000,2027,10,10,05,09,19000,VALID*a2ff113a";

    const uint64_t expected_ms_1 = 1747947628526; // UTC Time: Thu May 22 2025 21:00:28
    const uint64_t expected_ms_2 = 1823166559000; // UTC Time: 

    // Example of invalid and partially invalid GNSS TIMEA message
    char test_str_invalid_1[] = "<OK[COM1]#TIMEA,COM1,0,80.0,FINESTEERING,2367,411646.000,02040000,9924,17402;VALID,-4.474795457e-08,9.384349250e-09,-18.00000000000,2025,5,22,18,20,28526,INVALID*a2ff113a";
    char test_str_invalid_2[] = "<OK[COM1]#TIMEA,COM1,0,80.0,FINESTEERING,2367,411646.000,02040000,9924,17402;INVALID,-4.474795457e-08,9.384349250e-09,-18.00000000000,2025,5,22,18,20,28526,VALID*a2ff113a";
    
    const uint8_t error_response = 1; // Error for invalid response 1 (and subsequent)
    // test_str_invalid_2 should have same response as expected_ms_1

    // Example of invalid structure of string for GNSS TIMEA message
    char test_str_invalid_3[] = "<OK[COM1]n/#TIMEA,COM1,0,80.0,HI,0,2367,411646.000,02040000,9924,17402;INVALID,-4.474795457e-08,9.384349250e-09,-18.00000000000,2025,5,22,18,20,28526,VALID*a2ff113a";
    // test_str_invalid_3 should have same response as error_response

    // Test valid cases
    uint64_t result_1 = GNSS_parse_timea_response_and_convert_to_unix_time_ms(test_str_valid_1);
    uint64_t result_2 = GNSS_parse_timea_response_and_convert_to_unix_time_ms(test_str_valid_2);
    TEST_ASSERT_TRUE(result_1 == expected_ms_1);
    TEST_ASSERT_TRUE(result_2 == expected_ms_2);

    // Test invalid cases
    uint64_t response_1 = GNSS_parse_timea_response_and_convert_to_unix_time_ms(test_str_invalid_1);
    uint64_t response_2 = GNSS_parse_timea_response_and_convert_to_unix_time_ms(test_str_invalid_2);
    uint64_t response_3 = GNSS_parse_timea_response_and_convert_to_unix_time_ms(test_str_invalid_3);
    TEST_ASSERT_TRUE(response_1 == error_response)
    TEST_ASSERT_TRUE(response_2 == expected_ms_1)
    TEST_ASSERT_TRUE(response_3 == error_response)

    return 0;  // Indicate test passed
}
