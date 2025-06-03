#include "adcs_drivers/adcs_struct_packers.h"
#include "adcs_drivers/adcs_internal_drivers.h"
#include "adcs_drivers/adcs_command_ids.h"
#include "unit_tests/unit_test_helpers.h"  // for all unit tests
#include "unit_tests/test_adcs.h"          // for ADCS tests
#include "transforms/number_comparisons.h" // for comparing doubles
#include <stdio.h>
#include <string.h>

// Reminder: to run all unit tests, CTS1+run_all_unit_tests()!

uint8_t TEST_EXEC__ADCS_pack_to_ack_struct() {
    uint8_t input_params[4] = {0x11, 0x01, 0x03, 0x04};
    ADCS_cmd_ack_struct_t result;
    ADCS_pack_to_ack_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.last_id == 17);
    TEST_ASSERT_TRUE(result.processed == true);
    TEST_ASSERT_TRUE(result.error_flag == ADCS_ERROR_FLAG_INVALID_PARAMS);
    TEST_ASSERT_TRUE(result.error_index == 4);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_identification_struct() {
    uint8_t input_params[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    ADCS_id_struct_t result;
    ADCS_pack_to_identification_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.node_type == 17);
    TEST_ASSERT_TRUE(result.interface_version == 34);
    TEST_ASSERT_TRUE(result.major_firmware_version == 51);
    TEST_ASSERT_TRUE(result.minor_firmware_version == 68);
    TEST_ASSERT_TRUE(result.seconds_since_startup == 26197);
    TEST_ASSERT_TRUE(result.ms_past_second == 34935);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_program_status_struct() {
    uint8_t input_params[6] = {0xF2, 0x22, 0x33, 0x02, 0x55, 0x66};
    ADCS_boot_running_status_struct_t result;
    ADCS_pack_to_program_status_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.reset_cause == ADCS_RESET_CAUSE_UNKNOWN);
    TEST_ASSERT_TRUE(result.boot_cause == ADCS_BOOT_CAUSE_COMMUNICATIONS_TIMEOUT);
    TEST_ASSERT_TRUE(result.boot_counter == 13090);
    TEST_ASSERT_TRUE(result.boot_program_index == ADCS_RUNNING_PROGRAM_BOOTLOADER);
    TEST_ASSERT_TRUE(result.major_firmware_version == 85);
    TEST_ASSERT_TRUE(result.minor_firmware_version == 102);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_comms_status_struct() {
    uint8_t input_params[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
    ADCS_comms_status_struct_t result;
    ADCS_pack_to_comms_status_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.cmd_counter == 8721);
    TEST_ASSERT_TRUE(result.tlm_counter == 17459);
    TEST_ASSERT_TRUE(result.cmd_buffer_overrun == false);
    TEST_ASSERT_TRUE(result.i2c_tlm_error == true);
    TEST_ASSERT_TRUE(result.i2c_cmd_error == false);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_power_control_struct() {
    uint8_t input_params[3] = {0x11, 0x22, 0x31};
    ADCS_power_control_struct_t result;
    ADCS_pack_to_power_control_struct(input_params, &result);

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

uint8_t TEST_EXEC__ADCS_pack_to_angular_rates_struct() {
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0xff};
    ADCS_angular_rates_struct_t result;
    ADCS_pack_to_angular_rates_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.x_rate_mdeg_per_sec == 87210);
    TEST_ASSERT_TRUE(result.y_rate_mdeg_per_sec == 174590);
    TEST_ASSERT_TRUE(result.z_rate_mdeg_per_sec == -1710);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_llh_position_struct() {
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0xff, 0x55, 0x66};
    ADCS_llh_position_struct_t result;
    ADCS_pack_to_llh_position_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.latitude_mdeg ==  87210);
    TEST_ASSERT_TRUE(result.longitude_mdeg == -2050);
    TEST_ASSERT_TRUE(result.altitude_meters == 261970);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_unix_time_save_mode_struct() {
    uint8_t input_params[2] = {0x11, 0x22};
    ADCS_set_unix_time_save_mode_struct_t result;
    ADCS_pack_to_unix_time_save_mode_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.save_now == true);
    TEST_ASSERT_TRUE(result.save_on_update == false);
    TEST_ASSERT_TRUE(result.save_periodic == false);
    TEST_ASSERT_TRUE(result.period_s == 34);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_orbit_params_struct() {
    uint8_t input_params[64] = {
        0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0xF3, 0x3F,
        0x71, 0x3D, 0x0A, 0xD7, 0xA3, 0x70, 0xE5, 0x3F,
        0x67, 0x66, 0x66, 0x66, 0x66, 0x66, 0x16, 0x40,
        0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x1F, 0x40,
        0xCD, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xEC, 0x3F,
        0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x24, 0x40,
        0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x26, 0x40,
        0x9A, 0x99, 0x99, 0x99, 0x99, 0x99, 0x28, 0x40};
    ADCS_orbit_params_struct_t result;
    ADCS_pack_to_orbit_params_struct(input_params, &result);

    TEST_ASSERT_TRUE(GEN_compare_doubles(result.inclination_deg, 1.2, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.eccentricity, 0.67, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.ascending_node_right_ascension_deg, 5.6, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.b_star_drag_term, 0.9, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.mean_motion_orbits_per_day, 10.1, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.mean_anomaly_deg, 11.2, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.epoch_year_point_day, 12.3, ADCS_TEST_EPSILON));

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_rated_sensor_rates_struct() {
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0xff};
    ADCS_rated_sensor_rates_struct_t result;
    ADCS_pack_to_rated_sensor_rates_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.x_mdeg_per_sec == 87210);
    TEST_ASSERT_TRUE(result.y_mdeg_per_sec == 174590);
    TEST_ASSERT_TRUE(result.z_mdeg_per_sec == -1710);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_wheel_speed_struct() {
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0xff, 0x55, 0x66};
    ADCS_wheel_speed_struct_t result;
    ADCS_pack_to_wheel_speed_struct(input_params, &result);
    TEST_ASSERT_TRUE(result.actual_wheel_speed == true);
    TEST_ASSERT_TRUE(result.x_rpm == 8721);
    TEST_ASSERT_TRUE(result.y_rpm == -205);
    TEST_ASSERT_TRUE(result.z_rpm == 26197);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_magnetorquer_command_struct()
{
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0xff, 0x55, 0x66};

    ADCS_magnetorquer_command_struct_t result;
    ADCS_pack_to_magnetorquer_command_struct(input_params, &result);
    TEST_ASSERT_TRUE(result.x_ms == 87210);
    TEST_ASSERT_TRUE(result.y_ms == -2050);
    TEST_ASSERT_TRUE(result.z_ms == 261970);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_raw_magnetometer_values_struct()
{
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0xff};
    ADCS_raw_magnetometer_values_struct_t result;
    ADCS_pack_to_raw_magnetometer_values_struct(input_params, &result);
    TEST_ASSERT_TRUE(result.x_raw == 8721);
    TEST_ASSERT_TRUE(result.y_raw == 17459);
    TEST_ASSERT_TRUE(result.z_raw == -171);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_fine_angular_rates_struct()
{
    uint8_t input_params[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0xff};

    ADCS_fine_angular_rates_struct_t result;
    ADCS_pack_to_fine_angular_rates_struct(input_params, &result);
    TEST_ASSERT_TRUE(result.x_mdeg_per_sec == 8721);
    TEST_ASSERT_TRUE(result.y_mdeg_per_sec == 17459);
    TEST_ASSERT_TRUE(result.z_mdeg_per_sec == -171);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_magnetometer_config_struct()
{
    uint8_t input_params[30] = {
        0x22, 0x22, 0xDE, 0xDD, 0x22, 0x22, 0xDE, 0xDD,
        0x22, 0x22, 0xDE, 0xDD, 0x22, 0x22, 0xDE, 0xDD,
        0x22, 0x22, 0xDE, 0xDD, 0x22, 0x22, 0xDE, 0xDD,
        0x22, 0x22, 0xDE, 0xDD, 0x22, 0x22};

    ADCS_magnetometer_config_struct_t result;
    ADCS_pack_to_magnetometer_config_struct(input_params, &result);
    TEST_ASSERT_TRUE(result.mounting_transform_alpha_angle_mdeg_per_sec == 87380);
    TEST_ASSERT_TRUE(result.mounting_transform_beta_angle_mdeg_per_sec == -87380);
    TEST_ASSERT_TRUE(result.mounting_transform_gamma_angle_mdeg_per_sec == 87380);
    TEST_ASSERT_TRUE(result.channel_1_offset_mdeg_per_sec == -8738);
    TEST_ASSERT_TRUE(result.channel_2_offset_mdeg_per_sec == 8738);
    TEST_ASSERT_TRUE(result.channel_3_offset_mdeg_per_sec == -8738);
    TEST_ASSERT_TRUE(result.sensitivity_matrix_s11_mdeg_per_sec == 8738);
    TEST_ASSERT_TRUE(result.sensitivity_matrix_s22_mdeg_per_sec == -8738);
    TEST_ASSERT_TRUE(result.sensitivity_matrix_s33_mdeg_per_sec == 8738);
    TEST_ASSERT_TRUE(result.sensitivity_matrix_s12_mdeg_per_sec == -8738);
    TEST_ASSERT_TRUE(result.sensitivity_matrix_s13_mdeg_per_sec == 8738);
    TEST_ASSERT_TRUE(result.sensitivity_matrix_s21_mdeg_per_sec == -8738);
    TEST_ASSERT_TRUE(result.sensitivity_matrix_s23_mdeg_per_sec == 8738);
    TEST_ASSERT_TRUE(result.sensitivity_matrix_s31_mdeg_per_sec == -8738);
    TEST_ASSERT_TRUE(result.sensitivity_matrix_s32_mdeg_per_sec == 8738);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_commanded_attitude_angles_struct()
{
    uint8_t input_params[6] = {0x11, 0xaa, 0x22, 0xbb, 0x33, 0xcc};

    ADCS_commanded_angles_struct_t result;
    ADCS_pack_to_commanded_attitude_angles_struct(input_params, &result);
    TEST_ASSERT_TRUE(result.x_mdeg == -219990);
    TEST_ASSERT_TRUE(result.y_mdeg == -176300);
    TEST_ASSERT_TRUE(result.z_mdeg == -132610);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_estimation_params_struct()
{
    uint8_t input_params[31] = {0xCD, 0xCC, 0x8C, 0x3F, 0xCD, 0xCC, 0x0C, 0x40, 0x33, 0x33, 0x53,
                                0x40, 0xCD, 0xCC, 0x8C, 0x40, 0x00, 0x00, 0xB0, 0x40, 0x33, 0x33,
                                0xD3, 0x40, 0x66, 0x66, 0xF6, 0x40, 0xAA, 0x0D, 0x2C};

    ADCS_estimation_params_struct_t result;
    ADCS_pack_to_estimation_params_struct(input_params, &result);
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.magnetometer_rate_filter_system_noise, 1.1, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.extended_kalman_filter_system_noise, 2.2, ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.coarse_sun_sensor_measurement_noise, 3.3, ADCS_TEST_EPSILON));
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
    TEST_ASSERT_TRUE(result.magnetometer_selection_for_raw_magnetometer_telemetry == ADCS_MAGNETOMETER_MODE_REDUNDANT_SIGNAL);
    TEST_ASSERT_TRUE(result.automatic_estimation_transition_due_to_rate_sensor_errors == true);
    TEST_ASSERT_TRUE(result.wheel_30s_power_up_delay == true);
    TEST_ASSERT_TRUE(result.error_counter_reset_period_min == 44);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_augmented_sgp4_params_struct()
{
    uint8_t input_params[30] = {0x4C, 0x04, 0x98, 0x08, 0xE4, 0x0C, 0x30, 0x11,
                                0x7C, 0x15, 0xC8, 0x19, 0x4D, 0x01, 0x00, 0x3A,
                                0xC1, 0xFA, 0x40, 0x61, 0x19, 0xFA, 0x0A, 0x6F,
                                0xD4, 0x0D, 0xD6, 0xD7, 0x10, 0x00};

    ADCS_augmented_sgp4_params_struct_t result;
    ADCS_pack_to_augmented_sgp4_params_struct(input_params, &result);
    TEST_ASSERT_TRUE(result.incl_coefficient_milli == 1100);
    TEST_ASSERT_TRUE(result.raan_coefficient_milli == 2200);
    TEST_ASSERT_TRUE(result.ecc_coefficient_milli == 3300);
    TEST_ASSERT_TRUE(result.aop_coefficient_milli == 4400);
    TEST_ASSERT_TRUE(result.time_coefficient_milli == 5500);
    TEST_ASSERT_TRUE(result.pos_coefficient_milli == 6600);
    TEST_ASSERT_TRUE(result.maximum_position_error_milli == 7700);
    TEST_ASSERT_TRUE(result.augmented_sgp4_filter == ADCS_Augmented_SGP4_FILTER_AVERAGE);
    TEST_ASSERT_TRUE(result.xp_coefficient_nano == -8800000000);
    TEST_ASSERT_TRUE(result.yp_coefficient_nano == -9900000000);
    TEST_ASSERT_TRUE(result.gps_roll_over == 10);
    TEST_ASSERT_TRUE(result.position_sd_milli == 11100);
    TEST_ASSERT_TRUE(result.velocity_sd_milli == 2120);
    TEST_ASSERT_TRUE(result.min_satellites == 13);
    TEST_ASSERT_TRUE(result.time_gain_milli == 2140);
    TEST_ASSERT_TRUE(result.max_lag_milli == 2150);
    TEST_ASSERT_TRUE(result.min_samples == 16);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_tracking_controller_target_reference_struct()
{
    uint8_t input_params[12] = {0xCD, 0xCC, 0xDC, 0x42, 0x33, 0x33, 0x8B, 0xC2, 0x66, 0x66, 0x86, 0x3F};

    ADCS_tracking_controller_target_struct_t result;
    ADCS_pack_to_tracking_controller_target_reference_struct(input_params, &result);
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.longitude_degrees, 110.4, 10*ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.latitude_degrees, -69.6, 10*ADCS_TEST_EPSILON));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.altitude_meters, 1.05, ADCS_TEST_EPSILON));
    // per CubeSupport, latitude and longitude are only accurate to within 1e-5 degrees

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_rate_gyro_config_struct()
{
    uint8_t input_params[10] = {0xCD, 0xCC, 0xDC, 0x42, 0x33, 0x33, 0x8B, 0xC2, 0x66, 0x66};

    ADCS_rate_gyro_config_struct_t result;
    ADCS_pack_to_rate_gyro_config_struct(input_params, &result);
    TEST_ASSERT_TRUE(result.gyro1 == 205);
    TEST_ASSERT_TRUE(result.gyro2 == 204);
    TEST_ASSERT_TRUE(result.gyro3 == 220);
    TEST_ASSERT_TRUE(result.x_rate_offset_mdeg_per_sec == 13122);
    TEST_ASSERT_TRUE(result.y_rate_offset_mdeg_per_sec == -29901);
    TEST_ASSERT_TRUE(result.z_rate_offset_mdeg_per_sec == 26306);
    TEST_ASSERT_TRUE(result.rate_sensor_mult == 102);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_estimated_attitude_angles_struct()
{
    uint8_t input_params[6] = {0x10, 0x27, 0x34, 0xff, 0x56, 0x78}; 

    ADCS_estimated_attitude_angles_struct_t result;
    ADCS_pack_to_estimated_attitude_angles_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.estimated_roll_angle_mdeg == 100000);
    TEST_ASSERT_TRUE(result.estimated_pitch_angle_mdeg == -2040);
    TEST_ASSERT_TRUE(result.estimated_yaw_angle_mdeg == 308060);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_magnetic_field_vector_struct()
{
    uint8_t input_params[6] = {0x10, 0x27, 0x34, 0xff, 0x56, 0x78}; 

    ADCS_magnetic_field_vector_struct_t result;
    ADCS_pack_to_magnetic_field_vector_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.x_nT == 100000);
    TEST_ASSERT_TRUE(result.y_nT == -2040);
    TEST_ASSERT_TRUE(result.z_nT == 308060);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_nadir_vector_struct()
{
    uint8_t input_params[6] = {0x01, 0x10, 0xf1, 0x11, 0xF0, 0xF1}; 

    ADCS_nadir_vector_struct_t result;
    ADCS_pack_to_nadir_vector_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.x_micro == 409700);
    TEST_ASSERT_TRUE(result.y_micro == 459300);
    TEST_ASSERT_TRUE(result.z_micro == -360000);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_commanded_wheel_speed_struct()
{
    uint8_t input_params[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0xd6}; 

    ADCS_wheel_speed_struct_t result;
    ADCS_pack_to_commanded_wheel_speed_struct(input_params, &result);
    TEST_ASSERT_TRUE(result.actual_wheel_speed == false);
    TEST_ASSERT_TRUE(result.x_rpm == 513);
    TEST_ASSERT_TRUE(result.y_rpm == 1027);
    TEST_ASSERT_TRUE(result.z_rpm == -10747);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_igrf_magnetic_field_vector_struct()
{
    uint8_t input_params[6] = {0x09, 0xf8, 0x07, 0x06, 0x05, 0xd4}; 

    ADCS_magnetic_field_vector_struct_t result;
    ADCS_pack_to_igrf_magnetic_field_vector_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.x_nT == -20390);
    TEST_ASSERT_TRUE(result.y_nT == 15430);
    TEST_ASSERT_TRUE(result.z_nT == -112590);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_quaternion_error_vector_struct()
{
    uint8_t input_params[6] = {0x01, 0x02, 0x03, 0xe4, 0x05, 0x06}; 

    ADCS_quaternion_error_vector_struct_t result;
    ADCS_pack_to_quaternion_error_vector_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.quaternion_error_q1_micro == 51300);
    TEST_ASSERT_TRUE(result.quaternion_error_q2_micro == -716500);
    TEST_ASSERT_TRUE(result.quaternion_error_q3_micro == 154100);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_estimated_gyro_bias_struct()
{
    uint8_t input_params[6] = {0x01, 0x02, 0x03, 0xe4, 0x05, 0x06}; 

    ADCS_estimated_gyro_bias_struct_t result;
    ADCS_pack_to_estimated_gyro_bias_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.estimated_x_gyro_bias_mdeg_per_sec == 513);
    TEST_ASSERT_TRUE(result.estimated_y_gyro_bias_mdeg_per_sec == -7165);
    TEST_ASSERT_TRUE(result.estimated_z_gyro_bias_mdeg_per_sec == 1541);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_estimation_innovation_vector_struct()
{
    uint8_t input_params[6] = {0x01, 0x02, 0x03, 0xe4, 0x05, 0x06}; 

    ADCS_estimation_innovation_vector_struct_t result;
    ADCS_pack_to_estimation_innovation_vector_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.innovation_vector_x_micro ==  51300);
    TEST_ASSERT_TRUE(result.innovation_vector_y_micro == -716500);
    TEST_ASSERT_TRUE(result.innovation_vector_z_micro ==  154100);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_raw_cam1_sensor_struct()
{
    uint8_t input_params[6] = {0x01, 0x02, 0x03, 0xd4, 0x05, 0x06}; 

    ADCS_raw_cam_sensor_struct_t result;
    ADCS_pack_to_raw_cam1_sensor_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.which_sensor == ADCS_WHICH_CAM_SENSOR_CAM1);
    TEST_ASSERT_TRUE(result.cam_centroid_x == 513);
    TEST_ASSERT_TRUE(result.cam_centroid_y == -11261);
    TEST_ASSERT_TRUE(result.cam_capture_status == ADCS_CAPTURE_RESULT_SRAM_ERROR);
    TEST_ASSERT_TRUE(result.cam_detection_result == ADCS_DETECT_RESULT_SUN_NOT_FOUND);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_raw_cam2_sensor_struct()
{
    uint8_t input_params[6] = {0x01, 0x02, 0x03, 0xd4, 0x05, 0x06}; 

    ADCS_raw_cam_sensor_struct_t result;
    ADCS_pack_to_raw_cam1_sensor_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.which_sensor == ADCS_WHICH_CAM_SENSOR_CAM1);
    TEST_ASSERT_TRUE(result.cam_centroid_x == 513);
    TEST_ASSERT_TRUE(result.cam_centroid_y == -11261);
    TEST_ASSERT_TRUE(result.cam_capture_status == ADCS_CAPTURE_RESULT_SRAM_ERROR);
    TEST_ASSERT_TRUE(result.cam_detection_result == ADCS_DETECT_RESULT_SUN_NOT_FOUND);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_fine_sun_vector_struct() {
    
    uint8_t input_params[6] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc}; 

    ADCS_fine_sun_vector_struct_t result;
    ADCS_pack_to_fine_sun_vector_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.x_micro == 1333000);
    TEST_ASSERT_TRUE(result.y_micro == 3080600);
    TEST_ASSERT_TRUE(result.z_micro == -1725400);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_raw_coarse_sun_sensor_1_to_6_struct() {
    uint8_t input_params[6] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};
    ADCS_raw_coarse_sun_sensor_1_to_6_struct_t result;
    ADCS_pack_to_raw_coarse_sun_sensor_1_to_6_struct(input_params, &result);
    TEST_ASSERT_TRUE(result.coarse_sun_sensor_1 == 18);
    TEST_ASSERT_TRUE(result.coarse_sun_sensor_2 == 52);
    TEST_ASSERT_TRUE(result.coarse_sun_sensor_3 == 86);
    TEST_ASSERT_TRUE(result.coarse_sun_sensor_4 == 120);
    TEST_ASSERT_TRUE(result.coarse_sun_sensor_5 == 154);
    TEST_ASSERT_TRUE(result.coarse_sun_sensor_6 == 188);
    return 0;
}
uint8_t TEST_EXEC__ADCS_pack_to_raw_coarse_sun_sensor_7_to_10_struct() {
    uint8_t input_params[4] = {0x12, 0x34, 0x56, 0x78};
    ADCS_raw_coarse_sun_sensor_7_to_10_struct_t result;
    ADCS_pack_to_raw_coarse_sun_sensor_7_to_10_struct(input_params, &result);
    TEST_ASSERT_TRUE(result.coarse_sun_sensor_7 == 18);
    TEST_ASSERT_TRUE(result.coarse_sun_sensor_8 == 52);
    TEST_ASSERT_TRUE(result.coarse_sun_sensor_9 == 86);
    TEST_ASSERT_TRUE(result.coarse_sun_sensor_10 == 120);
    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_cubecontrol_current_struct() {
    uint8_t input_params[6] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};

    ADCS_cubecontrol_current_struct_t result;
    ADCS_pack_to_cubecontrol_current_struct(input_params, &result);

    TEST_ASSERT_TRUE(GEN_compare_doubles(result.cubecontrol_3v3_current_mA, 6508.7890625, ADCS_TEST_EPSILON / 100.0));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.cubecontrol_5v_current_mA, 15041.9921875, ADCS_TEST_EPSILON / 100.0));
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.cubecontrol_vbat_current_mA, 23575.1953125, ADCS_TEST_EPSILON / 100.0));
    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_raw_gps_status_struct() {
    uint8_t input_params[6] = {0x08, 0x34, 0x56, 0x78, 0x9a, 0xbc};
    ADCS_raw_gps_status_struct_t result;
    ADCS_pack_to_raw_gps_status_struct(input_params, &result);
    TEST_ASSERT_TRUE(result.gps_solution_status == ADCS_GPS_SOLUTION_STATUS_LARGE_RESIDUALS);
    TEST_ASSERT_TRUE(result.num_tracked_satellites == 52);
    TEST_ASSERT_TRUE(result.num_used_satellites == 86);
    TEST_ASSERT_TRUE(result.counter_xyz_log == 120);
    TEST_ASSERT_TRUE(result.counter_range_log == 154);
    TEST_ASSERT_TRUE(result.response_message_gps_log == 188);
    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_raw_gps_time_struct() {
    uint8_t input_params[6] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};
    ADCS_raw_gps_time_struct_t result;
    ADCS_pack_to_raw_gps_time_struct(input_params, &result);
    TEST_ASSERT_TRUE(result.gps_reference_week == 13330);
    TEST_ASSERT_TRUE(GEN_compare_doubles(result.gps_time_ms, 3164239958, ADCS_TEST_EPSILON));
    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_raw_gps_struct() {
    uint8_t input_params[6] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};
    ADCS_raw_gps_struct_t result;
    ADCS_pack_to_raw_gps_struct(ADCS_GPS_AXIS_Y, input_params, &result);
    TEST_ASSERT_TRUE(result.axis == ADCS_GPS_AXIS_Y);
    TEST_ASSERT_TRUE(result.ecef_position_meters == 2018915346);
    TEST_ASSERT_TRUE(result.ecef_velocity_meters_per_sec == -17254);

    uint8_t input_params_two[6] = {0x12, 0x34, 0xee, 0xff, 0x9a, 0x0c};
    ADCS_pack_to_raw_gps_struct(ADCS_GPS_AXIS_X, input_params_two, &result);
    TEST_ASSERT_TRUE(result.axis == ADCS_GPS_AXIS_X);
    TEST_ASSERT_TRUE(result.ecef_position_meters == -1166318);
    TEST_ASSERT_TRUE(result.ecef_velocity_meters_per_sec == 3226);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_measurements_struct() {
    
    uint8_t input_params[72] = {0xd2, 0x1c, 0xa3, 0xc5, 0x3e, 0x93, 0x49, 0xf8, 
                                0x65, 0xef, 0x1f, 0xd3, 0xd1, 0xdb, 0xce, 0x16, 
                                0x27, 0xc5, 0xb1, 0xe5, 0xd3, 0x19, 0x70, 0xd9, 
                                0x87, 0x6f, 0xa1, 0x09, 0xf0, 0xb7, 0x99, 0xcc, 
                                0xb9, 0x5f, 0x6b, 0xe1, 0xd3, 0x05, 0xc9, 0x19, 
                                0x9c, 0xd7, 0x02, 0x2a, 0x54, 0xc2, 0x4f, 0xca, 
                                0x7d, 0xa2, 0x0d, 0x03, 0x48, 0x98, 0xe6, 0xb7, 
                                0xac, 0x8d, 0x3d, 0x63, 0x0b, 0x2c, 0x84, 0x7f, 
                                0x32, 0x15, 0x47, 0x9e, 0x3c, 0x4a, 0xd3, 0x1c};
    
    ADCS_measurements_struct_t result;
    ADCS_pack_to_measurements_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.magnetic_field_x_nT == 73780);
    TEST_ASSERT_TRUE(result.magnetic_field_y_nT == -149410);
    TEST_ASSERT_TRUE(result.magnetic_field_z_nT == -278420);
    TEST_ASSERT_TRUE(result.coarse_sun_x_micro == -197500);
    TEST_ASSERT_TRUE(result.coarse_sun_y_micro == -425100);
    TEST_ASSERT_TRUE(result.coarse_sun_z_micro == -1148900);
    TEST_ASSERT_TRUE(result.sun_x_micro == -926300);
    TEST_ASSERT_TRUE(result.sun_y_micro == 583800);
    TEST_ASSERT_TRUE(result.sun_z_micro == -1506500);
    TEST_ASSERT_TRUE(result.nadir_x_micro == -673500);
    TEST_ASSERT_TRUE(result.nadir_y_micro == 661100);
    TEST_ASSERT_TRUE(result.nadir_z_micro == -987200);
    TEST_ASSERT_TRUE(result.x_angular_rate_mdeg_per_sec == 285510);
    TEST_ASSERT_TRUE(result.y_angular_rate_mdeg_per_sec == 24650);
    TEST_ASSERT_TRUE(result.z_angular_rate_mdeg_per_sec == -184480);
    TEST_ASSERT_TRUE(result.x_wheel_speed_rpm == -13159);
    TEST_ASSERT_TRUE(result.y_wheel_speed_rpm == 24505);
    TEST_ASSERT_TRUE(result.z_wheel_speed_rpm == -7829);
    TEST_ASSERT_TRUE(result.star1_body_x_micro == 149100);
    TEST_ASSERT_TRUE(result.star1_body_y_micro == 660100);
    TEST_ASSERT_TRUE(result.star1_body_z_micro == -1034000);
    TEST_ASSERT_TRUE(result.star1_orbit_x_micro == 1075400);
    TEST_ASSERT_TRUE(result.star1_orbit_y_micro == -1578800);
    TEST_ASSERT_TRUE(result.star1_orbit_z_micro == -1374500);
    TEST_ASSERT_TRUE(result.star2_body_x_micro == -2393900);
    TEST_ASSERT_TRUE(result.star2_body_y_micro == 78100);
    TEST_ASSERT_TRUE(result.star2_body_z_micro == -2655200);
    TEST_ASSERT_TRUE(result.star2_orbit_x_micro == -1845800);
    TEST_ASSERT_TRUE(result.star2_orbit_y_micro == -2926800);
    TEST_ASSERT_TRUE(result.star2_orbit_z_micro == 2540500);
    TEST_ASSERT_TRUE(result.star3_body_x_micro == 1127500);
    TEST_ASSERT_TRUE(result.star3_body_y_micro == 3264400);
    TEST_ASSERT_TRUE(result.star3_body_z_micro == 542600);
    TEST_ASSERT_TRUE(result.star3_orbit_x_micro == -2501700);
    TEST_ASSERT_TRUE(result.star3_orbit_y_micro == 1900400);
    TEST_ASSERT_TRUE(result.star3_orbit_z_micro == 737900);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_file_info_struct() {
    uint8_t input_params[12] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};
    ADCS_file_info_struct_t result;
    ADCS_pack_to_file_info_struct(input_params, &result);
    TEST_ASSERT_TRUE(result.busy_updating == true);
    TEST_ASSERT_TRUE(result.file_counter == 52);
    TEST_ASSERT_TRUE(result.file_crc16 == 48282);
    TEST_ASSERT_TRUE(result.file_date_time_msdos == 2018915346); 
    TEST_ASSERT_TRUE(result.file_size == 3164239958);
    TEST_ASSERT_TRUE(result.file_type == ADCS_FILE_TYPE_TELEMETRY_LOG);
    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_download_block_ready_struct() {
    uint8_t input_params[5] = {0x12, 0x34, 0x56, 0x78, 0x9a};
    ADCS_download_block_ready_struct_t result;
    ADCS_pack_to_download_block_ready_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.ready == false);
    TEST_ASSERT_TRUE(result.parameter_error == true);
    TEST_ASSERT_TRUE(result.block_crc16 == 22068);
    TEST_ASSERT_TRUE(result.block_length == 39544);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_acp_execution_state_struct() {

    uint8_t input_params[3] = {0x44, 0xab, 0x03};
    ADCS_acp_execution_state_struct_t result;

    ADCS_pack_to_acp_execution_state_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.time_since_iteration_start_ms == 43844);
    TEST_ASSERT_TRUE(result.current_execution_point == ADCS_CURRENT_EXECUTION_POINT_ADCS_UPDATE);

    return 0;

}

uint8_t TEST_EXEC__ADCS_pack_to_current_state_1_struct() {

    uint8_t input_params[6] = {0x44, 0xab, 0x03, 0x44, 0xab, 0x03};
    ADCS_current_state_1_struct_t result;

    ADCS_pack_to_current_state_1_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.estimation_mode == ADCS_ESTIMATION_MODE_MAGNETOMETER_AND_FINE_SUN_TRIAD_ALGORITHM);
    TEST_ASSERT_TRUE(result.control_mode == ADCS_CONTROL_MODE_Y_WHEEL_MOMENTUM_STABILIZED_STEADY_STATE);
    TEST_ASSERT_TRUE(result.run_mode == ADCS_RUN_MODE_SIMULATION);
    TEST_ASSERT_TRUE(result.asgp4_mode == ADCS_ASGP4_MODE_BACKGROUND);
    TEST_ASSERT_TRUE(result.cubecontrol_signal_enabled == false);
    TEST_ASSERT_TRUE(result.cubecontrol_motor_enabled == true);
    TEST_ASSERT_TRUE(result.cubesense1_enabled == false);
    TEST_ASSERT_TRUE(result.cubesense2_enabled == true);
    TEST_ASSERT_TRUE(result.cubewheel1_enabled == true);
    TEST_ASSERT_TRUE(result.cubewheel2_enabled == true);
    TEST_ASSERT_TRUE(result.cubewheel3_enabled == false);
    TEST_ASSERT_TRUE(result.cubestar_enabled == false);
    TEST_ASSERT_TRUE(result.gps_receiver_enabled == false);
    TEST_ASSERT_TRUE(result.gps_lna_power_enabled == false);
    TEST_ASSERT_TRUE(result.motor_driver_enabled == false);
    TEST_ASSERT_TRUE(result.sun_above_local_horizon == false);
    TEST_ASSERT_TRUE(result.cubesense1_comm_error == false);
    TEST_ASSERT_TRUE(result.cubesense2_comm_error == false);
    TEST_ASSERT_TRUE(result.cubecontrol_signal_comm_error == true);
    TEST_ASSERT_TRUE(result.cubecontrol_motor_comm_error == false);
    TEST_ASSERT_TRUE(result.cubewheel1_comm_error == false);
    TEST_ASSERT_TRUE(result.cubewheel2_comm_error == false);
    TEST_ASSERT_TRUE(result.cubewheel3_comm_error == true);
    TEST_ASSERT_TRUE(result.cubestar_comm_error == false);
    TEST_ASSERT_TRUE(result.magnetometer_range_error == true);
    TEST_ASSERT_TRUE(result.cam1_sram_overcurrent_detected == true);
    TEST_ASSERT_TRUE(result.cam1_3v3_overcurrent_detected == false);
    TEST_ASSERT_TRUE(result.cam1_sensor_busy_error == true);
    TEST_ASSERT_TRUE(result.cam1_sensor_detection_error == false);
    TEST_ASSERT_TRUE(result.sun_sensor_range_error == true);
    TEST_ASSERT_TRUE(result.cam2_sram_overcurrent_detected == false);
    TEST_ASSERT_TRUE(result.cam2_3v3_overcurrent_detected == true);
    TEST_ASSERT_TRUE(result.cam2_sensor_busy_error == true);
    TEST_ASSERT_TRUE(result.cam2_sensor_detection_error == true);
    TEST_ASSERT_TRUE(result.nadir_sensor_range_error == false);
    TEST_ASSERT_TRUE(result.rate_sensor_range_error == false);
    TEST_ASSERT_TRUE(result.wheel_speed_range_error == false);
    TEST_ASSERT_TRUE(result.coarse_sun_sensor_error == false);
    TEST_ASSERT_TRUE(result.startracker_match_error == false);
    TEST_ASSERT_TRUE(result.startracker_overcurrent_detected == false);

    uint8_t input_params_two[6] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};

    ADCS_pack_to_current_state_1_struct(input_params_two, &result);

    TEST_ASSERT_TRUE(result.estimation_mode == ADCS_ESTIMATION_MODE_MAGNETOMETER_RATE_FILTER);
    TEST_ASSERT_TRUE(result.control_mode == ADCS_CONTROL_MODE_DETUMBLING);
    TEST_ASSERT_TRUE(result.run_mode == ADCS_RUN_MODE_OFF);
    TEST_ASSERT_TRUE(result.asgp4_mode == ADCS_ASGP4_MODE_TRIGGER);
    TEST_ASSERT_TRUE(result.cubecontrol_signal_enabled == true);
    TEST_ASSERT_TRUE(result.cubecontrol_motor_enabled == true);
    TEST_ASSERT_TRUE(result.cubesense1_enabled == false);
    TEST_ASSERT_TRUE(result.cubesense2_enabled == false);
    TEST_ASSERT_TRUE(result.cubewheel1_enabled == false);
    TEST_ASSERT_TRUE(result.cubewheel2_enabled == true);
    TEST_ASSERT_TRUE(result.cubewheel3_enabled == true);
    TEST_ASSERT_TRUE(result.cubestar_enabled == false);
    TEST_ASSERT_TRUE(result.gps_receiver_enabled == true);
    TEST_ASSERT_TRUE(result.gps_lna_power_enabled == false);
    TEST_ASSERT_TRUE(result.motor_driver_enabled == true);
    TEST_ASSERT_TRUE(result.sun_above_local_horizon == false);
    TEST_ASSERT_TRUE(result.cubesense1_comm_error == false);
    TEST_ASSERT_TRUE(result.cubesense2_comm_error == false);
    TEST_ASSERT_TRUE(result.cubecontrol_signal_comm_error == false);
    TEST_ASSERT_TRUE(result.cubecontrol_motor_comm_error == true);
    TEST_ASSERT_TRUE(result.cubewheel1_comm_error == true);
    TEST_ASSERT_TRUE(result.cubewheel2_comm_error == true);
    TEST_ASSERT_TRUE(result.cubewheel3_comm_error == true);
    TEST_ASSERT_TRUE(result.cubestar_comm_error == false);
    TEST_ASSERT_TRUE(result.magnetometer_range_error == false);
    TEST_ASSERT_TRUE(result.cam1_sram_overcurrent_detected == true);
    TEST_ASSERT_TRUE(result.cam1_3v3_overcurrent_detected == false);
    TEST_ASSERT_TRUE(result.cam1_sensor_busy_error == true);
    TEST_ASSERT_TRUE(result.cam1_sensor_detection_error == true);
    TEST_ASSERT_TRUE(result.sun_sensor_range_error == false);
    TEST_ASSERT_TRUE(result.cam2_sram_overcurrent_detected == false);
    TEST_ASSERT_TRUE(result.cam2_3v3_overcurrent_detected == true);
    TEST_ASSERT_TRUE(result.cam2_sensor_busy_error == false);
    TEST_ASSERT_TRUE(result.cam2_sensor_detection_error == false);
    TEST_ASSERT_TRUE(result.nadir_sensor_range_error == true);
    TEST_ASSERT_TRUE(result.rate_sensor_range_error == true);
    TEST_ASSERT_TRUE(result.wheel_speed_range_error == true);
    TEST_ASSERT_TRUE(result.coarse_sun_sensor_error == true);
    TEST_ASSERT_TRUE(result.startracker_match_error == false);
    TEST_ASSERT_TRUE(result.startracker_overcurrent_detected == true);

    return 0;

}

uint8_t TEST_EXEC__ADCS_pack_to_raw_star_tracker_struct() {
    uint8_t input_params[57] = {0xae, 0xbc, 0x22, 0x53, 0x78, 0x37, 0x48, 0x83, 0x49, 0x85, 0x74, 0x78, 
                                0x90, 0xab, 0x89, 0x09, 0x0e, 0x97, 0x89, 0xc8, 0x88, 0x7d, 0x09, 0x0e, 
                                0x07, 0x90, 0xd0, 0x9c, 0xed, 0xc9, 0x07, 0xac, 0x0b, 0x70, 0xe7, 0x7d, 
                                0xc0, 0x9e, 0xae, 0xbc, 0x22, 0x53, 0x78, 0x37, 0x48, 0x83, 0x49, 0x85, 
                                0x74, 0x78, 0x90, 0xab, 0x89, 0xae};

    ADCS_raw_star_tracker_struct_t result;
    ADCS_pack_to_raw_star_tracker_struct(input_params, &result);

    TEST_ASSERT_TRUE(result.num_stars_detected == 174);
    TEST_ASSERT_TRUE(result.star_image_noise == 188);
    TEST_ASSERT_TRUE(result.invalid_stars == 34);
    TEST_ASSERT_TRUE(result.num_stars_identified == 83);
    TEST_ASSERT_TRUE(result.identification_mode == 120);
    TEST_ASSERT_TRUE(result.image_dark_value == 55);
    TEST_ASSERT_TRUE(result.image_capture_success == false);
    TEST_ASSERT_TRUE(result.detection_success == false);
    TEST_ASSERT_TRUE(result.identification_success == false);
    TEST_ASSERT_TRUE(result.attitude_success == true);
    TEST_ASSERT_TRUE(result.processing_time_error == false);
    TEST_ASSERT_TRUE(result.tracking_module_enabled == false);
    TEST_ASSERT_TRUE(result.prediction_enabled == true);
    TEST_ASSERT_TRUE(result.comms_error == false);
    TEST_ASSERT_TRUE(result.sample_period == 18819);
    TEST_ASSERT_TRUE(result.star1_confidence == 133);
    TEST_ASSERT_TRUE(result.star2_confidence == 116);
    TEST_ASSERT_TRUE(result.star3_confidence == 120);
    TEST_ASSERT_TRUE(result.magnitude_star1 == 43920);
    TEST_ASSERT_TRUE(result.magnitude_star2 == 2441);
    TEST_ASSERT_TRUE(result.magnitude_star3 == 38670);
    TEST_ASSERT_TRUE(result.catalogue_star1 == 51337);
    TEST_ASSERT_TRUE(result.centroid_x_star1 == 32136);
    TEST_ASSERT_TRUE(result.centroid_y_star1 == 3593);
    TEST_ASSERT_TRUE(result.catalogue_star2 == 36871);
    TEST_ASSERT_TRUE(result.centroid_x_star2 == -25392);
    TEST_ASSERT_TRUE(result.centroid_y_star2 == -13843);
    TEST_ASSERT_TRUE(result.catalogue_star3 == 44039);
    TEST_ASSERT_TRUE(result.centroid_x_star3 == 28683);
    TEST_ASSERT_TRUE(result.centroid_y_star3 == 32231);
    TEST_ASSERT_TRUE(result.capture_time_ms == 40640);
    TEST_ASSERT_TRUE(result.detection_time_ms == 48302);
    TEST_ASSERT_TRUE(result.identification_time_ms == 21282);
    TEST_ASSERT_TRUE(result.x_axis_rate_micro == 1420000);
    TEST_ASSERT_TRUE(result.y_axis_rate_micro == -3192800);
    TEST_ASSERT_TRUE(result.z_axis_rate_micro == -3141500);
    TEST_ASSERT_TRUE(result.q0_micro == 3083600);
    TEST_ASSERT_TRUE(result.q1_micro == -2161600);
    TEST_ASSERT_TRUE(result.q2_micro == -2085500);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_sd_card_format_erase_progress_struct() {
    uint8_t input_params[1] = {0x02};
    ADCS_sd_card_format_erase_progress_struct_t result;
    ADCS_pack_to_sd_card_format_erase_progress_struct(input_params, &result);
    TEST_ASSERT_TRUE(result.erase_all_busy == true);
    TEST_ASSERT_TRUE(result.format_busy == false);

    input_params[0] = 0x03;
    ADCS_pack_to_sd_card_format_erase_progress_struct(input_params, &result);
    TEST_ASSERT_TRUE(result.erase_all_busy == true);
    TEST_ASSERT_TRUE(result.format_busy == true);

    input_params[0] = 0x04;
    ADCS_pack_to_sd_card_format_erase_progress_struct(input_params, &result);
    TEST_ASSERT_TRUE(result.erase_all_busy == false);
    TEST_ASSERT_TRUE(result.format_busy == false);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_unix_time_ms() {
    uint8_t input_params[6] = {0x56, 0x8b, 0x21, 0x67, 0x62, 0x02};
    uint64_t result;

    ADCS_pack_to_unix_time_ms(input_params, &result);
    TEST_ASSERT_TRUE(result == 1730251606610);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_file_download_buffer_struct() {
    uint8_t input_params[22] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0x12, 0x34, 0x56, 0x78};
    ADCS_file_download_buffer_struct_t result;
    ADCS_pack_to_file_download_buffer_struct(input_params, &result);   

    TEST_ASSERT_TRUE(result.packet_counter == 13330);

    uint8_t bytes[20] = {86, 120, 154, 188, 18, 52, 86, 120, 154, 188, 18, 52, 86, 120, 154, 188, 18, 52, 86, 120};
    for (uint8_t i = 0; i < 20; i++) {
        TEST_ASSERT_TRUE(result.file_bytes[i] == bytes[i]);
    }

    return 0; 
}
uint8_t TEST_EXEC__ADCS_pack_to_sd_log_config_struct() {
    uint8_t input_params[13] = {0x56, 0x8b, 0x21, 0x67, 0x62, 0x02, 0x8c, 0x11, 0x62, 0x02, 0x8c, 0x11, 0x01};
    ADCS_sd_log_config_struct_t result_struct;

    ADCS_pack_to_sd_log_config_struct(input_params, 1, &result_struct);

    for (uint8_t i = 0; i < 10; i++) {
        TEST_ASSERT_TRUE((result_struct.log_bitmask)[i] == input_params[i]);
    }
    
    TEST_ASSERT_TRUE(result_struct.log_period_s == 4492);
    TEST_ASSERT_TRUE(result_struct.which_sd == ADCS_SD_LOG_DESTINATION_SECONDARY_SD);

    return 0;
}

uint8_t TEST_EXEC__ADCS_convert_double_to_string() {
    char output_string[20];
    
    TEST_ASSERT_TRUE(ADCS_convert_double_to_string(-2.5,6,&output_string[0],20) == 0);
    TEST_ASSERT_TRUE(strcmp(output_string, "-2.500000") == 0);

    TEST_ASSERT_TRUE(ADCS_convert_double_to_string(1000,2,&output_string[0],20) == 0);
    TEST_ASSERT_TRUE(strcmp(output_string, "1000.00") == 0);

    TEST_ASSERT_TRUE(ADCS_convert_double_to_string(-4,3,&output_string[0],20) == 0);
    TEST_ASSERT_TRUE(strcmp(output_string, "-4.000") == 0);

    TEST_ASSERT_TRUE(ADCS_convert_double_to_string(-0.9,1,&output_string[0],20) == 0);
    TEST_ASSERT_TRUE(strcmp(output_string, "-0.9") == 0);

    return 0;

}

uint8_t TEST_EXEC__ADCS_combine_sd_log_bitmasks() {
    uint8_t output[10] = {0};
    const uint8_t num_logs = 3;
    const uint8_t* commissioning_data[3] = {ADCS_SD_LOG_MASK_ESTIMATED_ANGULAR_RATES, ADCS_SD_LOG_MASK_RATE_SENSOR_RATES, ADCS_SD_LOG_MASK_RAW_MAGNETOMETER};
    ADCS_combine_sd_log_bitmasks(commissioning_data, num_logs, output);

    uint8_t test_array[10] = {0x00, 0x20, 0x00, 0x40, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00};
    for (uint8_t i = 0; i < 10; i++) {
        TEST_ASSERT_TRUE(output[i] == test_array[i]);
    }

    uint8_t output_two[10] = {0};
    const uint8_t num_logs_two = 14;  
    const uint8_t* commissioning_data_two[14] = {ADCS_SD_LOG_MASK_FINE_ESTIMATED_ANGULAR_RATES, ADCS_SD_LOG_MASK_ESTIMATED_ATTITUDE_ANGLES, ADCS_SD_LOG_MASK_ESTIMATED_GYRO_BIAS, ADCS_SD_LOG_MASK_ESTIMATION_INNOVATION_VECTOR, 
                                                    ADCS_SD_LOG_MASK_MAGNETIC_FIELD_VECTOR, ADCS_SD_LOG_MASK_RATE_SENSOR_RATES, ADCS_SD_LOG_MASK_FINE_SUN_VECTOR, ADCS_SD_LOG_MASK_NADIR_VECTOR, ADCS_SD_LOG_MASK_WHEEL_SPEED, ADCS_SD_LOG_MASK_MAGNETORQUER_COMMAND,
                                                    ADCS_SD_LOG_MASK_IGRF_MODELLED_MAGNETIC_FIELD_VECTOR, ADCS_SD_LOG_MASK_QUATERNION_ERROR_VECTOR, ADCS_SD_LOG_MASK_SATELLITE_POSITION_LLH, ADCS_SD_LOG_MASK_WHEEL_SPEED_COMMANDS};
    ADCS_combine_sd_log_bitmasks(commissioning_data_two, num_logs_two, output_two);

    uint8_t test_array_two[10] = {0x80, 0x7B, 0x00, 0x10, 0xEA, 0x00, 0x00, 0x08, 0x00, 0x00};
    for (uint8_t i = 0; i < 10; i++) {
        TEST_ASSERT_TRUE(output_two[i] == test_array_two[i]);
    }

    return 0;

}

uint8_t TEST_EXEC__ADCS_pack_to_wheel_currents_struct() {
    uint8_t input_data[6] = {0x10, 0x27, 0x20, 0x4E, 0x30, 0x75};
    ADCS_wheel_currents_struct_t result;

    ADCS_pack_to_wheel_currents_struct(input_data, &result);

    TEST_ASSERT_TRUE(result.wheel1_current_microamps == 100000);
    TEST_ASSERT_TRUE(result.wheel2_current_microamps == 200000);
    TEST_ASSERT_TRUE(result.wheel3_current_microamps == 300000);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_cubesense_currents_struct() {
    uint8_t input[8] = {0x10, 0x27, 0x20, 0x4E, 0x30, 0x75, 0x40, 0x9C};

    ADCS_cubesense_currents_struct_t result;
    ADCS_pack_to_cubesense_currents_struct(input, &result);

    TEST_ASSERT_TRUE(result.cubesense1_3v3_current_microamps == 1000000);
    TEST_ASSERT_TRUE(result.cubesense1_sram_current_microamps == 2000000);
    TEST_ASSERT_TRUE(result.cubesense2_3v3_current_microamps == 3000000);
    TEST_ASSERT_TRUE(result.cubesense2_sram_current_microamps == 4000000);

    return 0;
}

uint8_t TEST_EXEC__ADCS_pack_to_misc_currents_struct() {
    uint8_t input[6] = {0x10, 0x27, 0x20, 0x4E, 0x18, 0xFC};

    ADCS_misc_currents_struct_t result;
    ADCS_pack_to_misc_currents_struct(input, &result);

    TEST_ASSERT_TRUE(result.cubestar_current_microamps == 100000);
    TEST_ASSERT_TRUE(result.torquer_current_microamps == 2000000);
    TEST_ASSERT_TRUE(result.cubestar_mcu_temperature_mdeg_celsius == -10000);

    return 0;
}