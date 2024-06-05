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
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_ID_Struct)) == 0);
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
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Boot_Running_Status_Struct)) == 0);
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
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Comms_Status_Struct)) == 0);
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Power_Control() {
    uint8_t input_params[3] = {0x11, 0x22, 0x31};   
    ADCS_Power_Control_Struct expected;

    expected.cube_control_motor = ADCS_Power_Select_Off;
    expected.cube_control_signal = ADCS_Power_Select_On;
    expected.cube_sense1 =  ADCS_Power_Select_On;
    expected.cube_sense2 = ADCS_Power_Select_Off;
    expected.cube_star_power = ADCS_Power_Select_Same;
    expected.cube_wheel1_power = ADCS_Power_Select_Off;
    expected.cube_wheel2_power = ADCS_Power_Select_Same;
    expected.cube_wheel3_power = ADCS_Power_Select_Off;
    expected.gps_power = ADCS_Power_Select_Off;
    expected.motor_power = ADCS_Power_Select_On;

    ADCS_Power_Control_Struct result = ADCS_Pack_to_Power_Control(input_params);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Power_Control_Struct)) == 0);
    
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Angular_Rates() {
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};   
    ADCS_Angular_Rates_Struct expected;
    
    // this is for the estimated angular rates
    expected.x_rate = 87.21;
    expected.y_rate = 174.59;
    expected.z_rate = 261.97;

    ADCS_Angular_Rates_Struct result = ADCS_Pack_to_Angular_Rates(input_params);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Angular_Rates_Struct)) == 0);
    
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_LLH_Position() {
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66}; 
    ADCS_LLH_Position_Struct expected;

    expected.latitude = 87.21;
    expected.longitude = 174.59;
    expected.altitude = 261.97;
    
    ADCS_LLH_Position_Struct result = ADCS_Pack_to_LLH_Position(input_params);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_LLH_Position_Struct)) == 0);
    
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Unix_Time_Save_Mode() {
    uint8_t input_params[2] = {0x11, 0x22}; 
    ADCS_Set_Unix_Time_Save_Mode_Struct expected;

    expected.save_now = true;
	expected.save_on_update = false;
	expected.save_periodic = false;

	expected.period = 34;
    
    ADCS_Set_Unix_Time_Save_Mode_Struct result = ADCS_Pack_to_Unix_Time_Save_Mode(input_params);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Set_Unix_Time_Save_Mode_Struct)) == 0);
    
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Orbit_Params() {
    uint8_t input_params[64] = {
        0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0xF3, 0x3F, 
        0x71, 0x3D, 0x0A, 0xD7, 0xA3, 0x70, 0xE5, 0x3F, 
        0x67, 0x66, 0x66, 0x66, 0x66, 0x66, 0x16, 0x40, 
        0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x1F, 0x40, 
        0xCD, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xEC, 0x3F, 
        0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x24, 0x40, 
        0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x26, 0x40, 
        0x9A, 0x99, 0x99, 0x99, 0x99, 0x99, 0x28, 0x40
    };
    ADCS_Orbit_Params_Struct expected;

    expected.inclination = 1.2;
    expected.eccentricity = 0.67;
    expected.ascending_node_right_ascension = 5.6;
    expected.b_star_drag_term = 0.9;
    expected.mean_motion = 10.1;
    expected.mean_anomaly = 11.2;
    expected.epoch = 12.3;
    
    ADCS_Orbit_Params_Struct result = ADCS_Pack_to_Orbit_Params(input_params);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Orbit_Params_Struct)) == 0);
    
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Rated_Sensor_Rates() {
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66}; 
    
    ADCS_Rated_Sensor_Rates_Struct expected;
    expected.x = 87.21;
    expected.y = 174.59;
    expected.z = 261.97;
       
    ADCS_Rated_Sensor_Rates_Struct result = ADCS_Pack_to_Rated_Sensor_Rates(input_params);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Rated_Sensor_Rates_Struct)) == 0);
    
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Wheel_Speed() {
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66}; 
    ADCS_Wheel_Speed_Struct expected;
    expected.x = 8721;
    expected.y = 17459;
    expected.z = 26197;
    
    ADCS_Wheel_Speed_Struct result = ADCS_Pack_to_Wheel_Speed(input_params);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Wheel_Speed_Struct)) == 0);
    
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Magnetorquer_Command_Time() {
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66}; 
    ADCS_Magnetorquer_Command_Struct expected;
    expected.x = 87.21;
    expected.y = 174.59;
    expected.z = 261.97;
    
    ADCS_Magnetorquer_Command_Struct result = ADCS_Pack_to_Magnetorquer_Command_Time(input_params);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Magnetorquer_Command_Struct)) == 0);
    
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Raw_Magnetometer_Values() {
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0xff}; 
    ADCS_Raw_Mag_TLM_Struct expected;
    expected.x = 8721;
    expected.y = 17459;
    expected.z = -171;
    
    ADCS_Raw_Mag_TLM_Struct result = ADCS_Pack_to_Raw_Magnetometer_Values(input_params);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Raw_Mag_TLM_Struct)) == 0);
    
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Fine_Angular_Rates() {
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0xff}; 
    ADCS_Fine_Angular_Rates_Struct expected;
    
    expected.x = 8.721;
    expected.y = 17.459;
    expected.z = -0.171;
    
    ADCS_Fine_Angular_Rates_Struct result = ADCS_Pack_to_Fine_Angular_Rates(input_params);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Fine_Angular_Rates_Struct)) == 0);
    
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Magnetometer_Config() {
    uint8_t input_params[30] = {
                                0x22, 0x22, 0xDE, 0xDD, 0x22, 0x22, 0xDE, 0xDD, 
                                0x22, 0x22, 0xDE, 0xDD, 0x22, 0x22, 0xDE, 0xDD, 
                                0x22, 0x22, 0xDE, 0xDD, 0x22, 0x22, 0xDE, 0xDD, 
                                0x22, 0x22, 0xDE, 0xDD, 0x22, 0x22}; 
    ADCS_Magnetometer_Config_Struct expected;

    expected.mounting_transform_alpha_angle = 87.38;
    expected.mounting_transform_beta_angle = -87.38;
    expected.mounting_transform_gamma_angle = 87.38;
    expected.channel_1_offset = -8.738;        
    expected.channel_2_offset = 8.738;    
    expected.channel_3_offset = -8.738;  
    expected.sensitivity_matrix_s11 = 8.738;
    expected.sensitivity_matrix_s22 = -8.738;
    expected.sensitivity_matrix_s33 = 8.738;
    expected.sensitivity_matrix_s12 = -8.738;
    expected.sensitivity_matrix_s13 = 8.738;
    expected.sensitivity_matrix_s21 = -8.738;
    expected.sensitivity_matrix_s23 = 8.738;
    expected.sensitivity_matrix_s31 = -8.738;    
    expected.sensitivity_matrix_s32 = 8.738;          
  
    ADCS_Magnetometer_Config_Struct result = ADCS_Pack_to_Magnetometer_Config(input_params);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Magnetometer_Config_Struct)) == 0);
    
    return 0;
}

// TODO: five TLM left (though maybe should do TC as well), then need return values for main functions

uint8_t TEST_EXEC__ADCS_Pack_to_Commanded_Attitude_Angles() {
    uint8_t input_params[6] = {0}; 
    ADCS_Commanded_Angles_Struct expected;

    // Define expected values for ADCS_Commanded_Angles_Struct
    
    ADCS_Commanded_Angles_Struct result = ADCS_Pack_to_Commanded_Attitude_Angles(input_params);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Commanded_Angles_Struct)) == 0);
    
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Estimation_Params() {
    uint8_t input_params[31] = {0}; 
    ADCS_Estimation_Params_Struct expected;

    // Define expected values for ADCS_Estimation_Params_Struct
    
    ADCS_Estimation_Params_Struct result = ADCS_Pack_to_Estimation_Params(input_params);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Estimation_Params_Struct)) == 0);
    
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_ASGP4_Params() {
    uint8_t input_params[30] = {0}; 
    ADCS_ASGP4_Params_Struct expected;

    // Define expected values for ADCS_ASGP4_Params_Struct
    
    ADCS_ASGP4_Params_Struct result = ADCS_Pack_to_ASGP4_Params(input_params);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_ASGP4_Params_Struct)) == 0);
    
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Tracking_Controller_Target_Reference() {
    uint8_t input_params[12] = {0}; 
    ADCS_Tracking_Controller_Target_Struct expected;

    // Define expected values for ADCS_Tracking_Controller_Target_Struct
    
    ADCS_Tracking_Controller_Target_Struct result = ADCS_Pack_to_Tracking_Controller_Target_Reference(input_params);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Tracking_Controller_Target_Struct)) == 0);
    
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Rate_Gyro_Config() {
    uint8_t input_params[12] = {0}; 
    ADCS_Rate_Gyro_Config_Struct expected;

    // Define expected values for ADCS_Rate_Gyro_Config_Struct
    
    ADCS_Rate_Gyro_Config_Struct result = ADCS_Pack_to_Rate_Gyro_Config(input_params);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Rate_Gyro_Config_Struct)) == 0);
    
    return 0;
}

