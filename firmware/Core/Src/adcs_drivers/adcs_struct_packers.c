#include "adcs_drivers/adcs_types.h"
#include "adcs_drivers/adcs_command_ids.h"
#include "adcs_drivers/adcs_struct_packers.h"
#include "adcs_drivers/adcs_types_to_json.h"
#include "adcs_drivers/adcs_commands.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


/// @brief Packs the ADCS received raw data into the appropriate structure for the ADCS_CMD_Ack command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Ack_Struct(uint8_t* data_received, ADCS_CMD_Ack_Struct *result) {

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
uint8_t ADCS_Pack_to_Identification_Struct(uint8_t *data_received, ADCS_ID_Struct *result) {
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
uint8_t ADCS_Pack_to_Program_Status_Struct(uint8_t* data_received, ADCS_Boot_Running_Status_Struct *result) {
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
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Comms_Status_Struct(uint8_t *data_received, ADCS_Comms_Status_Struct *result) {
    result->cmd_counter = data_received[1] << 8 | data_received[0]; // uint16_t
    result->tlm_counter = data_received[3] << 8 | data_received[2]; // uint16_t
    result->cmd_buffer_overrun = data_received[4] & 128; // bit 0 is 1 if TC buffer was overrun while receiving a telecommand
    result->i2c_tlm_error = data_received[4] & 16; // bit 3 is 1 if the number of data clocked out was more than the telemetry package
    result->i2c_cmd_error = data_received[4] & 8; // bit 4 is 1 if the telecommand sent exceeded the buffer size
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Power_Control_Struct(uint8_t* data_received, ADCS_Power_Control_Struct* result) {

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
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Angular_Rates_Struct(uint8_t *data_received, ADCS_Angular_Rates_Struct *result) {
    // values given as int16, deg/s value is raw value * 0.01, give integer as m_deg/s
	// need to convert to int16 first, then double, to ensure negative numbers are represented correctly
    result->x_rate_milli_deg_per_sec = (int32_t) ((int16_t) (data_received[1] << 8 | data_received[0])) * 10;
    result->y_rate_milli_deg_per_sec = (int32_t) ((int16_t) (data_received[3] << 8 | data_received[2])) * 10;
    result->z_rate_milli_deg_per_sec = (int32_t) ((int16_t) (data_received[5] << 8 | data_received[4])) * 10;
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_LLH_Position_Struct(uint8_t *data_received, ADCS_LLH_Position_Struct *result) {
    // formatted value (deg or km) = raw value * 0.01
	// need to convert to int16 first, then double, to ensure negative numbers are represented correctly
	result->latitude_milli_deg = (int32_t) ((int16_t) (data_received[1] << 8 | data_received[0])) * 10; 
    result->longitude_milli_deg = (int32_t) ((int16_t) (data_received[3] << 8 | data_received[2])) * 10; 
    result->altitude_meters  = (int32_t) ((int16_t) (data_received[5] << 8 | data_received[4])) * 10; 
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Unix_Time_Save_Mode_Struct(uint8_t *data_received, ADCS_Set_Unix_Time_Save_Mode_Struct *result) {
    result->save_now = data_received[0] & 1; // 0b00000001
    result->save_on_update = (data_received[0] & 2) >> 1; // 0b00000010
    result->save_periodic = (data_received[0] & 4) >> 2; // 0b00000100
    result->period = data_received[1];
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Orbit_Params_Struct(uint8_t *data_received, ADCS_Orbit_Params_Struct *result) {
    memcpy(&result->inclination, &data_received[0], sizeof(double));
    memcpy(&result->eccentricity, &data_received[8], sizeof(double));
    memcpy(&result->ascending_node_right_ascension, &data_received[16], sizeof(double));
    memcpy(&result->perigee_argument, &data_received[24], sizeof(double));
    memcpy(&result->b_star_drag_term, &data_received[32], sizeof(double));
    memcpy(&result->mean_motion, &data_received[40], sizeof(double));
    memcpy(&result->mean_anomaly, &data_received[48], sizeof(double));
    memcpy(&result->epoch, &data_received[56], sizeof(double));
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Rated_Sensor_Rates_Struct(uint8_t *data_received, ADCS_Rated_Sensor_Rates_Struct *result) {
    // formatted value (deg/s) = raw value * 0.01
	// need to convert to int16 first, then double, to ensure negative numbers are represented correctly
	result->x_milli_deg_per_sec = (int32_t) ((int16_t) (data_received[1] << 8 | data_received[0])) * 10; 
    result->y_milli_deg_per_sec = (int32_t) ((int16_t) (data_received[3] << 8 | data_received[2])) * 10; 
    result->z_milli_deg_per_sec = (int32_t) ((int16_t) (data_received[5] << 8 | data_received[4])) * 10; 
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Wheel_Speed_Struct(uint8_t *data_received, ADCS_Wheel_Speed_Struct *result) {
	// all values in rpm
    result->x = data_received[1] << 8 | data_received[0];
    result->y = data_received[3] << 8 | data_received[2];
    result->z = data_received[5] << 8 | data_received[4];
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Magnetorquer_Command_Struct(uint8_t *data_received, ADCS_Magnetorquer_Command_Struct *result) {
    // formatted value (sec) = raw value * 0.01
	result->x_ms = (int32_t) ((int16_t) (data_received[1] << 8 | data_received[0])) * 10;
    result->y_ms = (int32_t) ((int16_t) (data_received[3] << 8 | data_received[2])) * 10;
    result->z_ms = (int32_t) ((int16_t) (data_received[5] << 8 | data_received[4])) * 10;
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Raw_Magnetometer_Values_Struct(uint8_t *data_received, ADCS_Raw_Magnetometer_Values_Struct *result) {
	result->x = data_received[1] << 8 | data_received[0];
    result->y = data_received[3] << 8 | data_received[2];
    result->z = data_received[5] << 8 | data_received[4];
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Fine_Angular_Rates_Struct(uint8_t *data_received, ADCS_Fine_Angular_Rates_Struct *result) {
    // formatted value (deg/s) = raw value * 0.001
	result->x_milli_deg_per_sec = (int16_t) (data_received[1] << 8 | data_received[0]);
    result->y_milli_deg_per_sec = (int16_t) (data_received[3] << 8 | data_received[2]);
    result->z_milli_deg_per_sec = (int16_t) (data_received[5] << 8 | data_received[4]);
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Magnetometer_Config_Struct(uint8_t *data_received, ADCS_Magnetometer_Config_Struct *result) {
    // formatted value for mounting transform angles (deg/s) = raw value * 0.01
	result->mounting_transform_alpha_angle_milli_deg_per_sec = (int32_t) ((int16_t) (data_received[1] << 8 | data_received[0])) * 10;
    result->mounting_transform_beta_angle_milli_deg_per_sec =  (int32_t) ((int16_t) (data_received[3] << 8 | data_received[2])) * 10;
    result->mounting_transform_gamma_angle_milli_deg_per_sec = (int32_t) ((int16_t) (data_received[5] << 8 | data_received[4])) * 10;
    // formatted value (deg/s) = raw value * 0.001
	result->channel_1_offset_milli_deg_per_sec = (int16_t) (data_received[7] << 8 | data_received[6]);
    result->channel_2_offset_milli_deg_per_sec = (int16_t) (data_received[9] << 8 | data_received[8]);
    result->channel_3_offset_milli_deg_per_sec = (int16_t) (data_received[11] << 8 | data_received[10]);
    result->sensitivity_matrix_s11_milli_deg_per_sec = (int16_t) (data_received[13] << 8 | data_received[12]);
    result->sensitivity_matrix_s22_milli_deg_per_sec = (int16_t) (data_received[15] << 8 | data_received[14]);
    result->sensitivity_matrix_s33_milli_deg_per_sec = (int16_t) (data_received[17] << 8 | data_received[16]);
    result->sensitivity_matrix_s12_milli_deg_per_sec = (int16_t) (data_received[19] << 8 | data_received[18]);
    result->sensitivity_matrix_s13_milli_deg_per_sec = (int16_t) (data_received[21] << 8 | data_received[20]);
    result->sensitivity_matrix_s21_milli_deg_per_sec = (int16_t) (data_received[23] << 8 | data_received[22]);
    result->sensitivity_matrix_s23_milli_deg_per_sec = (int16_t) (data_received[25] << 8 | data_received[24]);
    result->sensitivity_matrix_s31_milli_deg_per_sec = (int16_t) (data_received[27] << 8 | data_received[26]);
    result->sensitivity_matrix_s32_milli_deg_per_sec = (int16_t) (data_received[29] << 8 | data_received[28]);
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Commanded_Attitude_Angles_Struct(uint8_t *data_received, ADCS_Commanded_Angles_Struct *result) {
    // Formatted value is obtained using the formula: (formatted value) [deg] = RAWVAL*0.01
	result->x_milli_deg = ((int16_t)(data_received[1] << 8 | data_received[0])) * 10;
    result->y_milli_deg = ((int16_t)(data_received[3] << 8 | data_received[2])) * 10;
    result->z_milli_deg = ((int16_t)(data_received[5] << 8 | data_received[4])) * 10;
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Estimation_Params_Struct(uint8_t* data_received, ADCS_Estimation_Params_Struct* result) {
    // map temp buffer to struct
    memcpy(&result->magnetometer_rate_filter_system_noise, &data_received[0], 4);
    memcpy(&result->ekf_system_noise, &data_received[4], 4);
    memcpy(&result->css_measurement_noise, &data_received[8], 4);
    memcpy(&result->sun_sensor_measurement_noise, &data_received[12], 4);
    memcpy(&result->nadir_sensor_measurement_noise, &data_received[16], 4);
    memcpy(&result->magnetometer_measurement_noise, &data_received[20], 4);
    memcpy(&result->star_tracker_measurement_noise, &data_received[24], 4);
    result->use_sun_sensor = (data_received[28] & 1); // 0b00000001 
    result->use_nadir_sensor = (data_received[28] & 2) >> 1; // 0b00000010
    result->use_css = (data_received[28] & 4) >> 2; // 0b00000100
    result->use_star_tracker = (data_received[28] & 8) >> 3; // 0b00001000
    result->nadir_sensor_terminator_test = (data_received[28] & 16) >> 4; // 0b00010000
    result->automatic_magnetometer_recovery = (data_received[28] & 32) >> 5; // 0b00100000
    result->magnetometer_mode = (data_received[28] & 192) >> 6; // 0b11000000
    result->magnetometer_selection_for_raw_mtm_tlm = (data_received[29] & 3); // 0b00000011
    result->automatic_estimation_transition_due_to_rate_sensor_errors = (data_received[29] & 4) >> 2; // 0b00000100
    result->wheel_30s_power_up_delay = (data_received[29] & 8) >> 3; // 0b00001000
    result->cam1_and_cam2_sampling_period = data_received[30];

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_ASGP4_Params_Struct(uint8_t* data_received, ADCS_ASGP4_Params_Struct* result) {
    // map temp buffer to struct
    result->incl_coefficient_milli = (int16_t) (data_received[1] << 8 | data_received[0]);
    result->raan_coefficient_milli = (int16_t) (data_received[3] << 8 | data_received[2]);
    result->ecc_coefficient_milli = (int16_t) (data_received[5] << 8 | data_received[4]);
    result->aop_coefficient_milli = (int16_t) (data_received[7] << 8 | data_received[6]);
    result->time_coefficient_milli = (int16_t) (data_received[9] << 8 | data_received[8]);
    result->pos_coefficient_milli = (int16_t) (data_received[11] << 8 | data_received[10]);
    result->maximum_position_error_milli = ((int32_t)((int16_t)data_received[12])) * 100;
    result->asgp4_filter = (ADCS_ASGP4_Filter)data_received[13];
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
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Tracking_Controller_Target_Reference_Struct(uint8_t* data_received, ADCS_Tracking_Controller_Target_Struct* ref) {
    // map temp buffer to struct
    memcpy(&ref->lon, &data_received[0], 4);
    memcpy(&ref->lat, &data_received[4], 4);
    memcpy(&ref->alt, &data_received[8], 4);

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Rate_Gyro_Config_Struct(uint8_t* data_received, ADCS_Rate_Gyro_Config_Struct* result) {
    result->gyro1 = data_received[0];
    result->gyro2 = data_received[1];
    result->gyro3 = data_received[2];

	// Raw parameter value is obtained using the formula: (formatted value) [deg/s] = RAWVAL*0.001
    result->x_rate_offset_milli_deg_per_sec = (int16_t) (data_received[4] << 8 | data_received[3]);
    result->y_rate_offset_milli_deg_per_sec = (int16_t) (data_received[6] << 8 | data_received[5]);
    result->z_rate_offset_milli_deg_per_sec = (int16_t) (data_received[8] << 8 | data_received[7]);

    result->rate_sensor_mult = data_received[9];

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Estimated_Attitude_Angles_Struct(uint8_t *data_received, ADCS_Estimated_Attitude_Angles_Struct *angles) {
    angles->estimated_roll_angle_milli_deg = (int32_t) ((int16_t) (data_received[1] << 8 | data_received[0])) * 10;
    angles->estimated_pitch_angle_milli_deg = (int32_t) ((int16_t) (data_received[3] << 8 | data_received[2])) * 10;
    angles->estimated_yaw_angle_milli_deg = (int32_t) ((int16_t) (data_received[5] << 8 | data_received[4])) * 10;
	return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Magnetic_Field_Vector_Struct(uint8_t *data_received, ADCS_Magnetic_Field_Vector_Struct *vector_components) {
	// gives vector components in nT (10^-9 Teslas)
    vector_components->x_nT = (int32_t) ((int16_t) (data_received[1] << 8 | data_received[0])) * 10;
    vector_components->y_nT = (int32_t) ((int16_t) (data_received[3] << 8 | data_received[2])) * 10;
    vector_components->z_nT = (int32_t) ((int16_t) (data_received[5] << 8 | data_received[4])) * 10;
	return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Fine_Sun_Vector_Struct(uint8_t *data_received, ADCS_Fine_Sun_Vector_Struct *vector_components) {
    vector_components->x_micro = ((int32_t) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 100;
    vector_components->y_micro = ((int32_t) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 100;
    vector_components->z_micro = ((int32_t) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 100;
	return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Nadir_Vector_Struct(uint8_t *data_received, ADCS_Nadir_Vector_Struct *vector_components) {
    vector_components->x_micro = ((int32_t) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 100;
    vector_components->y_micro = ((int32_t) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 100;
    vector_components->z_micro = ((int32_t) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 100;
	return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Commanded_Wheel_Speed_Struct(uint8_t *data_received, ADCS_Wheel_Speed_Struct *result) {
	// all values in rpm
    result->x = data_received[1] << 8 | data_received[0];
    result->y = data_received[3] << 8 | data_received[2];
    result->z = data_received[5] << 8 | data_received[4];
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_IGRF_Magnetic_Field_Vector_Struct(uint8_t *data_received, ADCS_Magnetic_Field_Vector_Struct *vector_components) {
	// gives vector components in nT (10^-9 Teslas)
    vector_components->x_nT = (int32_t) ((int16_t) (data_received[1] << 8 | data_received[0])) * 10;
    vector_components->y_nT = (int32_t) ((int16_t) (data_received[3] << 8 | data_received[2])) * 10;
    vector_components->z_nT = (int32_t) ((int16_t) (data_received[5] << 8 | data_received[4])) * 10;
	return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Quaternion_Error_Vector_Struct(uint8_t *data_received, ADCS_Quaternion_Error_Vector_Struct *result) {
    result->quaternion_error_q1_micro = ((int32_t) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 100;
    result->quaternion_error_q2_micro = ((int32_t) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 100;
    result->quaternion_error_q3_micro = ((int32_t) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 100;

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Estimated_Gyro_Bias_Struct(uint8_t* data_received, ADCS_Estimated_Gyro_Bias_Struct *result) {
    result->estimated_x_gyro_bias_milli_deg_per_sec = (int32_t) ((int16_t) (data_received[1] << 8 | data_received[0]));
    result->estimated_y_gyro_bias_milli_deg_per_sec = (int32_t) ((int16_t) (data_received[3] << 8 | data_received[2]));
    result->estimated_z_gyro_bias_milli_deg_per_sec = (int32_t) ((int16_t) (data_received[5] << 8 | data_received[4]));

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Estimation_Innovation_Vector_Struct(uint8_t* data_received, ADCS_Estimation_Innovation_Vector_Struct* result) {
    result->innovation_vector_x_micro = ((int32_t) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 100;
    result->innovation_vector_y_micro = ((int32_t) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 100;
    result->innovation_vector_z_micro = ((int32_t) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 100;

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Raw_Cam1_Sensor_Struct(uint8_t* data_received, ADCS_Raw_Cam_Sensor_Struct* result) {
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
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Raw_Cam2_Sensor_Struct(uint8_t* data_received, ADCS_Raw_Cam_Sensor_Struct* result) {
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
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Raw_CSS_1_to_6_Struct(uint8_t* data_received, ADCS_Raw_CSS_1_to_6_Struct* result) {
    result->css1 = data_received[0];
    result->css2 = data_received[1];
    result->css3 = data_received[2];
    result->css4 = data_received[3];
    result->css5 = data_received[4];
    result->css6 = data_received[5];

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Raw_CSS_7_to_10_Struct(uint8_t* data_received, ADCS_Raw_CSS_7_to_10_Struct* result) {
    result->css7 = data_received[0];
    result->css8 = data_received[1];
    result->css9 = data_received[2];
    result->css10 = data_received[3];

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_CubeControl_Current_Struct(uint8_t* data_received, ADCS_CubeControl_Current_Struct* result) {
    // everything in mA after multiplying RAWVAL*0.48828125 (aka dividing by 2.048 exactly)
	result->cubecontrol_3v3_current_mA = (( double ) ((uint16_t) (data_received[1] << 8 | data_received[0]))) / 2.048;
    result->cubecontrol_5v_current_mA = (( double ) ((uint16_t) (data_received[3] << 8 | data_received[2]))) / 2.048;
    result->cubecontrol_vbat_current_mA = (( double ) ((uint16_t) (data_received[5] << 8 | data_received[4]))) / 2.048;

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Raw_GPS_Status_Struct(uint8_t* data_received, ADCS_Raw_GPS_Status_Struct* result) {
    result->gps_solution_status = (ADCS_GPS_Solution_Status) data_received[0];
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
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Raw_GPS_Time_Struct(uint8_t* data_received, ADCS_Raw_GPS_Time_Struct* result) {
    result->gps_reference_week = (uint16_t)(data_received[1] << 8 | data_received[0]);
    result->gps_time = ((uint32_t)(data_received[5] << 24 | data_received[4] << 16 | data_received[3] << 8 | data_received[2])) / 1000.0; // Convert milliseconds to seconds

    return 0; 
}

/// @brief Packs the ADCS received raw data into the appropriate structure for any of the three Raw_GPS commands (X, Y, Z).
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Raw_GPS_Struct(ADCS_GPS_Axis axis, uint8_t *data_received, ADCS_Raw_GPS_Struct *result) {
    result->axis = axis; // this function works for three commands, so we need to keep this information
    result->ecef_position_meters = (int32_t) (data_received[3] << 24 | data_received[2] << 16 | 
                                               data_received[1] << 8 | data_received[0]); // ECEF Position Z [m]
	result->ecef_velocity_meters_per_sec = (int16_t)(data_received[5] << 8 | data_received[4]);  // ECEF Velocity Z [m/s]

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Measurements_Struct(uint8_t* telemetry_data, ADCS_Measurements_Struct* measurements) {

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
    measurements->x_angular_rate_milli_deg_per_sec = (int32_t)((int16_t)(telemetry_data[25] << 8 | telemetry_data[24])) * 10;
    measurements->y_angular_rate_milli_deg_per_sec = (int32_t)((int16_t)(telemetry_data[27] << 8 | telemetry_data[26])) * 10;
    measurements->z_angular_rate_milli_deg_per_sec = (int32_t)((int16_t)(telemetry_data[29] << 8 | telemetry_data[28])) * 10;
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