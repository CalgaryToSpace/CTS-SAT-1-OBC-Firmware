#include "unit_tests/unit_test_helpers.h" // for all unit tests
#include "unit_tests/test_adcs.h" // for ADCS tests
#include <string.h>

/* TODO: ADCS tests
	uint8_t TEST_EXEC__ADCS_function_name() {
		type input_params = value;
		TEST_ASSERT_TRUE(ADCS_function_name(params) == expected_return_value);
	}
*/

// memcmp(&one, &two, byte_length) gives 0 iff both mems are equal

uint8_t TEST_EXEC__ADCS_Pack_to_Ack() {
    uint8_t input_params[4] = {0x11, 0x01, 0x03, 0x04};
    
    ADCS_TC_Ack_Struct expected;
    expected.last_id = 17;
    expected.processed = true;
    expected.error_flag = TC_Error_Invalid_Params;
    expected.error_index = 4;

    ADCS_TC_Ack_Struct result = ADCS_Pack_to_Ack(input_params);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, 4) == 0);
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Identification() {
    uint8_t input_params[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    
    ADCS_ID_Struct expected;
    expected.node_type = 17;
	expected.interface_version = 34;
	expected.major_firmware_version = 51;
	expected.minor_firmware_version = 68;
	expected.seconds_since_startup = 26197;
	expected.ms_past_second = 34935;

    ADCS_ID_Struct result = ADCS_Pack_to_Identification(input_params);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, 8) == 0);
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Program_Status() {
    uint8_t input_params[6] = {0xF2, 0x22, 0x33, 0x02, 0x55, 0x66};
    
    ADCS_Boot_Running_Status_Struct expected;
    expected.reset_cause = ADCS_Unknown_Reset_Cause;
	expected.boot_cause = ADCS_Boot_Cause_Communications_Timeout;
	expected.boot_counter = 13090;
	expected.boot_program_index = ADCS_Running_Bootloader;
	expected.major_firmware_version = 85;
	expected.minor_firmware_version = 102; 

    ADCS_Boot_Running_Status_Struct result = ADCS_Pack_to_Program_Status(input_params);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, 6) == 0);
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Comms_Status() {
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55};
    // 0x55 = 0b01010101

    ADCS_Comms_Status_Struct expected;
    expected.tc_counter = 0x2211; 
	expected.tlm_counter = 0x4433;
	expected.tc_buffer_overrun = false; 
	expected.i2c_tlm_error = true;
	expected.i2c_tc_error = false; 

    ADCS_Comms_Status_Struct result = ADCS_Pack_to_Comms_Status(input_params);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, 6) == 0);
    return 0;
}