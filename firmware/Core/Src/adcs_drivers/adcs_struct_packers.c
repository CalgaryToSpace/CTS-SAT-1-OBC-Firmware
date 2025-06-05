#include "adcs_drivers/adcs_types.h"
#include "adcs_drivers/adcs_command_ids.h"
#include "adcs_drivers/adcs_struct_packers.h"
#include "adcs_drivers/adcs_types_to_json.h"
#include "adcs_drivers/adcs_commands.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


/// @brief Packs the ADCS received raw data into the appropriate structure for the ADCS_cmd_ack command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_ack_struct(uint8_t* data_received, ADCS_cmd_ack_struct_t *result) {

    // map temp buffer to Ack struct
    result->last_id = data_received[0];
    result->processed = data_received[1];
    result->error_flag = data_received[2];
    result->error_index = data_received[3];

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_identification_struct(uint8_t *data_received, ADCS_id_struct_t *result) {
    result->node_type = data_received[0];
    result->interface_version = data_received[1];
    result->major_firmware_version = data_received[2];
    result->minor_firmware_version = data_received[3];
    result->seconds_since_startup = data_received[4] | (data_received[5] << 8);
    result->ms_past_second = data_received[6] | (data_received[7] << 8);
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_program_status_struct(uint8_t* data_received, ADCS_boot_running_status_struct_t *result) {
    // map to struct
    result->reset_cause = (data_received[0] & 0xF0) >> 4; // takes upper four bits of byte 0
    result->boot_cause = data_received[0] & 0x0F; // take upper four bits of byte 0
    result->boot_counter = data_received[2] << 8 | data_received[1]; // uint16_t
    result->boot_program_index = data_received[3];
    result->major_firmware_version = data_received[4]; // uint8_t
    result->minor_firmware_version = data_received[5]; // uint8_t

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_comms_status_struct(uint8_t *data_received, ADCS_comms_status_struct_t *result) {
    result->cmd_counter = data_received[1] << 8 | data_received[0]; // uint16_t
    result->tlm_counter = data_received[3] << 8 | data_received[2]; // uint16_t
    // bits we care about: 0b10011000 (the others are for UART and CAN, which we are not using)
    result->cmd_buffer_overrun = (data_received[4] & 0x80) >> 7; // first bit is 1 if TC buffer was overrun while receiving a telecommand
    result->i2c_tlm_error = (data_received[4] & 0x10) >> 4; // fourth bit is 1 if the number of data clocked out was more than the telemetry package
    result->i2c_cmd_error = (data_received[4] & 0x8) >> 3; // fifth bit is 1 if the telecommand sent exceeded the buffer size
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_power_control_struct(uint8_t* data_received, ADCS_power_control_struct_t *result) {

    // map to struct; all of these are two-bit enums
    // within the byte, everything goes in reverse order!!
    result->cube_control_signal = (data_received[0]) & 0x03;
    result->cube_control_motor = (data_received[0] >> 2) & 0x03;
    result->cube_sense1 = (data_received[0] >> 4) & 0x03;
    result->cube_sense2 = (data_received[0] >> 6) & 0x03;

    result->cube_star_power = (data_received[1]) & 0x03;
    result->cube_wheel1_power = (data_received[1] >> 2) & 0x03;
    result->cube_wheel2_power =  (data_received[1] >> 4) & 0x03;
    result->cube_wheel3_power = (data_received[1] >> 6) & 0x03;

    result->motor_power = (data_received[2]) & 0x03;
    result->gps_power = (data_received[2] >> 2) & 0x03;

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_angular_rates_struct(uint8_t *data_received, ADCS_angular_rates_struct_t *result) {
    // values given as int16, deg/s value is raw value * 0.01, give integer as m_deg/s
    // need to convert to int16 first, then double, to ensure negative numbers are represented correctly
    result->x_rate_mdeg_per_sec = (int32_t) ((int16_t) (data_received[1] << 8 | data_received[0])) * 10;
    result->y_rate_mdeg_per_sec = (int32_t) ((int16_t) (data_received[3] << 8 | data_received[2])) * 10;
    result->z_rate_mdeg_per_sec = (int32_t) ((int16_t) (data_received[5] << 8 | data_received[4])) * 10;
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_llh_position_struct(uint8_t *data_received, ADCS_llh_position_struct_t *result) {
    // formatted value (deg or km) = raw value * 0.01
    // need to convert to int16 first, then double, to ensure negative numbers are represented correctly
    result->latitude_mdeg = (int32_t) ((int16_t) (data_received[1] << 8 | data_received[0])) * 10; 
    result->longitude_mdeg = (int32_t) ((int16_t) (data_received[3] << 8 | data_received[2])) * 10; 
    result->altitude_meters  = (uint32_t) ((uint16_t) (data_received[5] << 8 | data_received[4])) * 10; 
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_unix_time_save_mode_struct(uint8_t *data_received, ADCS_set_unix_time_save_mode_struct_t *result) {
    result->save_now = data_received[0] & 0x1; // 0b00000001
    result->save_on_update = (data_received[0] & 0x2) >> 1; // 0b00000010
    result->save_periodic = (data_received[0] & 0x4) >> 2; // 0b00000100
    result->period_s = data_received[1];
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_orbit_params_struct(uint8_t *data_received, ADCS_orbit_params_struct_t *result) {
    memcpy(&result->inclination_deg, &data_received[0], sizeof(double));
    memcpy(&result->eccentricity, &data_received[8], sizeof(double));
    memcpy(&result->ascending_node_right_ascension_deg, &data_received[16], sizeof(double));
    memcpy(&result->perigee_argument_deg, &data_received[24], sizeof(double));
    memcpy(&result->b_star_drag_term, &data_received[32], sizeof(double));
    memcpy(&result->mean_motion_orbits_per_day, &data_received[40], sizeof(double));
    memcpy(&result->mean_anomaly_deg, &data_received[48], sizeof(double));
    memcpy(&result->epoch_year_point_day, &data_received[56], sizeof(double));
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_rated_sensor_rates_struct(uint8_t *data_received, ADCS_rated_sensor_rates_struct_t *result) {
    // formatted value (deg/s) = raw value * 0.01
    // need to convert to int16 first, then double, to ensure negative numbers are represented correctly
    result->x_mdeg_per_sec = (int32_t) ((int16_t) (data_received[1] << 8 | data_received[0])) * 10; 
    result->y_mdeg_per_sec = (int32_t) ((int16_t) (data_received[3] << 8 | data_received[2])) * 10; 
    result->z_mdeg_per_sec = (int32_t) ((int16_t) (data_received[5] << 8 | data_received[4])) * 10; 
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_wheel_speed_struct(uint8_t *data_received, ADCS_wheel_speed_struct_t *result) {
    // all values in rpm
    result->actual_wheel_speed = true; // actual wheel speed
    result->x_rpm = data_received[1] << 8 | data_received[0];
    result->y_rpm = data_received[3] << 8 | data_received[2];
    result->z_rpm = data_received[5] << 8 | data_received[4];
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_magnetorquer_command_struct(uint8_t *data_received, ADCS_magnetorquer_command_struct_t *result) {
    // formatted value (sec) = raw value * 0.01
    result->x_ms = (int32_t) ((int16_t) (data_received[1] << 8 | data_received[0])) * 10;
    result->y_ms = (int32_t) ((int16_t) (data_received[3] << 8 | data_received[2])) * 10;
    result->z_ms = (int32_t) ((int16_t) (data_received[5] << 8 | data_received[4])) * 10;
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_raw_magnetometer_values_struct(uint8_t *data_received, ADCS_raw_magnetometer_values_struct_t *result) {
    result->x_raw = data_received[1] << 8 | data_received[0];
    result->y_raw = data_received[3] << 8 | data_received[2];
    result->z_raw = data_received[5] << 8 | data_received[4];
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_fine_angular_rates_struct(uint8_t *data_received, ADCS_fine_angular_rates_struct_t *result) {
    // formatted value (deg/s) = raw value * 0.001
    result->x_mdeg_per_sec = (int16_t) (data_received[1] << 8 | data_received[0]);
    result->y_mdeg_per_sec = (int16_t) (data_received[3] << 8 | data_received[2]);
    result->z_mdeg_per_sec = (int16_t) (data_received[5] << 8 | data_received[4]);
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_magnetometer_config_struct(uint8_t *data_received, ADCS_magnetometer_config_struct_t *result) {
    // formatted value for mounting transform angles (deg/s) = raw value * 0.01
    result->mounting_transform_alpha_angle_mdeg_per_sec = (int32_t) ((int16_t) (data_received[1] << 8 | data_received[0])) * 10;
    result->mounting_transform_beta_angle_mdeg_per_sec =  (int32_t) ((int16_t) (data_received[3] << 8 | data_received[2])) * 10;
    result->mounting_transform_gamma_angle_mdeg_per_sec = (int32_t) ((int16_t) (data_received[5] << 8 | data_received[4])) * 10;
    // formatted value (deg/s) = raw value * 0.001
    result->channel_1_offset_mdeg_per_sec = (int16_t) (data_received[7] << 8 | data_received[6]);
    result->channel_2_offset_mdeg_per_sec = (int16_t) (data_received[9] << 8 | data_received[8]);
    result->channel_3_offset_mdeg_per_sec = (int16_t) (data_received[11] << 8 | data_received[10]);
    result->sensitivity_matrix_s11_mdeg_per_sec = (int16_t) (data_received[13] << 8 | data_received[12]);
    result->sensitivity_matrix_s22_mdeg_per_sec = (int16_t) (data_received[15] << 8 | data_received[14]);
    result->sensitivity_matrix_s33_mdeg_per_sec = (int16_t) (data_received[17] << 8 | data_received[16]);
    result->sensitivity_matrix_s12_mdeg_per_sec = (int16_t) (data_received[19] << 8 | data_received[18]);
    result->sensitivity_matrix_s13_mdeg_per_sec = (int16_t) (data_received[21] << 8 | data_received[20]);
    result->sensitivity_matrix_s21_mdeg_per_sec = (int16_t) (data_received[23] << 8 | data_received[22]);
    result->sensitivity_matrix_s23_mdeg_per_sec = (int16_t) (data_received[25] << 8 | data_received[24]);
    result->sensitivity_matrix_s31_mdeg_per_sec = (int16_t) (data_received[27] << 8 | data_received[26]);
    result->sensitivity_matrix_s32_mdeg_per_sec = (int16_t) (data_received[29] << 8 | data_received[28]);
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_commanded_attitude_angles_struct(uint8_t *data_received, ADCS_commanded_angles_struct_t *result) {
    // Formatted value is obtained using the formula: (formatted value) [deg] = RAWVAL*0.01
    result->x_mdeg = ((int16_t)(data_received[1] << 8 | data_received[0])) * 10;
    result->y_mdeg = ((int16_t)(data_received[3] << 8 | data_received[2])) * 10;
    result->z_mdeg = ((int16_t)(data_received[5] << 8 | data_received[4])) * 10;
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_estimation_params_struct(uint8_t* data_received, ADCS_estimation_params_struct_t *result) {
    // map temp buffer to struct
    memcpy(&result->magnetometer_rate_filter_system_noise, &data_received[0], 4);
    memcpy(&result->extended_kalman_filter_system_noise, &data_received[4], 4);
    memcpy(&result->coarse_sun_sensor_measurement_noise, &data_received[8], 4);
    memcpy(&result->sun_sensor_measurement_noise, &data_received[12], 4);
    memcpy(&result->nadir_sensor_measurement_noise, &data_received[16], 4);
    memcpy(&result->magnetometer_measurement_noise, &data_received[20], 4);
    memcpy(&result->star_tracker_measurement_noise, &data_received[24], 4);
    result->use_sun_sensor = (data_received[28] & 0x1); // 0b00000001 
    result->use_nadir_sensor = (data_received[28] & 0x2) >> 1; // 0b00000010
    result->use_css = (data_received[28] & 0x4) >> 2; // 0b00000100
    result->use_star_tracker = (data_received[28] & 0x8) >> 3; // 0b00001000
    result->nadir_sensor_terminator_test = (data_received[28] & 0x10) >> 4; // 0b00010000
    result->automatic_magnetometer_recovery = (data_received[28] & 0x20) >> 5; // 0b00100000
    result->magnetometer_mode = (data_received[28] & 0xc0) >> 6; // 0b11000000
    result->magnetometer_selection_for_raw_magnetometer_telemetry = (data_received[29] & 0x3); // 0b00000011
    result->automatic_estimation_transition_due_to_rate_sensor_errors = (data_received[29] & 4) >> 2; // 0b00000100
    result->wheel_30s_power_up_delay = (data_received[29] & 0x8) >> 3; // 0b00001000
    result->error_counter_reset_period_min = data_received[30];

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_augmented_sgp4_params_struct(uint8_t* data_received, ADCS_augmented_sgp4_params_struct_t *result) {
    // map temp buffer to struct
    result->incl_coefficient_milli = (int16_t) (data_received[1] << 8 | data_received[0]);
    result->raan_coefficient_milli = (int16_t) (data_received[3] << 8 | data_received[2]);
    result->ecc_coefficient_milli = (int16_t) (data_received[5] << 8 | data_received[4]);
    result->aop_coefficient_milli = (int16_t) (data_received[7] << 8 | data_received[6]);
    result->time_coefficient_milli = (int16_t) (data_received[9] << 8 | data_received[8]);
    result->pos_coefficient_milli = (int16_t) (data_received[11] << 8 | data_received[10]);
    result->maximum_position_error_milli = ((int32_t)((int16_t)data_received[12])) * 100;
    result->augmented_sgp4_filter = (ADCS_augmented_sgp4_filter_enum_t)data_received[13];
    result->xp_coefficient_nano = ((int64_t)((int32_t)(data_received[17] << 24 | data_received[16] << 16 | data_received[15] << 8 | data_received[14]))) * 100;
    result->yp_coefficient_nano = ((int64_t)((int32_t)(data_received[21] << 24 | data_received[20] << 16 | data_received[19] << 8 | data_received[18]))) * 100;
    result->gps_roll_over = data_received[22];
    result->position_sd_milli = ((int32_t)((int16_t)data_received[23])) * 100;
    result->velocity_sd_milli = (((int16_t)data_received[24])) * 10; 
    result->min_satellites = data_received[25];
    result->time_gain_milli = (((int16_t)data_received[26])) * 10; 
    result->max_lag_milli = (((int16_t)data_received[27])) * 10; 
    result->min_samples = (data_received[29] << 8 | data_received[28]);

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_tracking_controller_target_reference_struct(uint8_t* data_received, ADCS_tracking_controller_target_struct_t *ref) {
    // map temp buffer to struct
    memcpy(&ref->longitude_degrees, &data_received[0], 4);
    memcpy(&ref->latitude_degrees, &data_received[4], 4);
    memcpy(&ref->altitude_meters, &data_received[8], 4);

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_rate_gyro_config_struct(uint8_t* data_received, ADCS_rate_gyro_config_struct_t *result) {
    result->gyro1 = data_received[0];
    result->gyro2 = data_received[1];
    result->gyro3 = data_received[2];

    // Raw parameter value is obtained using the formula: (formatted value) [deg/s] = RAWVAL*0.001
    result->x_rate_offset_mdeg_per_sec = (int16_t) (data_received[4] << 8 | data_received[3]);
    result->y_rate_offset_mdeg_per_sec = (int16_t) (data_received[6] << 8 | data_received[5]);
    result->z_rate_offset_mdeg_per_sec = (int16_t) (data_received[8] << 8 | data_received[7]);

    result->rate_sensor_mult = data_received[9];

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_estimated_attitude_angles_struct(uint8_t *data_received, ADCS_estimated_attitude_angles_struct_t *angles) {
    angles->estimated_roll_angle_mdeg = (int32_t) ((int16_t) (data_received[1] << 8 | data_received[0])) * 10;
    angles->estimated_pitch_angle_mdeg = (int32_t) ((int16_t) (data_received[3] << 8 | data_received[2])) * 10;
    angles->estimated_yaw_angle_mdeg = (int32_t) ((int16_t) (data_received[5] << 8 | data_received[4])) * 10;
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_magnetic_field_vector_struct(uint8_t *data_received, ADCS_magnetic_field_vector_struct_t *vector_components) {
    // gives vector components in nT (10^-9 Teslas)
    vector_components->x_nT = (int32_t) ((int16_t) (data_received[1] << 8 | data_received[0])) * 10;
    vector_components->y_nT = (int32_t) ((int16_t) (data_received[3] << 8 | data_received[2])) * 10;
    vector_components->z_nT = (int32_t) ((int16_t) (data_received[5] << 8 | data_received[4])) * 10;
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_fine_sun_vector_struct(uint8_t *data_received, ADCS_fine_sun_vector_struct_t *vector_components) {
    vector_components->x_micro = ((int32_t) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 100;
    vector_components->y_micro = ((int32_t) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 100;
    vector_components->z_micro = ((int32_t) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 100;
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_nadir_vector_struct(uint8_t *data_received, ADCS_nadir_vector_struct_t *vector_components) {
    vector_components->x_micro = ((int32_t) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 100;
    vector_components->y_micro = ((int32_t) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 100;
    vector_components->z_micro = ((int32_t) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 100;
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_commanded_wheel_speed_struct(uint8_t *data_received, ADCS_wheel_speed_struct_t *result) {
    // all values in rpm
    result->actual_wheel_speed = false; // commanded, not actual
    result->x_rpm = data_received[1] << 8 | data_received[0];
    result->y_rpm = data_received[3] << 8 | data_received[2];
    result->z_rpm = data_received[5] << 8 | data_received[4];
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_igrf_magnetic_field_vector_struct(uint8_t *data_received, ADCS_magnetic_field_vector_struct_t *vector_components) {
    // gives vector components in nT (10^-9 Teslas)
    vector_components->x_nT = (int32_t) ((int16_t) (data_received[1] << 8 | data_received[0])) * 10;
    vector_components->y_nT = (int32_t) ((int16_t) (data_received[3] << 8 | data_received[2])) * 10;
    vector_components->z_nT = (int32_t) ((int16_t) (data_received[5] << 8 | data_received[4])) * 10;
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_quaternion_error_vector_struct(uint8_t *data_received, ADCS_quaternion_error_vector_struct_t *result) {
    result->quaternion_error_q1_micro = ((int32_t) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 100;
    result->quaternion_error_q2_micro = ((int32_t) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 100;
    result->quaternion_error_q3_micro = ((int32_t) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 100;

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_estimated_gyro_bias_struct(uint8_t* data_received, ADCS_estimated_gyro_bias_struct_t *result) {
    result->estimated_x_gyro_bias_mdeg_per_sec = (int32_t) ((int16_t) (data_received[1] << 8 | data_received[0]));
    result->estimated_y_gyro_bias_mdeg_per_sec = (int32_t) ((int16_t) (data_received[3] << 8 | data_received[2]));
    result->estimated_z_gyro_bias_mdeg_per_sec = (int32_t) ((int16_t) (data_received[5] << 8 | data_received[4]));

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_estimation_innovation_vector_struct(uint8_t* data_received, ADCS_estimation_innovation_vector_struct_t *result) {
    result->innovation_vector_x_micro = ((int32_t) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 100;
    result->innovation_vector_y_micro = ((int32_t) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 100;
    result->innovation_vector_z_micro = ((int32_t) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 100;

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_raw_cam1_sensor_struct(uint8_t* data_received, ADCS_raw_cam_sensor_struct_t *result) {
    result->which_sensor = ADCS_WHICH_CAM_SENSOR_CAM1;
    result->cam_centroid_x = (int16_t) (data_received[1] << 8 | data_received[0]);
    result->cam_centroid_y = (int16_t) (data_received[3] << 8 | data_received[2]);
    result->cam_capture_status = data_received[4];
    result->cam_detection_result = data_received[5];

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_raw_cam2_sensor_struct(uint8_t* data_received, ADCS_raw_cam_sensor_struct_t *result) {
    result->which_sensor = ADCS_WHICH_CAM_SENSOR_CAM2;
    result->cam_centroid_x = (int16_t) (data_received[1] << 8 | data_received[0]);
    result->cam_centroid_y = (int16_t) (data_received[3] << 8 | data_received[2]);
    result->cam_capture_status = data_received[4];
    result->cam_detection_result = data_received[5];

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_raw_coarse_sun_sensor_1_to_6_struct(uint8_t* data_received, ADCS_raw_coarse_sun_sensor_1_to_6_struct_t *result) {
    result->coarse_sun_sensor_1 = data_received[0];
    result->coarse_sun_sensor_2 = data_received[1];
    result->coarse_sun_sensor_3 = data_received[2];
    result->coarse_sun_sensor_4 = data_received[3];
    result->coarse_sun_sensor_5 = data_received[4];
    result->coarse_sun_sensor_6 = data_received[5];

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_raw_coarse_sun_sensor_7_to_10_struct(uint8_t* data_received, ADCS_raw_coarse_sun_sensor_7_to_10_struct_t *result) {
    result->coarse_sun_sensor_7 = data_received[0];
    result->coarse_sun_sensor_8 = data_received[1];
    result->coarse_sun_sensor_9 = data_received[2];
    result->coarse_sun_sensor_10 = data_received[3];

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_cubecontrol_current_struct(uint8_t* data_received, ADCS_cubecontrol_current_struct_t *result) {
    // everything in mA after multiplying RAWVAL*0.48828125 (aka dividing by 2.048 exactly)
    result->cubecontrol_3v3_current_mA = (( double ) ((uint16_t) (data_received[1] << 8 | data_received[0]))) / 2.048;
    result->cubecontrol_5v_current_mA = (( double ) ((uint16_t) (data_received[3] << 8 | data_received[2]))) / 2.048;
    result->cubecontrol_vbat_current_mA = (( double ) ((uint16_t) (data_received[5] << 8 | data_received[4]))) / 2.048;

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_raw_gps_status_struct(uint8_t* data_received, ADCS_raw_gps_status_struct_t *result) {
    result->gps_solution_status = (ADCS_gps_solution_status_enum_t) data_received[0];
    result->num_tracked_satellites = data_received[1];
    result->num_used_satellites = data_received[2];
    result->counter_xyz_log = data_received[3];
    result->counter_range_log = data_received[4];
    result->response_message_gps_log = data_received[5];

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_raw_gps_time_struct(uint8_t* data_received, ADCS_raw_gps_time_struct_t *result) {
    result->gps_reference_week = (uint16_t)(data_received[1] << 8 | data_received[0]);
    result->gps_time_ms = (uint32_t) (data_received[5] << 24 | data_received[4] << 16 | data_received[3] << 8 | data_received[2]);

    return 0; 
}

/// @brief Packs the ADCS received raw data into the appropriate structure for any of the three Raw_GPS commands (X, Y, Z).
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_raw_gps_struct(ADCS_gps_axis_enum_t axis, uint8_t *data_received, ADCS_raw_gps_struct_t *result) {
    result->axis = axis; // this function works for three commands, so we need to keep this information
    result->ecef_position_meters = (int32_t) (data_received[3] << 24 | data_received[2] << 16 | 
                                               data_received[1] << 8 | data_received[0]); // ECEF Position Z [m]
    result->ecef_velocity_meters_per_sec = (int16_t)(data_received[5] << 8 | data_received[4]);  // ECEF Velocity Z [m/s]

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_measurements_struct(uint8_t* telemetry_data, ADCS_measurements_struct_t *measurements) {

    // Parse each telemetry entry according to Table 150 in the Firmware Reference Manual
    measurements->magnetic_field_x_nT = (int32_t)((int16_t)(telemetry_data[1] << 8 | telemetry_data[0])) * 10;
    measurements->magnetic_field_y_nT = (int32_t)((int16_t)(telemetry_data[3] << 8 | telemetry_data[2])) * 10;
    measurements->magnetic_field_z_nT = (int32_t)((int16_t)(telemetry_data[5] << 8 | telemetry_data[4])) * 10;
    measurements->coarse_sun_x_micro = (int32_t)((int16_t)(telemetry_data[7] << 8 | telemetry_data[6])) * 100;
    measurements->coarse_sun_y_micro = (int32_t)((int16_t)(telemetry_data[9] << 8 | telemetry_data[8])) * 100;
    measurements->coarse_sun_z_micro = (int32_t)((int16_t)(telemetry_data[11] << 8 | telemetry_data[10])) * 100;
    measurements->sun_x_micro = (int32_t)((int16_t)(telemetry_data[13] << 8 | telemetry_data[12])) * 100;
    measurements->sun_y_micro = (int32_t)((int16_t)(telemetry_data[15] << 8 | telemetry_data[14])) * 100;
    measurements->sun_z_micro = (int32_t)((int16_t)(telemetry_data[17] << 8 | telemetry_data[16])) * 100;
    measurements->nadir_x_micro = (int32_t)((int16_t)(telemetry_data[19] << 8 | telemetry_data[18])) * 100;
    measurements->nadir_y_micro = (int32_t)((int16_t)(telemetry_data[21] << 8 | telemetry_data[20])) * 100;
    measurements->nadir_z_micro = (int32_t)((int16_t)(telemetry_data[23] << 8 | telemetry_data[22])) * 100;
    measurements->x_angular_rate_mdeg_per_sec = (int32_t)((int16_t)(telemetry_data[25] << 8 | telemetry_data[24])) * 10;
    measurements->y_angular_rate_mdeg_per_sec = (int32_t)((int16_t)(telemetry_data[27] << 8 | telemetry_data[26])) * 10;
    measurements->z_angular_rate_mdeg_per_sec = (int32_t)((int16_t)(telemetry_data[29] << 8 | telemetry_data[28])) * 10;
    measurements->x_wheel_speed_rpm = ((int16_t)(telemetry_data[31] << 8 | telemetry_data[30]));
    measurements->y_wheel_speed_rpm = ((int16_t)(telemetry_data[33] << 8 | telemetry_data[32]));
    measurements->z_wheel_speed_rpm = ((int16_t)(telemetry_data[35] << 8 | telemetry_data[34]));
    measurements->star1_body_x_micro = (int32_t)((int16_t)(telemetry_data[37] << 8 | telemetry_data[36])) * 100;
    measurements->star1_body_y_micro = (int32_t)((int16_t)(telemetry_data[39] << 8 | telemetry_data[38])) * 100;
    measurements->star1_body_z_micro = (int32_t)((int16_t)(telemetry_data[41] << 8 | telemetry_data[40])) * 100;
    measurements->star1_orbit_x_micro = (int32_t)((int16_t)(telemetry_data[43] << 8 | telemetry_data[42])) * 100;
    measurements->star1_orbit_y_micro = (int32_t)((int16_t)(telemetry_data[45] << 8 | telemetry_data[44])) * 100;
    measurements->star1_orbit_z_micro = (int32_t)((int16_t)(telemetry_data[47] << 8 | telemetry_data[46])) * 100;
    measurements->star2_body_x_micro = (int32_t)((int16_t)(telemetry_data[49] << 8 | telemetry_data[48])) * 100;
    measurements->star2_body_y_micro = (int32_t)((int16_t)(telemetry_data[51] << 8 | telemetry_data[50])) * 100;
    measurements->star2_body_z_micro = (int32_t)((int16_t)(telemetry_data[53] << 8 | telemetry_data[52])) * 100;
    measurements->star2_orbit_x_micro = (int32_t)((int16_t)(telemetry_data[55] << 8 | telemetry_data[54])) * 100;
    measurements->star2_orbit_y_micro = (int32_t)((int16_t)(telemetry_data[57] << 8 | telemetry_data[56])) * 100;
    measurements->star2_orbit_z_micro = (int32_t)((int16_t)(telemetry_data[59] << 8 | telemetry_data[58])) * 100;
    measurements->star3_body_x_micro = (int32_t)((int16_t)(telemetry_data[61] << 8 | telemetry_data[60])) * 100;
    measurements->star3_body_y_micro = (int32_t)((int16_t)(telemetry_data[63] << 8 | telemetry_data[62])) * 100;
    measurements->star3_body_z_micro = (int32_t)((int16_t)(telemetry_data[65] << 8 | telemetry_data[64])) * 100;
    measurements->star3_orbit_x_micro = (int32_t)((int16_t)(telemetry_data[67] << 8 | telemetry_data[66])) * 100;
    measurements->star3_orbit_y_micro = (int32_t)((int16_t)(telemetry_data[69] << 8 | telemetry_data[68])) * 100;
    measurements->star3_orbit_z_micro = (int32_t)((int16_t)(telemetry_data[71] << 8 | telemetry_data[70])) * 100;

    return 0;
}

/// @brief Parse File Information telemetry data into a struct.
/// @param[in] raw_data Pointer to the raw telemetry data buffer (12 bytes).
/// @param[out] file_info_struct Pointer to the struct to store parsed telemetry data.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_file_info_struct(uint8_t *raw_data, ADCS_file_info_struct_t *file_info_struct) {
    file_info_struct->file_type = raw_data[0] & 0x0F; // Bits 0-3
    file_info_struct->busy_updating = (raw_data[0] >> 4) & 0x01; // Bit 4
    file_info_struct->file_counter = raw_data[1]; // Byte 1

    file_info_struct->file_size = (raw_data[5] << 24) | (raw_data[4] << 16) | (raw_data[3] << 8) | raw_data[2]; // Bytes 2-5

    file_info_struct->file_date_time_msdos = (raw_data[9] << 24) | (raw_data[8] << 16) | (raw_data[7] << 8) | raw_data[6]; // Bytes 6-9

    file_info_struct->file_crc16 = (raw_data[11] << 8) | raw_data[10]; // Bytes 10-11
    return 0;
}

/// @brief Parse ACP Execution State telemetry data into a struct.
/// @param[in] data_received Pointer to the raw telemetry data buffer.
/// @param[out] output_struct Pointer to the struct to store parsed telemetry data.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_acp_execution_state_struct(uint8_t* data_received, ADCS_acp_execution_state_struct_t* output_struct) {
    output_struct->time_since_iteration_start_ms = (uint16_t)(data_received[1] << 8 | data_received[0]); 
    output_struct->current_execution_point = (ADCS_current_execution_point_enum_t) data_received[2];
    return 0;
}

/// @brief Parse Current State 1 telemetry data into a struct.
/// @param[in] data_received Pointer to the raw telemetry data buffer.
/// @param[out] output_struct Pointer to the struct to store parsed telemetry data.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_current_state_1_struct(uint8_t* data_received, ADCS_current_state_1_struct_t* output_struct) {
    
    output_struct->estimation_mode = (data_received[0]) & 0xf;
    
    output_struct->control_mode = (data_received[0] >> 4) & 0xf;

    output_struct->run_mode = (data_received[1]) & 0x3;  
    output_struct->asgp4_mode = (data_received[1] >> 2) & 0x3; 
    output_struct->cubecontrol_signal_enabled = (data_received[1] >> 4) & 0x01; // 1-bit bool
    output_struct->cubecontrol_motor_enabled = (data_received[1] >> 5) & 0x01; // 1-bit bool
    output_struct->cubesense1_enabled = (data_received[1] >> 6) & 0x01; // 1-bit bool
    output_struct->cubesense2_enabled = (data_received[1] >> 7) & 0x01; // 1-bit bool

    output_struct->cubewheel1_enabled = (data_received[2]) & 0x01; // 1-bit bool
    output_struct->cubewheel2_enabled = (data_received[2] >> 1) & 0x01; // 1-bit bool
    output_struct->cubewheel3_enabled = (data_received[2] >> 2) & 0x01; // 1-bit bool
    output_struct->cubestar_enabled = (data_received[2] >> 3) & 0x01; // 1-bit bool
    output_struct->gps_receiver_enabled = (data_received[2] >> 4) & 0x01; // 1-bit bool
    output_struct->gps_lna_power_enabled = (data_received[2] >> 5) & 0x01; // 1-bit bool
    output_struct->motor_driver_enabled = (data_received[2] >> 6) & 0x01; // 1-bit bool
    output_struct->sun_above_local_horizon = (data_received[2] >> 7) & 0x01; // 1-bit bool

    output_struct->cubesense1_comm_error = (data_received[3]) & 0x01; // 1-bit bool
    output_struct->cubesense2_comm_error = (data_received[3] >> 1) & 0x01; // 1-bit bool
    output_struct->cubecontrol_signal_comm_error = (data_received[3] >> 2) & 0x01; // 1-bit bool
    output_struct->cubecontrol_motor_comm_error = (data_received[3] >> 3) & 0x01; // 1-bit bool
    output_struct->cubewheel1_comm_error = (data_received[3] >> 4) & 0x01; // 1-bit bool
    output_struct->cubewheel2_comm_error = (data_received[3] >> 5) & 0x01; // 1-bit bool
    output_struct->cubewheel3_comm_error = (data_received[3] >> 6) & 0x01; // 1-bit bool
    output_struct->cubestar_comm_error = (data_received[3] >> 7) & 0x01; // 1-bit bool

    output_struct->magnetometer_range_error = (data_received[4]) & 0x01; // 1-bit bool
    output_struct->cam1_sram_overcurrent_detected = (data_received[4] >> 1) & 0x01; // 1-bit bool
    output_struct->cam1_3v3_overcurrent_detected = (data_received[4] >> 2) & 0x01; // 1-bit bool
    output_struct->cam1_sensor_busy_error = (data_received[4] >> 3) & 0x01; // 1-bit bool
    output_struct->cam1_sensor_detection_error = (data_received[4] >> 4) & 0x01; // 1-bit bool
    output_struct->sun_sensor_range_error = (data_received[4] >> 5) & 0x01; // 1-bit bool
    output_struct->cam2_sram_overcurrent_detected = (data_received[4] >> 6) & 0x01; // 1-bit bool
    output_struct->cam2_3v3_overcurrent_detected = (data_received[4] >> 7) & 0x01; // 1-bit bool

    output_struct->cam2_sensor_busy_error = (data_received[5]) & 0x01; // 1-bit bool
    output_struct->cam2_sensor_detection_error = (data_received[5] >> 1) & 0x01; // 1-bit bool
    output_struct->nadir_sensor_range_error = (data_received[5] >> 2) & 0x01; // 1-bit bool
    output_struct->rate_sensor_range_error = (data_received[5] >> 3) & 0x01; // 1-bit bool
    output_struct->wheel_speed_range_error = (data_received[5] >> 4) & 0x01; // 1-bit bool
    output_struct->coarse_sun_sensor_error = (data_received[5] >> 5) & 0x01; // 1-bit bool
    output_struct->startracker_match_error = (data_received[5] >> 6) & 0x01; // 1-bit bool
    output_struct->startracker_overcurrent_detected = (data_received[5] >> 7) & 0x01; // 1-bit bool

    return 0;
}

/// @brief Parse the Download Block Ready telemetry data into the provided struct.
/// @param[in] data_received Pointer to the telemetry data array.
/// @param[out] result Pointer to the struct to populate.
/// @return 0 once complete.
uint8_t ADCS_pack_to_download_block_ready_struct(const uint8_t *data_received, ADCS_download_block_ready_struct_t *result) {
    // Unpack Ready (1 bit) and ParameterError (1 bit) from the first byte
    result->ready = (data_received[0] & 0x01) != 0;               // Extract the 1st bit
    result->parameter_error = (data_received[0] & 0x02) != 0;    // Extract the 2nd bit

    // Unpack Block CRC16 (16 bits, reverse byte order)
    result->block_crc16 = (uint16_t)((data_received[2] << 8) | (data_received[1]));

    // Unpack Block Length (16 bits, reverse byte order)
    result->block_length = (uint16_t)((data_received[4] << 8) | (data_received[3]));

    return 0; 
}

/// @brief Parse the SD Card Format/Erase Progress data into the provided struct.
/// @param[in] data_received Pointer to the telemetry data array.
/// @param[out] result Pointer to the struct to populate.
/// @return 0 once complete.
uint8_t ADCS_pack_to_sd_card_format_erase_progress_struct(uint8_t *data_received, ADCS_sd_card_format_erase_progress_struct_t *result) {
    result->format_busy = (data_received[0] & 0x01) != 0; // First bit: Format Busy
    result->erase_all_busy = (data_received[0] & 0x02) != 0; // Second bit: Erase All Busy
    return 0;
}

/// @brief Parse the File Download Buffer data into the provided struct.
/// @param[in] data_received Pointer to the telemetry data array.
/// @param[out] result Pointer to the struct to populate.
/// @return 0 once complete.
uint8_t ADCS_pack_to_file_download_buffer_struct(uint8_t *data_received, ADCS_file_download_buffer_struct_t *result) {
    result->packet_counter = (uint16_t)data_received[0] | ((uint16_t)data_received[1] << 8);
    for (uint8_t i = 0; i < 20; i++) {
        result->file_bytes[i] = data_received[2+i];
    }
    return 0;
}

/// @brief Parse Raw Star Tracker telemetry data into a struct.
/// @param[in] input_data Pointer to the raw telemetry data buffer.
/// @param[out] output_data Pointer to the struct to store parsed telemetry data.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_raw_star_tracker_struct(uint8_t *input_data, ADCS_raw_star_tracker_struct_t *output_data) {
    
    output_data->num_stars_detected = input_data[0];
    output_data->star_image_noise = input_data[1];
    output_data->invalid_stars = input_data[2];
    output_data->num_stars_identified = input_data[3];
    output_data->identification_mode = input_data[4];
    output_data->image_dark_value = input_data[5];
    output_data->image_capture_success = input_data[6] & 0x01;
    output_data->detection_success = (input_data[6] >> 1) & 0x01;
    output_data->identification_success = (input_data[6] >> 2) & 0x01;
    output_data->attitude_success = (input_data[6] >> 3) & 0x01;
    output_data->processing_time_error = (input_data[6] >> 4) & 0x01;
    output_data->tracking_module_enabled = (input_data[6] >> 5) & 0x01;
    output_data->prediction_enabled = (input_data[6] >> 6) & 0x01;
    output_data->comms_error = (input_data[6] >> 7) & 0x01;
    output_data->sample_period = (input_data[8] << 8) | input_data[7];
    output_data->star1_confidence = input_data[9];
    output_data->star2_confidence = input_data[10];
    output_data->star3_confidence = input_data[11];
    output_data->magnitude_star1 = (input_data[13] << 8) | input_data[12];
    output_data->magnitude_star2 = (input_data[15] << 8) | input_data[14];
    output_data->magnitude_star3 = (input_data[17] << 8) | input_data[16];
    output_data->catalogue_star1 = (input_data[19] << 8) | input_data[18];
    output_data->centroid_x_star1 = (input_data[21] << 8) | input_data[20];
    output_data->centroid_y_star1 = (input_data[23] << 8) | input_data[22];
    output_data->catalogue_star2 = (input_data[25] << 8) | input_data[24];
    output_data->centroid_x_star2 = (input_data[27] << 8) | input_data[26];
    output_data->centroid_y_star2 = (input_data[29] << 8) | input_data[28];
    output_data->catalogue_star3 = (input_data[31] << 8) | input_data[30];
    output_data->centroid_x_star3 = (input_data[33] << 8) | input_data[32];
    output_data->centroid_y_star3 = (input_data[35] << 8) | input_data[34];
    output_data->capture_time_ms = (input_data[37] << 8) | input_data[36];
    output_data->detection_time_ms = (input_data[39] << 8) | input_data[38];
    output_data->identification_time_ms = (input_data[41] << 8) | input_data[40];
    output_data->x_axis_rate_micro = (int32_t) ((int16_t) ((input_data[43] << 8) | input_data[42])) * 100;
    output_data->y_axis_rate_micro = (int32_t) ((int16_t) ((input_data[45] << 8) | input_data[44])) * 100;
    output_data->z_axis_rate_micro = (int32_t) ((int16_t) ((input_data[47] << 8) | input_data[46])) * 100;
    output_data->q0_micro = (int32_t) ((int16_t) ((input_data[49] << 8) | input_data[48])) * 100;
    output_data->q1_micro = (int32_t) ((int16_t) ((input_data[51] << 8) | input_data[50])) * 100;
    output_data->q2_micro = (int32_t) ((int16_t) ((input_data[53] << 8) | input_data[52])) * 100;

    return 0;
}

/// @brief Parse Unix Time telemetry data into a uint64.
/// @param[in] data_received Pointer to the raw telemetry data buffer.
/// @param[out] output_data Pointer to the uint64 to store parsed telemetry data.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_unix_time_ms(uint8_t *data_received, uint64_t *output_data) {
    
                                                                                                                    // use ULL to perform unsigned, not signed, multiplication
    *output_data = ((data_received[3] << 24 | data_received[2] << 16 | data_received[1] << 8 | data_received[0]) * 1000ULL) + ((data_received[5] << 8) | data_received[4]);
    return 0;

}

/// @brief Parse SD Log Config telemetry data into a struct.
/// @param[in] data_received Pointer to the raw telemetry data buffer.
/// @param[in] which_log Which of the two concurrent logging task configurations is requested (1 or 2). 
/// @param[out] config Pointer to the struct to store parsed telemetry data.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_sd_log_config_struct(uint8_t *data_received, uint8_t which_log, ADCS_sd_log_config_struct_t *config) {
    
    for (uint8_t i = 0; i < 10; i++) {
        (config->log_bitmask)[i] = data_received[i];
    }
    config->which_log = which_log;
    config->log_period_s = (data_received[11] << 8) | data_received[10];
    config->which_sd = data_received[12];

    return 0;
}

/// @brief Parse Wheel Currents data into a struct.
/// @param[in] data_received Pointer to the raw telemetry data buffer.
/// @param[out] output Pointer to the struct to store parsed telemetry data.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_wheel_currents_struct(const uint8_t *data_received, ADCS_wheel_currents_struct_t *output) {

    uint16_t raw1 = (data_received[1] << 8) | data_received[0];
    uint16_t raw2 = (data_received[3] << 8) | data_received[2];
    uint16_t raw3 = (data_received[5] << 8) | data_received[4];

    output->wheel1_current_microamps = (uint32_t)raw1 * 10;
    output->wheel2_current_microamps = (uint32_t)raw2 * 10;
    output->wheel3_current_microamps = (uint32_t)raw3 * 10;

    return 0; 
}

/// @brief Unpacks CubeSense1 and CubeSense2 current measurements into a struct.
/// @param[in] input Pointer to 8-byte array (4 bytes for each CubeSense).
/// @param[out] output Pointer to the output struct.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_cubesense_currents_struct(const uint8_t *input, ADCS_cubesense_currents_struct_t *output) {

    // CubeSense1
    uint16_t raw_3v3_1  = (input[1] << 8) | input[0];
    uint16_t raw_sram_1 = (input[3] << 8) | input[2];

    // CubeSense2
    uint16_t raw_3v3_2  = (input[5] << 8) | input[4];
    uint16_t raw_sram_2 = (input[7] << 8) | input[6];

    output->cubesense1_3v3_current_microamps  = raw_3v3_1  * 100;
    output->cubesense1_sram_current_microamps = raw_sram_1 * 100;
    output->cubesense2_3v3_current_microamps  = raw_3v3_2  * 100;
    output->cubesense2_sram_current_microamps = raw_sram_2 * 100;

    return 0;
}

/// @brief Unpacks ADCS Misc Current Measurements from telemetry bytes.
/// @param[in] input Pointer to 6-byte input buffer.
/// @param[out] output Pointer to output struct.
/// @return 0 once the function is finished running.
uint8_t ADCS_pack_to_misc_currents_struct(const uint8_t *input, ADCS_misc_currents_struct_t *output) {

    uint16_t raw_cubestar = (input[1] << 8) | input[0];
    uint16_t raw_torquer  = (input[3] << 8) | input[2];
    int16_t  raw_temp     = (input[5] << 8) | input[4];

    output->cubestar_current_microamps = raw_cubestar * 10;   // mA * 1000 = µA
    output->torquer_current_microamps  = raw_torquer * 100;   // mA * 1000 = µA 
    output->cubestar_mcu_temperature_mdeg_celsius = raw_temp * 10; // centi°C * 10 = milli°C

    return 0;
}