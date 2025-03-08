#include "unit_tests/test_eps_drivers.h"

#include "eps_drivers/eps_channel_control.h"
#include "eps_drivers/eps_types.h"

#include "unit_tests/unit_test_helpers.h"
#include <string.h>

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

uint8_t TEST_EXEC__EPS_channel_to_str(){
    // Success Cases:
    TEST_ASSERT(strcmp(EPS_channel_to_str(EPS_CHANNEL_VBATT_STACK),"VBATT_STACK")==0);
    TEST_ASSERT(strcmp(EPS_channel_to_str(EPS_CHANNEL_3V3_CAMERA),"3V3_CAMERA")==0);
    TEST_ASSERT(strcmp(EPS_channel_to_str(EPS_CHANNEL_UNKNOWN),"CHANNEL_UNKOWN")==0);

    // Failure Cases:
    TEST_ASSERT(strcmp(EPS_channel_to_str(137),"INVALID_CHANNEL")==0); // Out of bounds channel number
    TEST_ASSERT(strcmp(EPS_channel_to_str(69),"INVALID_CHANNEL")==0);

    return 0;
}

uint8_t TEST_EXEC__EPS_check_status_bit_of_channel(){
    // Success Cases:
    TEST_ASSERT(EPS_check_status_bit_of_channel(0x0000,0x0000,0) == 0); // All channels are disabled, checking ch 0
    TEST_ASSERT(EPS_check_status_bit_of_channel(0x0000,0x0001,16) == 1); // Checking ch16 (bit is on the 2nd bitfield)
    TEST_ASSERT(EPS_check_status_bit_of_channel(0x3080,0x0001,7) == 1); 
    TEST_ASSERT(EPS_check_status_bit_of_channel(0x31FB,0x0001,2) == 0); 

    // Fail Cases:
    TEST_ASSERT(EPS_check_status_bit_of_channel(0x31F3,0x0001,32) == 2); // Out of bounds channel check
    TEST_ASSERT(EPS_check_status_bit_of_channel(0x31F3,0x0001,60) == 2); 

    return 0;
}
