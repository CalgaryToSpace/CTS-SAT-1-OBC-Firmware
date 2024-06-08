#include "unit_tests/unit_test_helpers.h" // for all unit tests
#include "unit_tests/test_adcs.h"         // for ADCS tests
#include "transforms/number_comparisons.h" // for comparing doubles
#include <stdio.h>
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
    ADCS_TC_Ack_Struct result;
    ADCS_Pack_to_Ack(input_params, &result);

    TEST_ASSERT_TRUE(result.last_id == 17);
    TEST_ASSERT_TRUE(result.processed == true);
    TEST_ASSERT_TRUE(result.error_flag == TC_Error_Invalid_Params);
    TEST_ASSERT_TRUE(result.error_index == 4);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Identification() {
    uint8_t input_params[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    ADCS_ID_Struct result;
    ADCS_Pack_to_Identification(input_params, &result);

    TEST_ASSERT_TRUE(result.node_type == 17);
    TEST_ASSERT_TRUE(result.interface_version == 34);
    TEST_ASSERT_TRUE(result.major_firmware_version == 51);
    TEST_ASSERT_TRUE(result.minor_firmware_version == 68);
    TEST_ASSERT_TRUE(result.seconds_since_startup == 26197);
    TEST_ASSERT_TRUE(result.ms_past_second == 34935);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Program_Status() {
    uint8_t input_params[6] = {0xF2, 0x22, 0x33, 0x02, 0x55, 0x66};
    ADCS_Boot_Running_Status_Struct result;
    ADCS_Pack_to_Program_Status(input_params, &result);

    TEST_ASSERT_TRUE(result.reset_cause == ADCS_Unknown_Reset_Cause);
    TEST_ASSERT_TRUE(result.boot_cause == ADCS_Boot_Cause_Communications_Timeout);
    TEST_ASSERT_TRUE(result.boot_counter == 13090);
    TEST_ASSERT_TRUE(result.boot_program_index == ADCS_Running_Bootloader);
    TEST_ASSERT_TRUE(result.major_firmware_version == 85);
    TEST_ASSERT_TRUE(result.minor_firmware_version == 102);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Comms_Status() {
    uint8_t input_params[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
    ADCS_Comms_Status_Struct result;
    ADCS_Pack_to_Comms_Status(input_params, &result);

    TEST_ASSERT_TRUE(result.tc_counter == 0x2211);
    TEST_ASSERT_TRUE(result.tlm_counter == 0x4433);
    TEST_ASSERT_TRUE(result.tc_buffer_overrun == false);
    TEST_ASSERT_TRUE(result.i2c_tlm_error == true);
    TEST_ASSERT_TRUE(result.i2c_tc_error == false);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Power_Control() {
    uint8_t input_params[3] = {0x11, 0x22, 0x31};
    ADCS_Power_Control_Struct result;
    ADCS_Pack_to_Power_Control(input_params, &result);

    TEST_ASSERT_TRUE(result.cube_control_motor == ADCS_Power_Select_Off);
    TEST_ASSERT_TRUE(result.cube_control_signal == ADCS_Power_Select_On);
    TEST_ASSERT_TRUE(result.cube_sense1 == ADCS_Power_Select_On);
    TEST_ASSERT_TRUE(result.cube_sense2 == ADCS_Power_Select_Off);
    TEST_ASSERT_TRUE(result.cube_star_power == ADCS_Power_Select_Same);
    TEST_ASSERT_TRUE(result.cube_wheel1_power == ADCS_Power_Select_Off);
    TEST_ASSERT_TRUE(result.cube_wheel2_power == ADCS_Power_Select_Same);
    TEST_ASSERT_TRUE(result.cube_wheel3_power == ADCS_Power_Select_Off);
    TEST_ASSERT_TRUE(result.gps_power == ADCS_Power_Select_Off);
    TEST_ASSERT_TRUE(result.motor_power == ADCS_Power_Select_On);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Angular_Rates() {
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0xff};
    ADCS_Angular_Rates_Struct result;
    ADCS_Pack_to_Angular_Rates(input_params, &result);

    TEST_ASSERT_TRUE(compare_doubles(result.x_rate, 87.21, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.y_rate, 174.59, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.z_rate, -1.71, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_LLH_Position() {
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0xff, 0x55, 0x66};
    ADCS_LLH_Position_Struct result;
    ADCS_Pack_to_LLH_Position(input_params, &result);

    TEST_ASSERT_TRUE(compare_doubles(result.latitude, 87.21, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.longitude, -2.05, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.altitude, 261.97, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Unix_Time_Save_Mode() {
    uint8_t input_params[2] = {0x11, 0x22};
    ADCS_Set_Unix_Time_Save_Mode_Struct result;
    ADCS_Pack_to_Unix_Time_Save_Mode(input_params, &result);

    TEST_ASSERT_TRUE(result.save_now == true);
    TEST_ASSERT_TRUE(result.save_on_update == false);
    TEST_ASSERT_TRUE(result.save_periodic == false);
    TEST_ASSERT_TRUE(result.period == 34);

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
        0x9A, 0x99, 0x99, 0x99, 0x99, 0x99, 0x28, 0x40};
    ADCS_Orbit_Params_Struct result;
    ADCS_Pack_to_Orbit_Params(input_params, &result);

    TEST_ASSERT_TRUE(compare_doubles(result.inclination, 1.2, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.eccentricity, 0.67, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.ascending_node_right_ascension, 5.6, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.b_star_drag_term, 0.9, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.mean_motion, 10.1, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.mean_anomaly, 11.2, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.epoch, 12.3, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Rated_Sensor_Rates() {
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0xff};
    ADCS_Rated_Sensor_Rates_Struct result;
    ADCS_Pack_to_Rated_Sensor_Rates(input_params, &result);

    TEST_ASSERT_TRUE(compare_doubles(result.x, 87.21, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.y, 174.59, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.z, -1.71, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Wheel_Speed() {
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    ADCS_Wheel_Speed_Struct result;
    ADCS_Pack_to_Wheel_Speed(input_params, &result);
    // TODO: pick up here with compare_doubles and fixing negatives
    TEST_ASSERT_TRUE(result.x == 8721);
    TEST_ASSERT_TRUE(result.y == 17459);
    TEST_ASSERT_TRUE(result.z == 26197);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Magnetorquer_Command_Time()
{
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    ADCS_Magnetorquer_Command_Struct expected;
    expected.x = 87.21;
    expected.y = 174.59;
    expected.z = 261.97;

    ADCS_Magnetorquer_Command_Struct result;
    ADCS_Pack_to_Magnetorquer_Command_Time(input_params, &result);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Magnetorquer_Command_Struct)) == 0);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Raw_Magnetometer_Values()
{
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0xff};
    ADCS_Raw_Mag_TLM_Struct expected;
    expected.x = 8721;
    expected.y = 17459;
    expected.z = -171;

    ADCS_Raw_Mag_TLM_Struct result;
    ADCS_Pack_to_Raw_Magnetometer_Values(input_params, &result);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Raw_Mag_TLM_Struct)) == 0);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Fine_Angular_Rates()
{
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0xff};
    ADCS_Fine_Angular_Rates_Struct expected;

    expected.x = 8.721;
    expected.y = 17.459;
    expected.z = -0.171;

    ADCS_Fine_Angular_Rates_Struct result;
    ADCS_Pack_to_Fine_Angular_Rates(input_params, &result);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Fine_Angular_Rates_Struct)) == 0);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Magnetometer_Config()
{
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

    ADCS_Magnetometer_Config_Struct result;
    ADCS_Pack_to_Magnetometer_Config(input_params, &result);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Magnetometer_Config_Struct)) == 0);

    return 0;
}

// TODO: five TLM left (though maybe should do TC as well), then need return values for main functions

uint8_t TEST_EXEC__ADCS_Pack_to_Commanded_Attitude_Angles()
{
    uint8_t input_params[6] = {0x11, 0xaa, 0x22, 0xbb, 0x33, 0xcc};
    ADCS_Commanded_Angles_Struct expected;

    expected.x = -219.99;
    expected.y = -176.3;
    expected.z = -132.61;

    ADCS_Commanded_Angles_Struct result;
    ADCS_Pack_to_Commanded_Attitude_Angles(input_params, &result);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Commanded_Angles_Struct)) == 0);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Estimation_Params()
{
    uint8_t input_params[31] = {0xCD, 0xCC, 0x8C, 0x3F, 0xCD, 0xCC, 0x0C, 0x40, 0x33, 0x33, 0x53,
                                0x40, 0xCD, 0xCC, 0x8C, 0x40, 0x00, 0x00, 0xB0, 0x40, 0x33, 0x33,
                                0xD3, 0x40, 0x66, 0x66, 0xF6, 0x40, 0xAA, 0x0D, 0x2C};
    ADCS_Estimation_Params_Struct expected;

    expected.magnetometer_rate_filter_system_noise = 1.1;
    expected.ekf_system_noise = 2.2;
    expected.css_measurement_noise = 3.3;
    expected.sun_sensor_measurement_noise = 4.4;
    expected.nadir_sensor_measurement_noise = 5.5;
    expected.magnetometer_measurement_noise = 6.6;
    expected.star_tracker_measurement_noise = 7.7;
    expected.use_sun_sensor = false;
    expected.use_nadir_sensor = true;
    expected.use_css = false;
    expected.use_star_tracker = true;
    expected.nadir_sensor_terminator_test = false;
    expected.automatic_magnetometer_recovery = true;
    expected.magnetometer_mode = ADCS_Magnetometer_Mode_Main_Motor;
    expected.magnetometer_selection_for_raw_mtm_tlm = ADCS_Magnetometer_Mode_Redundant_Signal;
    expected.automatic_estimation_transition_due_to_rate_sensor_errors = true;
    expected.wheel_30s_power_up_delay = true;
    expected.cam1_and_cam2_sampling_period = 44; // error counter reset period?

    ADCS_Estimation_Params_Struct result;
    ADCS_Pack_to_Estimation_Params(input_params, &result);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Estimation_Params_Struct)) == 0);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_ASGP4_Params()
{
    uint8_t input_params[30] = {0x4C, 0x04, 0x98, 0x08, 0xE4, 0x0C, 0x30, 0x11,
                                0x7C, 0x15, 0xC8, 0x19, 0x4D, 0x01, 0x00, 0x3A,
                                0xC1, 0xFA, 0x40, 0x61, 0x19, 0xFA, 0x0A, 0x6F,
                                0xD4, 0x0D, 0xD6, 0xD7, 0x10, 0x00};
    ADCS_ASGP4_Params_Struct expected;

    expected.incl_coefficient = 1.1;
    expected.raan_coefficient = 2.2;
    expected.ecc_coefficient = 3.3;
    expected.aop_coefficient = 4.4;
    expected.time_coefficient = 5.5;
    expected.pos_coefficient = 6.6;
    expected.maximum_position_error = 7.7;
    expected.asgp4_filter = ADCS_ASGP4_Filter_Average;
    expected.xp_coefficient = -8.8;
    expected.yp_coefficient = -9.9;
    expected.gps_roll_over = 10;
    expected.position_sd = 11.1;
    expected.velocity_sd = 2.12;
    expected.min_satellites = 13;
    expected.time_gain = 2.14;
    expected.max_lag = 2.15;
    expected.min_samples = 16;

    ADCS_ASGP4_Params_Struct result;
    ADCS_Pack_to_ASGP4_Params(input_params, &result);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_ASGP4_Params_Struct)) == 0);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Tracking_Controller_Target_Reference()
{
    uint8_t input_params[12] = {0xCD, 0xCC, 0xDC, 0x42, 0x33, 0x33, 0x8B, 0xC2, 0x66, 0x66, 0x86, 0x3F};
    ADCS_Tracking_Controller_Target_Struct expected;

    expected.lon = 110.4;
    expected.lat = -69.6;
    expected.alt = 1.05;

    ADCS_Tracking_Controller_Target_Struct result;
    ADCS_Pack_to_Tracking_Controller_Target_Reference(input_params, &result);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Tracking_Controller_Target_Struct)) == 0);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Rate_Gyro_Config()
{
    uint8_t input_params[10] = {0xCD, 0xCC, 0xDC, 0x42, 0x33, 0x33, 0x8B, 0xC2, 0x66, 0x66};
    ADCS_Rate_Gyro_Config_Struct expected;

    expected.gyro1 = 205;
    expected.gyro2 = 204;
    expected.gyro3 = 220;
    expected.x_rate_offset = 13.122;
    expected.y_rate_offset = -29.901;
    expected.z_rate_offset = 26.306;
    expected.rate_sensor_mult = 102;

    ADCS_Rate_Gyro_Config_Struct result;
    ADCS_Pack_to_Rate_Gyro_Config(input_params, &result);
    TEST_ASSERT_TRUE(memcmp(&result, &expected, sizeof(ADCS_Rate_Gyro_Config_Struct)) == 0);

    return 0;
}
