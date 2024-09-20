#include "unit_tests/test_eps_drivers.h"

#include "eps_drivers/eps_channel_control.h"
#include "eps_drivers/eps_types.h"

#include "unit_tests/unit_test_helpers.h"

uint8_t TEST_EXEC__EPS_channel_from_str() {
    // Success cases: Numbers.
    TEST_ASSERT(EPS_channel_from_str("0") == EPS_CHANNEL_VBATT_STACK);
    TEST_ASSERT(EPS_channel_from_str("1") == EPS_CHANNEL_5V_STACK);
    TEST_ASSERT(EPS_channel_from_str("15") == EPS_CHANNEL_3V3_CH15_UNUSED);
    TEST_ASSERT(EPS_channel_from_str("16") == EPS_CHANNEL_28V6_CH16_UNUSED);

    // Success cases: Names.
    TEST_ASSERT(EPS_channel_from_str("vbatt_stack") == EPS_CHANNEL_VBATT_STACK);

    // Failure Cases:
    TEST_ASSERT(EPS_channel_from_str("17") == EPS_CHANNEL_UNKNOWN);
    TEST_ASSERT(EPS_channel_from_str("16 ") == EPS_CHANNEL_UNKNOWN); // Trailing space.
    TEST_ASSERT(EPS_channel_from_str(" 16") == EPS_CHANNEL_UNKNOWN); // Leading space.
    TEST_ASSERT(EPS_channel_from_str(" 16 ") == EPS_CHANNEL_UNKNOWN); // Leading and trailing space.
    TEST_ASSERT(EPS_channel_from_str("a") == EPS_CHANNEL_UNKNOWN);
    TEST_ASSERT(EPS_channel_from_str("") == EPS_CHANNEL_UNKNOWN); // Empty string
    TEST_ASSERT(EPS_channel_from_str(" ") == EPS_CHANNEL_UNKNOWN); // Whitespace string
    TEST_ASSERT(EPS_channel_from_str(".") == EPS_CHANNEL_UNKNOWN); // Whitespace string
    
    return 0;
}
