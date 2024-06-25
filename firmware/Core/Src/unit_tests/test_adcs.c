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

// to run all unit tests: CTS1+run_all_unit_tests()!

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
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0xff, 0x55, 0x66};
    ADCS_Wheel_Speed_Struct result;
    ADCS_Pack_to_Wheel_Speed(input_params, &result);
    TEST_ASSERT_TRUE(result.x == 8721);
    TEST_ASSERT_TRUE(result.y == -205);
    TEST_ASSERT_TRUE(result.z == 26197);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Magnetorquer_Command()
{
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0xff, 0x55, 0x66};

    ADCS_Magnetorquer_Command_Struct result;
    ADCS_Pack_to_Magnetorquer_Command(input_params, &result);
    TEST_ASSERT_TRUE(compare_doubles(result.x, 87.21, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.y, -2.05, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.z, 261.97, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Raw_Magnetometer_Values()
{
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0xff};
    ADCS_Raw_Mag_TLM_Struct result;
    ADCS_Pack_to_Raw_Magnetometer_Values(input_params, &result);
    TEST_ASSERT_TRUE(result.x == 8721);
    TEST_ASSERT_TRUE(result.y == 17459);
    TEST_ASSERT_TRUE(result.z == -171);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Fine_Angular_Rates()
{
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0xff};

    ADCS_Fine_Angular_Rates_Struct result;
    ADCS_Pack_to_Fine_Angular_Rates(input_params, &result);
    TEST_ASSERT_TRUE(compare_doubles(result.x, 8.721, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.y, 17.459, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.z, -0.171, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Magnetometer_Config()
{
    uint8_t input_params[30] = {
        0x22, 0x22, 0xDE, 0xDD, 0x22, 0x22, 0xDE, 0xDD,
        0x22, 0x22, 0xDE, 0xDD, 0x22, 0x22, 0xDE, 0xDD,
        0x22, 0x22, 0xDE, 0xDD, 0x22, 0x22, 0xDE, 0xDD,
        0x22, 0x22, 0xDE, 0xDD, 0x22, 0x22};

    ADCS_Magnetometer_Config_Struct result;
    ADCS_Pack_to_Magnetometer_Config(input_params, &result);
    TEST_ASSERT_TRUE(compare_doubles(result.mounting_transform_alpha_angle, 87.38, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.mounting_transform_beta_angle, -87.38, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.mounting_transform_gamma_angle, 87.38, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.channel_1_offset, -8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.channel_2_offset, 8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.channel_3_offset, -8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.sensitivity_matrix_s11, 8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.sensitivity_matrix_s22, -8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.sensitivity_matrix_s33, 8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.sensitivity_matrix_s12, -8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.sensitivity_matrix_s13, 8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.sensitivity_matrix_s21, -8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.sensitivity_matrix_s23, 8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.sensitivity_matrix_s31, -8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.sensitivity_matrix_s32, 8.738, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Commanded_Attitude_Angles()
{
    uint8_t input_params[6] = {0x11, 0xaa, 0x22, 0xbb, 0x33, 0xcc};

    ADCS_Commanded_Angles_Struct result;
    ADCS_Pack_to_Commanded_Attitude_Angles(input_params, &result);
    TEST_ASSERT_TRUE(compare_doubles(result.x, -219.99, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.y, -176.3, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.z, -132.61, ADCS_TEST_EPSILON));

    return 0;
}

// TODO: pick up here with compare_doubles and testing negatives, then create return values
uint8_t TEST_EXEC__ADCS_Pack_to_Estimation_Params()
{
    uint8_t input_params[31] = {0xCD, 0xCC, 0x8C, 0x3F, 0xCD, 0xCC, 0x0C, 0x40, 0x33, 0x33, 0x53,
                                0x40, 0xCD, 0xCC, 0x8C, 0x40, 0x00, 0x00, 0xB0, 0x40, 0x33, 0x33,
                                0xD3, 0x40, 0x66, 0x66, 0xF6, 0x40, 0xAA, 0x0D, 0x2C};

    ADCS_Estimation_Params_Struct result;
    ADCS_Pack_to_Estimation_Params(input_params, &result);
    TEST_ASSERT_TRUE(compare_doubles(result.magnetometer_rate_filter_system_noise, 1.1, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.ekf_system_noise, 2.2, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.css_measurement_noise, 3.3, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.sun_sensor_measurement_noise, 4.4, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.nadir_sensor_measurement_noise, 5.5, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.magnetometer_measurement_noise, 6.6, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.star_tracker_measurement_noise, 7.7, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(result.use_sun_sensor == false);
    TEST_ASSERT_TRUE(result.use_nadir_sensor == true);
    TEST_ASSERT_TRUE(result.use_css == false);
    TEST_ASSERT_TRUE(result.use_star_tracker == true);
    TEST_ASSERT_TRUE(result.nadir_sensor_terminator_test == false);
    TEST_ASSERT_TRUE(result.automatic_magnetometer_recovery == true);
    TEST_ASSERT_TRUE(result.magnetometer_mode == ADCS_Magnetometer_Mode_Main_Motor);
    TEST_ASSERT_TRUE(result.magnetometer_selection_for_raw_mtm_tlm == ADCS_Magnetometer_Mode_Redundant_Signal);
    TEST_ASSERT_TRUE(result.automatic_estimation_transition_due_to_rate_sensor_errors == true);
    TEST_ASSERT_TRUE(result.wheel_30s_power_up_delay == true);
    TEST_ASSERT_TRUE(result.cam1_and_cam2_sampling_period == 44); // error counter reset period?

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_ASGP4_Params()
{
    uint8_t input_params[30] = {0x4C, 0x04, 0x98, 0x08, 0xE4, 0x0C, 0x30, 0x11,
                                0x7C, 0x15, 0xC8, 0x19, 0x4D, 0x01, 0x00, 0x3A,
                                0xC1, 0xFA, 0x40, 0x61, 0x19, 0xFA, 0x0A, 0x6F,
                                0xD4, 0x0D, 0xD6, 0xD7, 0x10, 0x00};

    ADCS_ASGP4_Params_Struct result;
    ADCS_Pack_to_ASGP4_Params(input_params, &result);
    TEST_ASSERT_TRUE(compare_doubles(result.incl_coefficient, 1.1, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.raan_coefficient, 2.2, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.ecc_coefficient, 3.3, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.aop_coefficient, 4.4, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.time_coefficient, 5.5, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.pos_coefficient, 6.6, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.maximum_position_error, 7.7, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(result.asgp4_filter == ADCS_ASGP4_Filter_Average);
    TEST_ASSERT_TRUE(compare_doubles(result.xp_coefficient, -8.8, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.yp_coefficient, -9.9, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(result.gps_roll_over == 10);
    TEST_ASSERT_TRUE(compare_doubles(result.position_sd, 11.1, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.velocity_sd, 2.12, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(result.min_satellites == 13);
    TEST_ASSERT_TRUE(compare_doubles(result.time_gain, 2.14, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.max_lag, 2.15, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(result.min_samples == 16);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Tracking_Controller_Target_Reference()
{
    uint8_t input_params[12] = {0xCD, 0xCC, 0xDC, 0x42, 0x33, 0x33, 0x8B, 0xC2, 0x66, 0x66, 0x86, 0x3F};

    ADCS_Tracking_Controller_Target_Struct result;
    ADCS_Pack_to_Tracking_Controller_Target_Reference(input_params, &result);
    TEST_ASSERT_TRUE(compare_doubles(result.lon, 110.4, 10*ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.lat, -69.6, 10*ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.alt, 1.05, ADCS_TEST_EPSILON));
    // per CubeSupport, latitude and longitude are only accurate to within 1e-5 degrees

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Rate_Gyro_Config()
{
    uint8_t input_params[10] = {0xCD, 0xCC, 0xDC, 0x42, 0x33, 0x33, 0x8B, 0xC2, 0x66, 0x66};

    ADCS_Rate_Gyro_Config_Struct result;
    ADCS_Pack_to_Rate_Gyro_Config(input_params, &result);
    TEST_ASSERT_TRUE(result.gyro1 == 205);
    TEST_ASSERT_TRUE(result.gyro2 == 204);
    TEST_ASSERT_TRUE(result.gyro3 == 220);
    TEST_ASSERT_TRUE(compare_doubles(result.x_rate_offset, 13.122, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.y_rate_offset, -29.901, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(compare_doubles(result.z_rate_offset, 26.306, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(result.rate_sensor_mult == 102);

    return 0;
}
