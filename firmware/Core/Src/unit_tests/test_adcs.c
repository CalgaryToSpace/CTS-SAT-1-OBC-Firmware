#include "adcs_drivers/adcs_struct_packers.h"
#include "unit_tests/unit_test_helpers.h"  // for all unit tests
#include "unit_tests/test_adcs.h"          // for ADCS tests
#include "transforms/number_comparisons.h" // for comparing doubles
#include <stdio.h>
#include <string.h>

// Reminder: to run all unit tests, CTS1+run_all_unit_tests()!

uint8_t TEST_EXEC__ADCS_Pack_to_Ack_Struct() {
    uint8_t input_params[4] = {0x11, 0x01, 0x03, 0x04};
    ADCS_CMD_Ack_Struct result;
    ADCS_Pack_to_Ack_Struct(input_params, &result);

    TEST_ASSERT_TRUE(result.last_id == 17);
    TEST_ASSERT_TRUE(result.processed == true);
    TEST_ASSERT_TRUE(result.error_flag == ADCS_ERROR_FLAG_INVALID_PARAMS);
    TEST_ASSERT_TRUE(result.error_index == 4);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Identification_Struct() {
    uint8_t input_params[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    ADCS_ID_Struct result;
    ADCS_Pack_to_Identification_Struct(input_params, &result);

    TEST_ASSERT_TRUE(result.node_type == 17);
    TEST_ASSERT_TRUE(result.interface_version == 34);
    TEST_ASSERT_TRUE(result.major_firmware_version == 51);
    TEST_ASSERT_TRUE(result.minor_firmware_version == 68);
    TEST_ASSERT_TRUE(result.seconds_since_startup == 26197);
    TEST_ASSERT_TRUE(result.ms_past_second == 34935);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Program_Status_Struct() {
    uint8_t input_params[6] = {0xF2, 0x22, 0x33, 0x02, 0x55, 0x66};
    ADCS_Boot_Running_Status_Struct result;
    ADCS_Pack_to_Program_Status_Struct(input_params, &result);

    TEST_ASSERT_TRUE(result.reset_cause == ADCS_RESET_CAUSE_UNKNOWN);
    TEST_ASSERT_TRUE(result.boot_cause == ADCS_BOOT_CAUSE_COMMUNICATIONS_TIMEOUT);
    TEST_ASSERT_TRUE(result.boot_counter == 13090);
    TEST_ASSERT_TRUE(result.boot_program_index == ADCS_RUNNING_PROGRAM_BOOTLOADER);
    TEST_ASSERT_TRUE(result.major_firmware_version == 85);
    TEST_ASSERT_TRUE(result.minor_firmware_version == 102);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Comms_Status_Struct() {
    uint8_t input_params[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
    ADCS_Comms_Status_Struct result;
    ADCS_Pack_to_Comms_Status_Struct(input_params, &result);

    TEST_ASSERT_TRUE(result.cmd_counter == 0x2211);
    TEST_ASSERT_TRUE(result.tlm_counter == 0x4433);
    TEST_ASSERT_TRUE(result.cmd_buffer_overrun == false);
    TEST_ASSERT_TRUE(result.i2c_tlm_error == true);
    TEST_ASSERT_TRUE(result.i2c_cmd_error == false);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Power_Control_Struct() {
    uint8_t input_params[3] = {0x11, 0x22, 0x31};
    ADCS_Power_Control_Struct result;
    ADCS_Pack_to_Power_Control_Struct(input_params, &result);

    TEST_ASSERT_TRUE(result.cube_control_motor == ADCS_POWER_SELECT_OFF);
    TEST_ASSERT_TRUE(result.cube_control_signal == ADCS_POWER_SELECT_ON);
    TEST_ASSERT_TRUE(result.cube_sense1 == ADCS_POWER_SELECT_ON);
    TEST_ASSERT_TRUE(result.cube_sense2 == ADCS_POWER_SELECT_OFF);
    TEST_ASSERT_TRUE(result.cube_star_power == ADCS_POWER_SELECT_SAME);
    TEST_ASSERT_TRUE(result.cube_wheel1_power == ADCS_POWER_SELECT_OFF);
    TEST_ASSERT_TRUE(result.cube_wheel2_power == ADCS_POWER_SELECT_SAME);
    TEST_ASSERT_TRUE(result.cube_wheel3_power == ADCS_POWER_SELECT_OFF);
    TEST_ASSERT_TRUE(result.gps_power == ADCS_POWER_SELECT_OFF);
    TEST_ASSERT_TRUE(result.motor_power == ADCS_POWER_SELECT_ON);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Angular_Rates_Struct() {
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0xff};
    ADCS_Angular_Rates_Struct result;
    ADCS_Pack_to_Angular_Rates_Struct(input_params, &result);

    TEST_ASSERT_TRUE(GEN_compare_doubles(result.x_rate, 87.21, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.y_rate, 174.59, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.z_rate, -1.71, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_LLH_Position_Struct() {
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0xff, 0x55, 0x66};
    ADCS_LLH_Position_Struct result;
    ADCS_Pack_to_LLH_Position_Struct(input_params, &result);

    TEST_ASSERT_TRUE(GEN_compare_doubles(result.latitude, 87.21, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.longitude, -2.05, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.altitude, 261.97, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Unix_Time_Save_Mode_Struct() {
    uint8_t input_params[2] = {0x11, 0x22};
    ADCS_Set_Unix_Time_Save_Mode_Struct result;
    ADCS_Pack_to_Unix_Time_Save_Mode_Struct(input_params, &result);

    TEST_ASSERT_TRUE(result.save_now == true);
    TEST_ASSERT_TRUE(result.save_on_update == false);
    TEST_ASSERT_TRUE(result.save_periodic == false);
    TEST_ASSERT_TRUE(result.period == 34);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Orbit_Params_Struct() {
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
    ADCS_Pack_to_Orbit_Params_Struct(input_params, &result);

    TEST_ASSERT_TRUE(GEN_compare_doubles(result.inclination, 1.2, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.eccentricity, 0.67, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.ascending_node_right_ascension, 5.6, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.b_star_drag_term, 0.9, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.mean_motion, 10.1, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.mean_anomaly, 11.2, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.epoch, 12.3, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Rated_Sensor_Rates_Struct() {
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0xff};
    ADCS_Rated_Sensor_Rates_Struct result;
    ADCS_Pack_to_Rated_Sensor_Rates_Struct(input_params, &result);

    TEST_ASSERT_TRUE(GEN_compare_doubles(result.x, 87.21, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.y, 174.59, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.z, -1.71, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Wheel_Speed_Struct() {
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0xff, 0x55, 0x66};
    ADCS_Wheel_Speed_Struct result;
    ADCS_Pack_to_Wheel_Speed_Struct(input_params, &result);
    TEST_ASSERT_TRUE(result.x == 8721);
    TEST_ASSERT_TRUE(result.y == -205);
    TEST_ASSERT_TRUE(result.z == 26197);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Magnetorquer_Command_Struct()
{
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0xff, 0x55, 0x66};

    ADCS_Magnetorquer_Command_Struct result;
    ADCS_Pack_to_Magnetorquer_Command_Struct(input_params, &result);
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.x, 87.21, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.y, -2.05, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.z, 261.97, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Raw_Magnetometer_Values_Struct()
{
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0xff};
    ADCS_Raw_Mag_TLM_Struct result;
    ADCS_Pack_to_Raw_Magnetometer_Values_Struct(input_params, &result);
    TEST_ASSERT_TRUE(result.x == 8721);
    TEST_ASSERT_TRUE(result.y == 17459);
    TEST_ASSERT_TRUE(result.z == -171);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Fine_Angular_Rates_Struct()
{
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0xff};

    ADCS_Fine_Angular_Rates_Struct result;
    ADCS_Pack_to_Fine_Angular_Rates_Struct(input_params, &result);
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.x, 8.721, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.y, 17.459, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.z, -0.171, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Magnetometer_Config_Struct()
{
    uint8_t input_params[30] = {
        0x22, 0x22, 0xDE, 0xDD, 0x22, 0x22, 0xDE, 0xDD,
        0x22, 0x22, 0xDE, 0xDD, 0x22, 0x22, 0xDE, 0xDD,
        0x22, 0x22, 0xDE, 0xDD, 0x22, 0x22, 0xDE, 0xDD,
        0x22, 0x22, 0xDE, 0xDD, 0x22, 0x22};

    ADCS_Magnetometer_Config_Struct result;
    ADCS_Pack_to_Magnetometer_Config_Struct(input_params, &result);
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.mounting_transform_alpha_angle, 87.38, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.mounting_transform_beta_angle, -87.38, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.mounting_transform_gamma_angle, 87.38, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.channel_1_offset, -8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.channel_2_offset, 8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.channel_3_offset, -8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.sensitivity_matrix_s11, 8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.sensitivity_matrix_s22, -8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.sensitivity_matrix_s33, 8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.sensitivity_matrix_s12, -8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.sensitivity_matrix_s13, 8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.sensitivity_matrix_s21, -8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.sensitivity_matrix_s23, 8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.sensitivity_matrix_s31, -8.738, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.sensitivity_matrix_s32, 8.738, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Commanded_Attitude_Angles_Struct()
{
    uint8_t input_params[6] = {0x11, 0xaa, 0x22, 0xbb, 0x33, 0xcc};

    ADCS_Commanded_Angles_Struct result;
    ADCS_Pack_to_Commanded_Attitude_Angles_Struct(input_params, &result);
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.x, -219.99, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.y, -176.3, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.z, -132.61, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Estimation_Params_Struct()
{
    uint8_t input_params[31] = {0xCD, 0xCC, 0x8C, 0x3F, 0xCD, 0xCC, 0x0C, 0x40, 0x33, 0x33, 0x53,
                                0x40, 0xCD, 0xCC, 0x8C, 0x40, 0x00, 0x00, 0xB0, 0x40, 0x33, 0x33,
                                0xD3, 0x40, 0x66, 0x66, 0xF6, 0x40, 0xAA, 0x0D, 0x2C};

    ADCS_Estimation_Params_Struct result;
    ADCS_Pack_to_Estimation_Params_Struct(input_params, &result);
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.magnetometer_rate_filter_system_noise, 1.1, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.ekf_system_noise, 2.2, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.css_measurement_noise, 3.3, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.sun_sensor_measurement_noise, 4.4, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.nadir_sensor_measurement_noise, 5.5, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.magnetometer_measurement_noise, 6.6, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.star_tracker_measurement_noise, 7.7, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(result.use_sun_sensor == false);
    TEST_ASSERT_TRUE(result.use_nadir_sensor == true);
    TEST_ASSERT_TRUE(result.use_css == false);
    TEST_ASSERT_TRUE(result.use_star_tracker == true);
    TEST_ASSERT_TRUE(result.nadir_sensor_terminator_test == false);
    TEST_ASSERT_TRUE(result.automatic_magnetometer_recovery == true);
    TEST_ASSERT_TRUE(result.magnetometer_mode == ADCS_MAGNETOMETER_MODE_MAIN_MOTOR);
    TEST_ASSERT_TRUE(result.magnetometer_selection_for_raw_mtm_tlm == ADCS_MAGNETOMETER_MODE_REDUNDANT_SIGNAL);
    TEST_ASSERT_TRUE(result.automatic_estimation_transition_due_to_rate_sensor_errors == true);
    TEST_ASSERT_TRUE(result.wheel_30s_power_up_delay == true);
    TEST_ASSERT_TRUE(result.cam1_and_cam2_sampling_period == 44); // error counter reset period?

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_ASGP4_Params_Struct()
{
    uint8_t input_params[30] = {0x4C, 0x04, 0x98, 0x08, 0xE4, 0x0C, 0x30, 0x11,
                                0x7C, 0x15, 0xC8, 0x19, 0x4D, 0x01, 0x00, 0x3A,
                                0xC1, 0xFA, 0x40, 0x61, 0x19, 0xFA, 0x0A, 0x6F,
                                0xD4, 0x0D, 0xD6, 0xD7, 0x10, 0x00};

    ADCS_ASGP4_Params_Struct result;
    ADCS_Pack_to_ASGP4_Params_Struct(input_params, &result);
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.incl_coefficient, 1.1, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.raan_coefficient, 2.2, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.ecc_coefficient, 3.3, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.aop_coefficient, 4.4, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.time_coefficient, 5.5, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.pos_coefficient, 6.6, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.maximum_position_error, 7.7, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(result.asgp4_filter == ADCS_ASGP4_FILTER_AVERAGE);
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.xp_coefficient, -8.8, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.yp_coefficient, -9.9, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(result.gps_roll_over == 10);
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.position_sd, 11.1, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.velocity_sd, 2.12, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(result.min_satellites == 13);
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.time_gain, 2.14, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.max_lag, 2.15, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(result.min_samples == 16);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Tracking_Controller_Target_Reference_Struct()
{
    uint8_t input_params[12] = {0xCD, 0xCC, 0xDC, 0x42, 0x33, 0x33, 0x8B, 0xC2, 0x66, 0x66, 0x86, 0x3F};

    ADCS_Tracking_Controller_Target_Struct result;
    ADCS_Pack_to_Tracking_Controller_Target_Reference_Struct(input_params, &result);
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.lon, 110.4, 10*ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.lat, -69.6, 10*ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.alt, 1.05, ADCS_TEST_EPSILON));
    // per CubeSupport, latitude and longitude are only accurate to within 1e-5 degrees

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Rate_Gyro_Config_Struct()
{
    uint8_t input_params[10] = {0xCD, 0xCC, 0xDC, 0x42, 0x33, 0x33, 0x8B, 0xC2, 0x66, 0x66};

    ADCS_Rate_Gyro_Config_Struct result;
    ADCS_Pack_to_Rate_Gyro_Config_Struct(input_params, &result);
    TEST_ASSERT_TRUE(result.gyro1 == 205);
    TEST_ASSERT_TRUE(result.gyro2 == 204);
    TEST_ASSERT_TRUE(result.gyro3 == 220);
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.x_rate_offset, 13.122, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.y_rate_offset, -29.901, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.z_rate_offset, 26.306, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(result.rate_sensor_mult == 102);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Estimated_Attitude_Angles_Struct()
{
    uint8_t input_params[6] = {0x10, 0x27, 0x34, 0xff, 0x56, 0x78}; 

    ADCS_Estimated_Attitude_Angles_Struct result;
    ADCS_Pack_to_Estimated_Attitude_Angles_Struct(input_params, &result);

    TEST_ASSERT_TRUE(GEN_compare_doubles(result.estimated_roll_angle, 100, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.estimated_pitch_angle, -2.04, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.estimated_yaw_angle, 308.06, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Magnetic_Field_Vector_Struct()
{
    uint8_t input_params[6] = {0x10, 0x27, 0x34, 0xff, 0x56, 0x78}; 

    ADCS_Magnetic_Field_Vector_Struct result;
    ADCS_Pack_to_Magnetic_Field_Vector_Struct(input_params, &result);

    TEST_ASSERT_TRUE(GEN_compare_doubles(result.x, 100, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.y, -2.04, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.z, 308.06, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Nadir_Vector_Struct()
{
    uint8_t input_params[6] = {0x01, 0x10, 0xf1, 0x11, 0xF0, 0xF1}; 

    ADCS_Nadir_Vector_Struct result;
    ADCS_Pack_to_Nadir_Vector_Struct(input_params, &result);

    TEST_ASSERT_TRUE(GEN_compare_doubles(result.x, 0.4097, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.y, 0.4593, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.z, -0.36, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Commanded_Wheel_Speed_Struct()
{
    uint8_t input_params[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0xd6}; 

    ADCS_Wheel_Speed_Struct result;
    ADCS_Pack_to_Commanded_Wheel_Speed_Struct(input_params, &result);

    TEST_ASSERT_TRUE(result.x == 513);
    TEST_ASSERT_TRUE(result.y == 1027);
    TEST_ASSERT_TRUE(result.z == -10747);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_IGRF_Magnetic_Field_Vector_Struct()
{
    uint8_t input_params[6] = {0x09, 0xf8, 0x07, 0x06, 0x05, 0xd4}; 

    ADCS_Magnetic_Field_Vector_Struct result;
    ADCS_Pack_to_IGRF_Magnetic_Field_Vector_Struct(input_params, &result);

    TEST_ASSERT_TRUE(GEN_compare_doubles(result.x, -20.39, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.y, 15.43, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.z, -112.59, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Quaternion_Error_Vector_Struct()
{
    uint8_t input_params[6] = {0x01, 0x02, 0x03, 0xe4, 0x05, 0x06}; 

    ADCS_Quaternion_Error_Vector_Struct result;
    ADCS_Pack_to_Quaternion_Error_Vector_Struct(input_params, &result);

    TEST_ASSERT_TRUE(GEN_compare_doubles(result.quaternion_error_q1, 0.0513, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.quaternion_error_q2, -0.7165, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.quaternion_error_q3, 0.1541, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Estimated_Gyro_Bias_Struct()
{
    uint8_t input_params[6] = {0x01, 0x02, 0x03, 0xe4, 0x05, 0x06}; 

    ADCS_Estimated_Gyro_Bias_Struct result;
    ADCS_Pack_to_Estimated_Gyro_Bias_Struct(input_params, &result);

    TEST_ASSERT_TRUE(GEN_compare_doubles(result.estimated_x_gyro_bias, 0.513, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.estimated_y_gyro_bias, -7.165, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.estimated_z_gyro_bias, 1.541, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Estimation_Innovation_Vector_Struct()
{
    uint8_t input_params[6] = {0x01, 0x02, 0x03, 0xe4, 0x05, 0x06}; 

    ADCS_Estimation_Innovation_Vector_Struct result;
    ADCS_Pack_to_Estimation_Innovation_Vector_Struct(input_params, &result);

    TEST_ASSERT_TRUE(GEN_compare_doubles(result.innovation_vector_x, 0.0513, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.innovation_vector_y, -0.7165, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.innovation_vector_z, 0.1541, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Raw_Cam1_Sensor_Struct()
{
    uint8_t input_params[6] = {0x01, 0x02, 0x03, 0xd4, 0x05, 0x06}; 

    ADCS_Raw_Cam_Sensor_Struct result;
    ADCS_Pack_to_Raw_Cam1_Sensor_Struct(input_params, &result);

    TEST_ASSERT_TRUE(result.which_sensor == ADCS_WHICH_CAM_SENSOR_CAM1);
    TEST_ASSERT_TRUE(result.cam_centroid_x == 513);
    TEST_ASSERT_TRUE(result.cam_centroid_y == -11261);
    TEST_ASSERT_TRUE(result.cam_capture_status == ADCS_CAPTURE_RESULT_SRAM_ERROR);
    TEST_ASSERT_TRUE(result.cam_detection_result == ADCS_DETECT_RESULT_SUN_NOT_FOUND);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Raw_Cam2_Sensor_Struct()
{
    uint8_t input_params[6] = {0x01, 0x02, 0x03, 0xd4, 0x05, 0x06}; 

    ADCS_Raw_Cam_Sensor_Struct result;
    ADCS_Pack_to_Raw_Cam1_Sensor_Struct(input_params, &result);

    TEST_ASSERT_TRUE(result.which_sensor == ADCS_WHICH_CAM_SENSOR_CAM1);
    TEST_ASSERT_TRUE(result.cam_centroid_x == 513);
    TEST_ASSERT_TRUE(result.cam_centroid_y == -11261);
    TEST_ASSERT_TRUE(result.cam_capture_status == ADCS_CAPTURE_RESULT_SRAM_ERROR);
    TEST_ASSERT_TRUE(result.cam_detection_result == ADCS_DETECT_RESULT_SUN_NOT_FOUND);

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Fine_Sun_Vector_Struct() {
    
    uint8_t input_params[6] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc}; 

    ADCS_Fine_Sun_Vector_Struct result;
    ADCS_Pack_to_Fine_Sun_Vector_Struct(input_params, &result);

    TEST_ASSERT_TRUE(GEN_compare_doubles(result.x, 1.333, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.y, 3.0806, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.z, -1.7254, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Raw_CSS_1_to_6_Struct() {
    uint8_t input_params[6] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};
    ADCS_Raw_CSS_1_to_6_Struct result;
    ADCS_Pack_to_Raw_CSS_1_to_6_Struct(input_params, &result);
    TEST_ASSERT_TRUE(result.css1 == 18);
    TEST_ASSERT_TRUE(result.css2 == 52);
    TEST_ASSERT_TRUE(result.css3 == 86);
    TEST_ASSERT_TRUE(result.css4 == 120);
    TEST_ASSERT_TRUE(result.css5 == 154);
    TEST_ASSERT_TRUE(result.css6 == 188);
    return 0;
}
uint8_t TEST_EXEC__ADCS_Pack_to_Raw_CSS_7_to_10_Struct() {
    uint8_t input_params[4] = {0x12, 0x34, 0x56, 0x78};
    ADCS_Raw_CSS_7_to_10_Struct result;
    ADCS_Pack_to_Raw_CSS_7_to_10_Struct(input_params, &result);
    TEST_ASSERT_TRUE(result.css7 == 18);
    TEST_ASSERT_TRUE(result.css8 == 52);
    TEST_ASSERT_TRUE(result.css9 == 86);
    TEST_ASSERT_TRUE(result.css10 == 120);
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_CubeControl_Current_Struct() {
    uint8_t input_params[6] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};

    ADCS_CubeControl_Current_Struct result;
    ADCS_Pack_to_CubeControl_Current_Struct(input_params, &result);

    TEST_ASSERT_TRUE(GEN_compare_doubles(result.cubecontrol_3v3_current, 6508.7890625, ADCS_TEST_EPSILON / 100.0));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.cubecontrol_5v_current, 15041.9921875, ADCS_TEST_EPSILON / 100.0));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.cubecontrol_vbat_current, 23575.1953125, ADCS_TEST_EPSILON / 100.0));
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Raw_GPS_Status_Struct() {
    uint8_t input_params[6] = {0x08, 0x34, 0x56, 0x78, 0x9a, 0xbc};
    ADCS_Raw_GPS_Status_Struct result;
    ADCS_Pack_to_Raw_GPS_Status_Struct(input_params, &result);
    TEST_ASSERT_TRUE(result.gps_solution_status == ADCS_GPS_SOLUTION_STATUS_LARGE_RESIDUALS);
    TEST_ASSERT_TRUE(result.num_tracked_satellites == 52);
    TEST_ASSERT_TRUE(result.num_used_satellites == 86);
    TEST_ASSERT_TRUE(result.counter_xyz_log == 120);
    TEST_ASSERT_TRUE(result.counter_range_log == 154);
    TEST_ASSERT_TRUE(result.response_message_gps_log == 188);
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Raw_GPS_Time_Struct() {
    uint8_t input_params[6] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};
    ADCS_Raw_GPS_Time_Struct result;
    ADCS_Pack_to_Raw_GPS_Time_Struct(input_params, &result);
    TEST_ASSERT_TRUE(result.gps_reference_week == 13330);
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.gps_time, 3164239.958, ADCS_TEST_EPSILON));
    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Raw_GPS_Struct() {
    uint8_t input_params[6] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};
    ADCS_Raw_GPS_Struct result;
    ADCS_Pack_to_Raw_GPS_Struct(ADCS_GPS_AXIS_Y, input_params, &result);
    TEST_ASSERT_TRUE(result.axis == ADCS_GPS_AXIS_Y);
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.ecef_position, 2018915346, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.ecef_velocity, -17254, ADCS_TEST_EPSILON));

    uint8_t input_params_two[6] = {0x12, 0x34, 0xee, 0xff, 0x9a, 0x0c};
    ADCS_Pack_to_Raw_GPS_Struct(ADCS_GPS_AXIS_X, input_params_two, &result);
    TEST_ASSERT_TRUE(result.axis == ADCS_GPS_AXIS_X);
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.ecef_position, -1166318, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.ecef_velocity, 3226, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_Pack_to_Measurements_Struct() {
    
    uint8_t input_params[72] = {0xd2, 0x1c, 0xa3, 0xc5, 0x3e, 0x93, 0x49, 0xf8, 
                                0x65, 0xef, 0x1f, 0xd3, 0xd1, 0xdb, 0xce, 0x16, 
                                0x27, 0xc5, 0xb1, 0xe5, 0xd3, 0x19, 0x70, 0xd9, 
                                0x87, 0x6f, 0xa1, 0x09, 0xf0, 0xb7, 0x99, 0xcc, 
                                0xb9, 0x5f, 0x6b, 0xe1, 0xd3, 0x05, 0xc9, 0x19, 
                                0x9c, 0xd7, 0x02, 0x2a, 0x54, 0xc2, 0x4f, 0xca, 
                                0x7d, 0xa2, 0x0d, 0x03, 0x48, 0x98, 0xe6, 0xb7, 
                                0xac, 0x8d, 0x3d, 0x63, 0x0b, 0x2c, 0x84, 0x7f, 
                                0x32, 0x15, 0x47, 0x9e, 0x3c, 0x4a, 0xd3, 0x1c};
    
    ADCS_Measurements_Struct result;
    ADCS_Pack_to_Measurements_Struct(input_params, &result);

    TEST_ASSERT_TRUE(GEN_compare_doubles(result.magnetic_field_x, 73.78, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.magnetic_field_y, -149.41, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.magnetic_field_z, -278.42, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.coarse_sun_x, -0.1975, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.coarse_sun_y, -0.4251, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.coarse_sun_z, -1.1489, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.sun_x, -0.9263, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.sun_y, 0.5838, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.sun_z, -1.5065, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.nadir_x, -0.6735, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.nadir_y, 0.6611, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.nadir_z, -0.9872, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.x_angular_rate, 285.51, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.y_angular_rate, 24.65, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.z_angular_rate, -184.48, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.x_wheel_speed, -13159, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.y_wheel_speed, 24505, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.z_wheel_speed, -7829, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.star1_body_x, 0.1491, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.star1_body_y, 0.6601, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.star1_body_z, -1.034, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.star1_orbit_x, 1.0754, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.star1_orbit_y, -1.5788, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.star1_orbit_z, -1.3745, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.star2_body_x, -2.3939, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.star2_body_y, 0.0781, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.star2_body_z, -2.6552, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.star2_orbit_x, -1.8458, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.star2_orbit_y, -2.9268, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.star2_orbit_z, 2.5405, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.star3_body_x, 1.1275, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.star3_body_y, 3.2644, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.star3_body_z, 0.5426, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.star3_orbit_x, -2.5017, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.star3_orbit_y, 1.9004, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.star3_orbit_z, 0.7379, ADCS_TEST_EPSILON));

    return 0;
}
