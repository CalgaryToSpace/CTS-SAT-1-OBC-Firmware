#include "unit_tests/unit_test_helpers.h" // for all unit tests
#include "unit_tests/test_adcs.h" // for ADCS tests

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
}