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
uint8_t ADCS_Pack_to_Ack(uint8_t* data_received, ADCS_CMD_Ack_Struct *result) {

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
uint8_t ADCS_Pack_to_Identification(uint8_t *data_received, ADCS_ID_Struct *result) {
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
uint8_t ADCS_Pack_to_Program_Status(uint8_t* data_received, ADCS_Boot_Running_Status_Struct *result) {
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
uint8_t ADCS_Pack_to_Comms_Status(uint8_t *data_received, ADCS_Comms_Status_Struct *result) {
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
uint8_t ADCS_Pack_to_Power_Control(uint8_t* data_received, ADCS_Power_Control_Struct* result) {

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
uint8_t ADCS_Pack_to_Angular_Rates(uint8_t *data_received, ADCS_Angular_Rates_Struct *result) {
	// need to convert to int16 first, then double, to ensure negative numbers are represented correctly
    result->x_rate = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 0.01;
    result->y_rate = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 0.01;
    result->z_rate = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 0.01;
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_LLH_Position(uint8_t *data_received, ADCS_LLH_Position_Struct *result) {
    // formatted value (deg or km) = raw value * 0.01
	// need to convert to int16 first, then double, to ensure negative numbers are represented correctly
	result->latitude  = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 0.01; 
    result->longitude = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 0.01; 
    result->altitude  = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 0.01; 
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Unix_Time_Save_Mode(uint8_t *data_received, ADCS_Set_Unix_Time_Save_Mode_Struct *result) {
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
uint8_t ADCS_Pack_to_Orbit_Params(uint8_t *data_received, ADCS_Orbit_Params_Struct *result) {
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
uint8_t ADCS_Pack_to_Rated_Sensor_Rates(uint8_t *data_received, ADCS_Rated_Sensor_Rates_Struct *result) {
    // formatted value (deg/s) = raw value * 0.01
	// need to convert to int16 first, then double, to ensure negative numbers are represented correctly
	result->x  = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 0.01; 
    result->y = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 0.01; 
    result->z  = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 0.01; 
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Wheel_Speed(uint8_t *data_received, ADCS_Wheel_Speed_Struct *result) {
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
uint8_t ADCS_Pack_to_Magnetorquer_Command(uint8_t *data_received, ADCS_Magnetorquer_Command_Struct *result) {
    // formatted value (sec) = raw value * 0.01
	result->x = ((double)((int16_t)(data_received[1] << 8 | data_received[0]))) * 0.01;
    result->y = ((double)((int16_t)(data_received[3] << 8 | data_received[2]))) * 0.01;
    result->z = ((double)((int16_t)(data_received[5] << 8 | data_received[4]))) * 0.01;
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Raw_Magnetometer_Values(uint8_t *data_received, ADCS_Raw_Mag_TLM_Struct *result) {
	result->x = data_received[1] << 8 | data_received[0];
    result->y = data_received[3] << 8 | data_received[2];
    result->z = data_received[5] << 8 | data_received[4];
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Fine_Angular_Rates(uint8_t *data_received, ADCS_Fine_Angular_Rates_Struct *result) {
    // formatted value (deg/s) = raw value * 0.001
	result->x = ((double)((int16_t)(data_received[1] << 8 | data_received[0]))) * 0.001;
    result->y = ((double)((int16_t)(data_received[3] << 8 | data_received[2]))) * 0.001;
    result->z = ((double)((int16_t)(data_received[5] << 8 | data_received[4]))) * 0.001;
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Magnetometer_Config(uint8_t *data_received, ADCS_Magnetometer_Config_Struct *result) {
    // formatted value for mounting transform angles (deg/s) = raw value * 0.01
	result->mounting_transform_alpha_angle = ((double)((int16_t)(data_received[1] << 8 | data_received[0]))) * 0.01;
    result->mounting_transform_beta_angle = ((double)((int16_t)(data_received[3] << 8 | data_received[2]))) * 0.01;
    result->mounting_transform_gamma_angle = ((double)((int16_t)(data_received[5] << 8 | data_received[4]))) * 0.01;
    // formatted value (deg/s) = raw value * 0.001
	result->channel_1_offset = ((double)((int16_t)(data_received[7] << 8 | data_received[6]))) * 0.001;
    result->channel_2_offset = ((double)((int16_t)(data_received[9] << 8 | data_received[8]))) * 0.001;
    result->channel_3_offset = ((double)((int16_t)(data_received[11] << 8 | data_received[10]))) * 0.001;
    result->sensitivity_matrix_s11 = ((double)((int16_t)(data_received[13] << 8 | data_received[12]))) * 0.001;
    result->sensitivity_matrix_s22 = ((double)((int16_t)(data_received[15] << 8 | data_received[14]))) * 0.001;
    result->sensitivity_matrix_s33 = ((double)((int16_t)(data_received[17] << 8 | data_received[16]))) * 0.001;
    result->sensitivity_matrix_s12 = ((double)((int16_t)(data_received[19] << 8 | data_received[18]))) * 0.001;
    result->sensitivity_matrix_s13 = ((double)((int16_t)(data_received[21] << 8 | data_received[20]))) * 0.001;
    result->sensitivity_matrix_s21 = ((double)((int16_t)(data_received[23] << 8 | data_received[22]))) * 0.001;
    result->sensitivity_matrix_s23 = ((double)((int16_t)(data_received[25] << 8 | data_received[24]))) * 0.001;
    result->sensitivity_matrix_s31 = ((double)((int16_t)(data_received[27] << 8 | data_received[26]))) * 0.001;
    result->sensitivity_matrix_s32 = ((double)((int16_t)(data_received[29] << 8 | data_received[28]))) * 0.001;
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Commanded_Attitude_Angles(uint8_t *data_received, ADCS_Commanded_Angles_Struct *result) {
    // Formatted value is obtained using the formula: (formatted value) [deg] = RAWVAL*0.01
	result->x = (double)((int16_t)(data_received[1] << 8 | data_received[0])) * 0.01;
    result->y = (double)((int16_t)(data_received[3] << 8 | data_received[2])) * 0.01;
    result->z = (double)((int16_t)(data_received[5] << 8 | data_received[4])) * 0.01;
    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Estimation_Params(uint8_t* data_received, ADCS_Estimation_Params_Struct* result) {
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
uint8_t ADCS_Pack_to_ASGP4_Params(uint8_t* data_received, ADCS_ASGP4_Params_Struct* result) {
    // map temp buffer to struct
    result->incl_coefficient = ((double)((int16_t)(data_received[1] << 8 | data_received[0]))) * 0.001;
    result->raan_coefficient = ((double)((int16_t)(data_received[3] << 8 | data_received[2]))) * 0.001;
    result->ecc_coefficient = ((double)((int16_t)(data_received[5] << 8 | data_received[4]))) * 0.001;
    result->aop_coefficient = ((double)((int16_t)(data_received[7] << 8 | data_received[6]))) * 0.001;
    result->time_coefficient = ((double)((int16_t)(data_received[9] << 8 | data_received[8]))) * 0.001;
    result->pos_coefficient = ((double)((int16_t)(data_received[11] << 8 | data_received[10]))) * 0.001;
    result->maximum_position_error = ((double)((int16_t)data_received[12])) * 0.1;
    result->asgp4_filter = (ADCS_ASGP4_Filter)data_received[13];
    result->xp_coefficient = ((double)((int32_t)(data_received[17] << 24 | data_received[16] << 16 | data_received[15] << 8 | data_received[14]))) * 0.0000001;
    result->yp_coefficient = ((double)((int32_t)(data_received[21] << 24 | data_received[20] << 16 | data_received[19] << 8 | data_received[18]))) * 0.0000001;
    result->gps_roll_over = data_received[22];
    result->position_sd = ((double)((int16_t)data_received[23])) * 0.1;
    result->velocity_sd = ((double)((int16_t)data_received[24])) * 0.01;
    result->min_satellites = data_received[25];
    result->time_gain = ((double)((int16_t)data_received[26])) * 0.01;
    result->max_lag = ((double)((int16_t)data_received[27])) * 0.01;
    result->min_samples = (data_received[29] << 8 | data_received[28]);

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Tracking_Controller_Target_Reference(uint8_t* data_received, ADCS_Tracking_Controller_Target_Struct* ref) {
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
uint8_t ADCS_Pack_to_Rate_Gyro_Config(uint8_t* data_received, ADCS_Rate_Gyro_Config_Struct* result) {
    result->gyro1 = data_received[0];
    result->gyro2 = data_received[1];
    result->gyro3 = data_received[2];

	// Raw parameter value is obtained using the formula: (formatted value) [deg/s] = RAWVAL*0.001
    result->x_rate_offset = ((double)((int16_t)(data_received[4] << 8 | data_received[3]))) * 0.001;
    result->y_rate_offset = ((double)((int16_t)(data_received[6] << 8 | data_received[5]))) * 0.001;
    result->z_rate_offset = ((double)((int16_t)(data_received[8] << 8 | data_received[7]))) * 0.001;

    result->rate_sensor_mult = data_received[9];

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Estimated_Attitude_Angles(uint8_t *data_received, ADCS_Estimated_Attitude_Angles_Struct *angles) {
    angles->estimated_roll_angle = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 0.01;
    angles->estimated_pitch_angle = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 0.01;
    angles->estimated_yaw_angle = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 0.01;
	return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Magnetic_Field_Vector(uint8_t *data_received, ADCS_Magnetic_Field_Vector_Struct *vector_components) {
	// gives vector components in uT (10^-6 Teslas)
    vector_components->x = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 0.01;
    vector_components->y = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 0.01;
    vector_components->z = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 0.01;
	return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Fine_Sun_Vector(uint8_t *data_received, ADCS_Fine_Sun_Vector_Struct *vector_components) {
    vector_components->x = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) / 10000.0;
    vector_components->y = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) / 10000.0;
    vector_components->z = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) / 10000.0;
	return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Nadir_Vector(uint8_t *data_received, ADCS_Nadir_Vector_Struct *vector_components) {
    vector_components->x = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) / 10000.0;
    vector_components->y = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) / 10000.0;
    vector_components->z = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) / 10000.0;
	return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Commanded_Wheel_Speed(uint8_t *data_received, ADCS_Wheel_Speed_Struct *result) {
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
uint8_t ADCS_Pack_to_IGRF_Magnetic_Field_Vector(uint8_t *data_received, ADCS_Magnetic_Field_Vector_Struct *vector_components) {
	// gives vector components in uT (10^-6 Teslas)
    vector_components->x = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 0.01;
    vector_components->y = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 0.01;
    vector_components->z = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 0.01;
	return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Quaternion_Error_Vector(uint8_t *data_received, ADCS_Quaternion_Error_Vector_Struct *result) {
    result->quaternion_error_q1 = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 0.0001;
    result->quaternion_error_q2 = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 0.0001;
    result->quaternion_error_q3 = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 0.0001;

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Estimated_Gyro_Bias(uint8_t* data_received, ADCS_Estimated_Gyro_Bias_Struct *result) {
    result->estimated_x_gyro_bias = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 0.001;
    result->estimated_y_gyro_bias = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 0.001;
    result->estimated_z_gyro_bias = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 0.001;

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Estimation_Innovation_Vector(uint8_t* data_received, ADCS_Estimation_Innovation_Vector_Struct* result) {
    result->innovation_vector_x = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 0.0001;
    result->innovation_vector_y = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 0.0001;
    result->innovation_vector_z = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 0.0001;

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Raw_Cam1_Sensor(uint8_t* data_received, ADCS_Raw_Cam_Sensor_Struct* result) {
    result->which_sensor = ADCS_CAM1_SENSOR;
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
uint8_t ADCS_Pack_to_Raw_Cam2_Sensor(uint8_t* data_received, ADCS_Raw_Cam_Sensor_Struct* result) {
    result->which_sensor = ADCS_CAM2_SENSOR;
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
uint8_t ADCS_Pack_to_Raw_CSS_1_to_6(uint8_t* data_received, ADCS_Raw_CSS_1_to_6_Struct* result) {
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
uint8_t ADCS_Pack_to_Raw_CSS_7_to_10(uint8_t* data_received, ADCS_Raw_CSS_7_to_10_Struct* result) {
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
uint8_t ADCS_Pack_to_CubeControl_Current(uint8_t* data_received, ADCS_CubeControl_Current_Struct* result) {
    // everything in mA after multiplying RAWVAL*0.48828125 (aka dividing by 2.048 exactly)
	result->cubecontrol_3v3_current = ((double) ((uint16_t) (data_received[1] << 8 | data_received[0]))) / 2.048;
    result->cubecontrol_5v_current = ((double) ((uint16_t) (data_received[3] << 8 | data_received[2]))) / 2.048;
    result->cubecontrol_vbat_current = ((double) ((uint16_t) (data_received[5] << 8 | data_received[4]))) / 2.048;

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Raw_GPS_Status(uint8_t* data_received, ADCS_Raw_GPS_Status_Struct* result) {
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
uint8_t ADCS_Pack_to_Raw_GPS_Time(uint8_t* data_received, ADCS_Raw_GPS_Time_Struct* result) {
    result->gps_reference_week = (uint16_t)(data_received[1] << 8 | data_received[0]);
    result->gps_time = ((uint32_t)(data_received[5] << 24 | data_received[4] << 16 | data_received[3] << 8 | data_received[2])) / 1000.0; // Convert milliseconds to seconds

    return 0; 
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Raw_GPS(ADCS_GPS_Axis axis, uint8_t *data_received, ADCS_Raw_GPS_Struct *result) {
    result->axis = axis;
    result->ecef_position = (int32_t)(data_received[3] << 24 | data_received[2] << 16 | 
                                               data_received[1] << 8 | data_received[0]); // ECEF Position Z [m]
	result->ecef_velocity = (int16_t)(data_received[5] << 8 | data_received[4]);  // ECEF Velocity Z [m/s]

    return 0;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.}
uint8_t ADCS_Pack_to_Measurements(uint8_t* telemetry_data, ADCS_Measurements_Struct* measurements) {
    // Constants for conversion factors
    const double MAGNETIC_FIELD_FACTOR = 0.01;
    const double COARSE_SUN_FACTOR = 1.0 / 10000.0;
    const double ANGULAR_RATE_FACTOR = 0.01;
    const double WHEEL_SPEED_FACTOR = 1.0;
    const double VECTOR_FACTOR = 1.0 / 10000.0;

    // Parse each telemetry entry according to Table 150 in the Firmware Reference Manual
    measurements->magnetic_field_x = (double)((int16_t)(telemetry_data[1] << 8 | telemetry_data[0])) * MAGNETIC_FIELD_FACTOR;
    measurements->magnetic_field_y = (double)((int16_t)(telemetry_data[3] << 8 | telemetry_data[2])) * MAGNETIC_FIELD_FACTOR;
    measurements->magnetic_field_z = (double)((int16_t)(telemetry_data[5] << 8 | telemetry_data[4])) * MAGNETIC_FIELD_FACTOR;
    measurements->coarse_sun_x = (double)((int16_t)(telemetry_data[7] << 8 | telemetry_data[6])) * COARSE_SUN_FACTOR;
    measurements->coarse_sun_y = (double)((int16_t)(telemetry_data[9] << 8 | telemetry_data[8])) * COARSE_SUN_FACTOR;
    measurements->coarse_sun_z = (double)((int16_t)(telemetry_data[11] << 8 | telemetry_data[10])) * COARSE_SUN_FACTOR;
    measurements->sun_x = (double)((int16_t)(telemetry_data[13] << 8 | telemetry_data[12])) * COARSE_SUN_FACTOR;
    measurements->sun_y = (double)((int16_t)(telemetry_data[15] << 8 | telemetry_data[14])) * COARSE_SUN_FACTOR;
    measurements->sun_z = (double)((int16_t)(telemetry_data[17] << 8 | telemetry_data[16])) * COARSE_SUN_FACTOR;
    measurements->nadir_x = (double)((int16_t)(telemetry_data[19] << 8 | telemetry_data[18])) * COARSE_SUN_FACTOR;
    measurements->nadir_y = (double)((int16_t)(telemetry_data[21] << 8 | telemetry_data[20])) * COARSE_SUN_FACTOR;
    measurements->nadir_z = (double)((int16_t)(telemetry_data[23] << 8 | telemetry_data[22])) * COARSE_SUN_FACTOR;
    measurements->x_angular_rate = (double)((int16_t)(telemetry_data[25] << 8 | telemetry_data[24])) * ANGULAR_RATE_FACTOR;
    measurements->y_angular_rate = (double)((int16_t)(telemetry_data[27] << 8 | telemetry_data[26])) * ANGULAR_RATE_FACTOR;
    measurements->z_angular_rate = (double)((int16_t)(telemetry_data[29] << 8 | telemetry_data[28])) * ANGULAR_RATE_FACTOR;
    measurements->x_wheel_speed = (double)((int16_t)(telemetry_data[31] << 8 | telemetry_data[30])) * WHEEL_SPEED_FACTOR;
    measurements->y_wheel_speed = (double)((int16_t)(telemetry_data[33] << 8 | telemetry_data[32])) * WHEEL_SPEED_FACTOR;
    measurements->z_wheel_speed = (double)((int16_t)(telemetry_data[35] << 8 | telemetry_data[34])) * WHEEL_SPEED_FACTOR;
    measurements->star1_body_x = (double)((int16_t)(telemetry_data[37] << 8 | telemetry_data[36])) * VECTOR_FACTOR;
    measurements->star1_body_y = (double)((int16_t)(telemetry_data[39] << 8 | telemetry_data[38])) * VECTOR_FACTOR;
    measurements->star1_body_z = (double)((int16_t)(telemetry_data[41] << 8 | telemetry_data[40])) * VECTOR_FACTOR;
    measurements->star1_orbit_x = (double)((int16_t)(telemetry_data[43] << 8 | telemetry_data[42])) * VECTOR_FACTOR;
    measurements->star1_orbit_y = (double)((int16_t)(telemetry_data[45] << 8 | telemetry_data[44])) * VECTOR_FACTOR;
    measurements->star1_orbit_z = (double)((int16_t)(telemetry_data[47] << 8 | telemetry_data[46])) * VECTOR_FACTOR;
    measurements->star2_body_x = (double)((int16_t)(telemetry_data[49] << 8 | telemetry_data[48])) * VECTOR_FACTOR;
    measurements->star2_body_y = (double)((int16_t)(telemetry_data[51] << 8 | telemetry_data[50])) * VECTOR_FACTOR;
    measurements->star2_body_z = (double)((int16_t)(telemetry_data[53] << 8 | telemetry_data[52])) * VECTOR_FACTOR;
    measurements->star2_orbit_x = (double)((int16_t)(telemetry_data[55] << 8 | telemetry_data[54])) * VECTOR_FACTOR;
    measurements->star2_orbit_y = (double)((int16_t)(telemetry_data[57] << 8 | telemetry_data[56])) * VECTOR_FACTOR;
    measurements->star2_orbit_z = (double)((int16_t)(telemetry_data[59] << 8 | telemetry_data[58])) * VECTOR_FACTOR;
    measurements->star3_body_x = (double)((int16_t)(telemetry_data[61] << 8 | telemetry_data[60])) * VECTOR_FACTOR;
    measurements->star3_body_y = (double)((int16_t)(telemetry_data[63] << 8 | telemetry_data[62])) * VECTOR_FACTOR;
    measurements->star3_body_z = (double)((int16_t)(telemetry_data[65] << 8 | telemetry_data[64])) * VECTOR_FACTOR;
    measurements->star3_orbit_x = (double)((int16_t)(telemetry_data[67] << 8 | telemetry_data[66])) * VECTOR_FACTOR;
    measurements->star3_orbit_y = (double)((int16_t)(telemetry_data[69] << 8 | telemetry_data[68])) * VECTOR_FACTOR;
    measurements->star3_orbit_z = (double)((int16_t)(telemetry_data[71] << 8 | telemetry_data[70])) * VECTOR_FACTOR;

    return 0;
}