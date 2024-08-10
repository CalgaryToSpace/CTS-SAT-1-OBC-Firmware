/*
 * adcs_types.c
 *
 *  Created on: Mar 7, 2024
 *      Author: Nadeem Moosa, Saksham Puri, Zachary Uy
 */

#include "adcs_types.h"
#include "adcs_ids.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

extern I2C_HandleTypeDef hi2c1; // allows not needing the parameters

/// @brief Instructs the ADCS to determine whether the last command succeeded. (Doesn't work for telemetry requests, by design.)
/// @param[out] ack Structure containing the formatted information about the last command sent.
/// @return 0 if successful, non-zero if an error occurred in transmission.
uint8_t ADCS_TC_Ack(ADCS_TC_Ack_Struct *ack) {
	uint8_t data_received[8]; // define temp buffer
	uint8_t data_length = 4;

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_COMMAND_ACK, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

	// map temp buffer to Ack struct
	ADCS_Pack_to_Ack(&data_received[0], ack);

	WRITE_STRUCT_TO_MEMORY(ack) // memory module function

	if (tlm_status == 0) {
		return ack->error_flag;
	}

	return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for the ADCS_TC_Ack command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Ack(uint8_t* data_received, ADCS_TC_Ack_Struct *result) {

	// map temp buffer to Ack struct
	result->last_id = data_received[0];
	result->processed = data_received[1];
	result->error_flag = data_received[2];
	result->error_index = data_received[3];

	return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Reset command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Reset() {
	// returns telecommand error flag
	uint8_t data_send[1] = {ADCS_MAGIC_NUMBER};
	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_RESET, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Identification command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Identification() {

	uint8_t data_length = 8;
	uint8_t data_received[data_length]; // define temp buffer

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_IDENTIFICATION, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

	ADCS_ID_Struct id; 
	ADCS_Pack_to_Identification(&data_received[0], &id);

	WRITE_STRUCT_TO_MEMORY(id) // memory module function

	return tlm_status;
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

/// @brief Instruct the ADCS to execute the ADCS_Program_Status command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Program_Status() {
	uint8_t data_length = 6;
	uint8_t data_received[data_length]; // define temp buffer

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_BOOT_RUNNING_PROGRAM_STATUS, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

	ADCS_Boot_Running_Status_Struct status; 
	ADCS_Pack_to_Program_Status(&data_received[0], &status);

	WRITE_STRUCT_TO_MEMORY(status) // memory module function

	return tlm_status;
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

/// @brief Instruct the ADCS to execute the ADCS_Communication_Status command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Communication_Status() {
	// returns I2C communication status of the ADCS (Table 37)
	uint8_t data_length = 6;
	uint8_t data_received[data_length]; // define temp buffer

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_SATSTATE_COMM_STATUS, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

	ADCS_Comms_Status_Struct status;
	ADCS_Pack_to_Comms_Status(&data_received[0], &status);

	WRITE_STRUCT_TO_MEMORY(status) // memory module function

	return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Comms_Status(uint8_t *data_received, ADCS_Comms_Status_Struct *result) {
    result->tc_counter = data_received[1] << 8 | data_received[0]; // uint16_t
    result->tlm_counter = data_received[3] << 8 | data_received[2]; // uint16_t
    result->tc_buffer_overrun = data_received[4] & 128; // bit 0 is 1 if TC buffer was overrun while receiving a telecommand
    result->i2c_tlm_error = data_received[4] & 16; // bit 3 is 1 if the number of data clocked out was more than the telemetry package
    result->i2c_tc_error = data_received[4] & 8; // bit 4 is 1 if the telecommand sent exceeded the buffer size
    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Deploy_Magnetometer command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Deploy_Magnetometer(uint8_t deploy_timeout) {
	// Deploys the magnetometer boom, timeout in seconds
	uint8_t data_send[1] = {deploy_timeout};
	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_DEPLOY_MAGNETOMETER_BOOM, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Set_Run_Mode command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Set_Run_Mode(ADCS_Run_Mode mode) {
	// Disables the ADCS
	uint8_t data_send[1] = {mode};
	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_ADCS_RUN_MODE, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Clear_Errors command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Clear_Errors() {
	// Clears error flags
	// NOTE: THERE IS ANOTHER, SEPARATE CLEAR ERROR FLAG TC FOR THE BOOTLODER (ADCS_COMMAND_BL_CLEAR_ERRORS)
	uint8_t data_send[1] = {192}; //0b11000000
	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_CLEAR_ERRORS, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the titular command.
/// @param mode Control mode to set (Table 77 in Firmware Manual)
/// @param timeout Timeout to set control mode
/// @return 0 if successful, non-zero if an error occurred in transmission.
uint8_t ADCS_Attitude_Control_Mode(ADCS_Control_Mode mode, uint16_t timeout) {
	// Sets the ADCS attitude control mode
	// See User Manual, Section 4.4.3 Table 3 for requirements to switch control mode
	uint8_t data_send[3] = {mode, timeout & 0x00FF, timeout >> 8};
	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_SET_ATTITUDE_CONTROL_MODE, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Attitude_Estimation_Mode command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Attitude_Estimation_Mode(ADCS_Estimation_Mode mode) {
	// Sets the ADCS attitude estimation mode
	// Possible values for mode given in Section 6.3 Table 80 of Firmware Reference Manual (ranges from 0 to 7)
	// needs power control to be on
	uint8_t data_send[1] = {mode};
	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_SET_ATTITUDE_ESTIMATION_MODE, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Run_Once command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Run_Once() {
	// requires ADCS_Enable_Triggered to have run first
	// (if ADCS_Enable_On has run instead, then this is unnecessary)
	uint8_t data_send[1]; // 0-byte data (from manual) input into wrapper, but one-byte here to avoid warnings
	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_TRIGGER_ADCS_LOOP, data_send, 0, ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the titular command.
/// @param mode Mode to set (Table 89 in Firmware Manual)
/// @return 0 if successful, non-zero if an error occurred in transmission.
uint8_t ADCS_Set_Magnetometer_Mode(ADCS_Magnetometer_Mode mode) {
	uint8_t data_send[1] = {mode};
	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_SET_MODE_OF_MAGNETOMETER_OPERATION, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Set_Magnetorquer_Output command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Set_Magnetorquer_Output(double x_duty, double y_duty, double z_duty) {
	// only valid after ADCS_Enable_Manual_Control is run
	// for the duty equations, raw parameter value is obtained using the formula: (raw parameter) = (formatted value)*1000.0
	// duty >> 8 gives upper byte, duty & 0x00FF gives lower byte
	uint8_t data_send[6];
	// swap low and high bytes and populate data_send
	ADCS_switch_order(data_send, ((uint16_t) (x_duty * 1000)), 0);
	ADCS_switch_order(data_send, ((uint16_t) (y_duty * 1000)), 2);
	ADCS_switch_order(data_send, ((uint16_t) (z_duty * 1000)), 4);
	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_SET_MAGNETORQUER_OUTPUT, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the titular command.
/// @param x_speed Wheel speed X value
/// @param y_speed Wheel speed Y value
/// @param z_speed Wheel speed Z value
/// @return 0 if successful, non-zero if an error occurred in transmission.
uint8_t ADCS_Set_Wheel_Speed(int16_t x_speed, int16_t y_speed, int16_t z_speed) {
	// only valid after ADCS_Enable_Manual_Control is run
	// for the duty equations, raw parameter value is in rpm
	uint8_t data_send[6]; // 6-byte data to send
	// swap low and high bytes and populate data_send
	ADCS_switch_order(data_send, x_speed, 0);
	ADCS_switch_order(data_send, y_speed, 2);
	ADCS_switch_order(data_send, z_speed, 4);
	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_SET_WHEEL_SPEED, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the titular command.
/// @param cube_control_signal Power control mode for cube control signal
/// @param cube_control_motor Power control mode for cube control motor
/// @param cube_sense1 Power control mode for cube sense 1
/// @param cube_sense2 Power control mode for cube sense 2
/// @param cube_star_power Power control mode for cube star
/// @param cube_wheel1_power Power control mode for cube wheel 1
/// @param cube_wheel2_power Power control mode for cube wheel 2
/// @param cube_wheel3_power Power control mode for cube wheel 3
/// @param motor_power Power control mode for motor
/// @param gps_power Power control mode for gps
/// @return 0 if successful, non-zero if an error occurred in transmission.
uint8_t ADCS_Set_Power_Control(ADCS_Power_Select cube_control_signal, ADCS_Power_Select cube_control_motor, ADCS_Power_Select cube_sense1,
        ADCS_Power_Select cube_sense2, ADCS_Power_Select cube_star_power, ADCS_Power_Select cube_wheel1_power,
        ADCS_Power_Select cube_wheel2_power, ADCS_Power_Select cube_wheel3_power, ADCS_Power_Select motor_power,
        ADCS_Power_Select gps_power) {
	uint8_t data_send[3]; // 3-byte data (from manual)
	// within the byte, everything goes in reverse order!!
	data_send[0] = (cube_control_signal) | (cube_control_motor << 2) | (cube_sense1 << 4) | (cube_sense2 << 6);
	data_send[1] = (cube_star_power) | (cube_wheel1_power << 2) | (cube_wheel2_power << 4) | (cube_wheel3_power << 6);
	data_send[2] = (motor_power) | (gps_power << 2);
	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_CUBEACP_SET_ADCS_POWER_CONTROL, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Power_Control command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Get_Power_Control() {
	uint8_t data_length = 3;
	uint8_t data_received[data_length]; // define temp buffer

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_GET_ADCS_POWER_CONTROL, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

	ADCS_Power_Control_Struct power;
	ADCS_Pack_to_Power_Control(data_received, &power);

	WRITE_STRUCT_TO_MEMORY(power) // memory module function

	return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
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


/// @brief Instruct the ADCS to set the magnetometer configuration.
/// @param mounting_transform_alpha_angle Mounting transform alpha angle [deg] 
/// @param mounting_transform_beta_angle Mounting transform beta angle [deg]
/// @param mounting_transform_gamma_angle Mounting transform gamma angle [deg]
/// @param channel_1_offset Channel 1 offset value
/// @param channel_2_offset Channel 2 offset value
/// @param channel_3_offset Channel 3 offset value
/// @param sensitivity_matrix_s11 Value (1, 1) of the magnetometer sensitivity matrix
/// @param sensitivity_matrix_s22 Value (2, 2) of the magnetometer sensitivity matrix
/// @param sensitivity_matrix_s33 Value (3, 3) of the magnetometer sensitivity matrix
/// @param sensitivity_matrix_s12 Value (1, 2) of the magnetometer sensitivity matrix
/// @param sensitivity_matrix_s13 Value (1, 3) of the magnetometer sensitivity matrix
/// @param sensitivity_matrix_s21 Value (2, 1) of the magnetometer sensitivity matrix
/// @param sensitivity_matrix_s23 Value (2, 3) of the magnetometer sensitivity matrix
/// @param sensitivity_matrix_s31 Value (3, 1) of the magnetometer sensitivity matrix
/// @param sensitivity_matrix_s32 Value (3, 2) of the magnetometer sensitivity matrix
/// @return 0 if successful, non-zero if an error occurred in transmission.
uint8_t ADCS_Set_Magnetometer_Config(
		double mounting_transform_alpha_angle,
        double mounting_transform_beta_angle,
        double mounting_transform_gamma_angle,
        double channel_1_offset,
        double channel_2_offset,
        double channel_3_offset,
        double sensitivity_matrix_s11,
        double sensitivity_matrix_s22,
        double sensitivity_matrix_s33,
        double sensitivity_matrix_s12,
        double sensitivity_matrix_s13,
        double sensitivity_matrix_s21,
        double sensitivity_matrix_s23,
        double sensitivity_matrix_s31,
        double sensitivity_matrix_s32) {

	uint8_t data_send[30]; // 30-byte data (from manual)

	// reorder uint8_t bytes to be low-byte then high-byte
	// actual config is raw value divided by 100 for mounting transform angles
	// and raw value divided by 1000 for everything else
	ADCS_switch_order(data_send, ((int16_t) (mounting_transform_alpha_angle * 100)), 0);
	ADCS_switch_order(data_send, ((int16_t) (mounting_transform_beta_angle * 100)), 2);
	ADCS_switch_order(data_send, ((int16_t) (mounting_transform_gamma_angle * 100)), 4);
	ADCS_switch_order(data_send, ((int16_t) (channel_1_offset * 1000)), 6);
	ADCS_switch_order(data_send, ((int16_t) (channel_2_offset * 1000)), 8);
	ADCS_switch_order(data_send, ((int16_t) (channel_3_offset * 1000)), 10);
	ADCS_switch_order(data_send, ((int16_t) (sensitivity_matrix_s11 * 1000)), 12);
	ADCS_switch_order(data_send, ((int16_t) (sensitivity_matrix_s22 * 1000)), 14);
	ADCS_switch_order(data_send, ((int16_t) (sensitivity_matrix_s33 * 1000)), 16);
	ADCS_switch_order(data_send, ((int16_t) (sensitivity_matrix_s12 * 1000)), 18);
	ADCS_switch_order(data_send, ((int16_t) (sensitivity_matrix_s13 * 1000)), 20);
	ADCS_switch_order(data_send, ((int16_t) (sensitivity_matrix_s21 * 1000)), 22);
	ADCS_switch_order(data_send, ((int16_t) (sensitivity_matrix_s23 * 1000)), 24);
	ADCS_switch_order(data_send, ((int16_t) (sensitivity_matrix_s31 * 1000)), 26);
	ADCS_switch_order(data_send, ((int16_t) (sensitivity_matrix_s32 * 1000)), 28);

	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_CUBEACP_SET_MAGNETOMETER_CONFIG, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
	return tc_status;

}

/// @brief Instruct the ADCS to execute the ADCS_Save_Config command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Save_Config() {
	uint8_t data_send[1]; // 0-byte data (from manual) input into wrapper, but one-byte here to avoid warnings
	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_SAVE_CONFIG, data_send, 0, ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Save_Orbit_Params command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Save_Orbit_Params() {
	uint8_t data_send[1]; // 0-byte data (from manual) input into wrapper, but one-byte here to avoid warnings
	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_SAVE_ORBIT_PARAMS, data_send, 0, ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Estimate_Angular_Rates command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Estimate_Angular_Rates() {
	uint8_t data_length = 6;
	uint8_t data_received[data_length]; // define temp buffer

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_ESTIMATED_ANGULAR_RATES, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

	ADCS_Angular_Rates_Struct rates;
	ADCS_Pack_to_Angular_Rates(data_received, &rates);

	WRITE_STRUCT_TO_MEMORY(rates) // memory module function

	return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Angular_Rates(uint8_t *data_received, ADCS_Angular_Rates_Struct *result) {
	// need to convert to int16 first, then double, to ensure negative numbers are represented correctly
    result->x_rate = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 0.01;
    result->y_rate = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 0.01;
    result->z_rate = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 0.01;
    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_LLH_Position command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Get_LLH_Position() {
	uint8_t data_length = 6;
	uint8_t data_received[data_length]; // define temp buffer

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_SATELLITE_POSITION_LLH, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

	ADCS_LLH_Position_Struct pos;
	ADCS_Pack_to_LLH_Position(data_received, &pos);

	WRITE_STRUCT_TO_MEMORY(pos) // memory module function

	return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_LLH_Position(uint8_t *data_received, ADCS_LLH_Position_Struct *result) {
    // formatted value (deg or km) = raw value * 0.01
	// need to convert to int16 first, then double, to ensure negative numbers are represented correctly
	result->latitude  = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 0.01; 
    result->longitude = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 0.01; 
    result->altitude  = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 0.01; 
    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Bootloader_Clear_Errors command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Bootloader_Clear_Errors() {
	uint8_t data_send[1]; // 0-byte data (from manual) input into wrapper, but one-byte here to avoid warnings
	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_BOOTLOADER_CLEAR_ERRORS, data_send, 0, ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Set_Unix_Time_Save_Mode command.
/// @param[in] save_now whether to save the current Unix time immediately
/// @param[in] save_on_update whether to save the current Unix time whenever a command is used to update it
/// @param[in] save_periodic whether to save the current Unix time periodically
/// @param[in] period the period of saving the current Unix time
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Set_Unix_Time_Save_Mode(bool save_now, bool save_on_update, bool save_periodic, uint8_t period) {
	uint8_t data_send[2] = { (save_now | (save_on_update << 1) | (save_periodic << 2) ) , period}; // 2-byte data (from manual)
	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_SET_UNIX_TIME_SAVE_TO_FLASH, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Unix_Time_Save_Mode command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Get_Unix_Time_Save_Mode() {
	uint8_t data_length = 2;
	uint8_t data_received[data_length]; // define temp buffer

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_GET_UNIX_TIME_SAVE_TO_FLASH, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

	ADCS_Set_Unix_Time_Save_Mode_Struct mode; 
	ADCS_Pack_to_Unix_Time_Save_Mode(data_received, &mode);

	WRITE_STRUCT_TO_MEMORY(mode) // memory module function

	return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Unix_Time_Save_Mode(uint8_t *data_received, ADCS_Set_Unix_Time_Save_Mode_Struct *result) {
    result->save_now = data_received[0] & 1; // 0b00000001
    result->save_on_update = (data_received[0] & 2) >> 1; // 0b00000010
    result->save_periodic = (data_received[0] & 4) >> 2; // 0b00000100
    result->period = data_received[1];
    return 0;
}

// TODO: Figure out why the SGP4 get/set functions aren't working (all zeroes).
// I bet it's to do with memcpy and double types.

/// @brief Instruct the ADCS to execute the ADCS_Set_SGP4_Orbit_Params command.
/// @param[in] inclination inclination (degrees)
/// @param[in] eccentricity eccentricity (dimensionless)
/// @param[in] ascending_node_right_ascension right ascension of the ascending node (degrees)
/// @param[in] perigee_argument argument of perigee (degrees)
/// @param[in] b_star_drag_term b-star drag term (dimensionless)
/// @param[in] mean_motion mean motion (orbits per day)
/// @param[in] mean_anomaly mean anomaly (degrees)
/// @param[in] epoch epoch (double; integer component is year, decimal component is day)
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Set_SGP4_Orbit_Params(double inclination, double eccentricity, double ascending_node_right_ascension,
								//         degrees,			   dimensionless, 		    degrees
		double perigee_argument, double b_star_drag_term, double mean_motion, double mean_anomaly, double epoch) {
		// degrees,					dimensionless,			orbits/day,			degrees,			years.days

	uint8_t data_send[64];

	// convert doubles to arrays of uint8_t
	memcpy(&data_send[0],  &inclination, sizeof(inclination));
	memcpy(&data_send[8],  &eccentricity, sizeof(eccentricity));
	memcpy(&data_send[16], &ascending_node_right_ascension, sizeof(ascending_node_right_ascension));
	memcpy(&data_send[24], &perigee_argument, sizeof(perigee_argument));
	memcpy(&data_send[32], &b_star_drag_term, sizeof(b_star_drag_term));
	memcpy(&data_send[40], &mean_motion, sizeof(mean_motion));
	memcpy(&data_send[48], &mean_anomaly, sizeof(mean_anomaly));
	memcpy(&data_send[56], &epoch, sizeof(epoch));

	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_CUBEACP_SET_SGP4_ORBIT_PARAMETERS, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_SGP4_Orbit_Params command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Get_SGP4_Orbit_Params() {
	uint8_t data_length = 64;
	uint8_t data_received[data_length]; // define temp buffer

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_GET_SGP4_ORBIT_PARAMETERS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

	ADCS_Orbit_Params_Struct params;
	ADCS_Pack_to_Orbit_Params(data_received, &params);

	WRITE_STRUCT_TO_MEMORY(params) // memory module function

	return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
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

/// @brief Instruct the ADCS to execute the ADCS_Rate_Sensor_Rates command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Rate_Sensor_Rates() {
	uint8_t data_length = 6;
	uint8_t data_received[data_length]; // define temp buffer

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_RATE_SENSOR_RATES, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

	ADCS_Rated_Sensor_Rates_Struct rates;
	ADCS_Pack_to_Rated_Sensor_Rates(data_received, &rates);

	WRITE_STRUCT_TO_MEMORY(rates) // memory module function

	return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Rated_Sensor_Rates(uint8_t *data_received, ADCS_Rated_Sensor_Rates_Struct *result) {
    // formatted value (deg/s) = raw value * 0.01
	// need to convert to int16 first, then double, to ensure negative numbers are represented correctly
	result->x  = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 0.01; 
    result->y = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 0.01; 
    result->z  = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 0.01; 
    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Wheel_Speed command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Get_Wheel_Speed() {
	uint8_t data_length = 6;
	uint8_t data_received[data_length]; // define temp buffer

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_WHEEL_SPEED, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

	ADCS_Wheel_Speed_Struct speeds;
	ADCS_Pack_to_Wheel_Speed(data_received, &speeds);

	WRITE_STRUCT_TO_MEMORY(speeds) // memory module function

	return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Wheel_Speed(uint8_t *data_received, ADCS_Wheel_Speed_Struct *result) {
	// all values in rpm
    result->x = data_received[1] << 8 | data_received[0];
    result->y = data_received[3] << 8 | data_received[2];
    result->z = data_received[5] << 8 | data_received[4];
    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Magnetorquer_Command command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Get_Magnetorquer_Command() {
	uint8_t data_length = 6;
	uint8_t data_received[data_length]; // define temp buffer

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_MAGNETORQUER_COMMAND, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

	ADCS_Magnetorquer_Command_Struct time;
	ADCS_Pack_to_Magnetorquer_Command(data_received, &time);

	WRITE_STRUCT_TO_MEMORY(time) // memory module function

	return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Magnetorquer_Command(uint8_t *data_received, ADCS_Magnetorquer_Command_Struct *result) {
    // formatted value (sec) = raw value * 0.01
	result->x = ((double)((int16_t)(data_received[1] << 8 | data_received[0]))) * 0.01;
    result->y = ((double)((int16_t)(data_received[3] << 8 | data_received[2]))) * 0.01;
    result->z = ((double)((int16_t)(data_received[5] << 8 | data_received[4]))) * 0.01;
    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Raw_Magnetometer_Values command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Get_Raw_Magnetometer_Values() {
	uint8_t data_length = 6;
	uint8_t data_received[data_length]; // define temp buffer

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_RAW_MAGNETOMETER, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

	ADCS_Raw_Mag_TLM_Struct mag_vals;
	ADCS_Pack_to_Raw_Magnetometer_Values(data_received, &mag_vals);

	WRITE_STRUCT_TO_MEMORY(mag_vals) // memory module function

	return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Raw_Magnetometer_Values(uint8_t *data_received, ADCS_Raw_Mag_TLM_Struct *result) {
	result->x = data_received[1] << 8 | data_received[0];
    result->y = data_received[3] << 8 | data_received[2];
    result->z = data_received[5] << 8 | data_received[4];
    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Estimate_Fine_Angular_Rates command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Estimate_Fine_Angular_Rates() {
	uint8_t data_length = 6;
	uint8_t data_received[data_length]; // define temp buffer

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_FINE_ESTIMATED_ANGULAR_RATES, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

	ADCS_Fine_Angular_Rates_Struct rates;
	ADCS_Pack_to_Fine_Angular_Rates(data_received, &rates);

	WRITE_STRUCT_TO_MEMORY(rates) // memory module function

	return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Fine_Angular_Rates(uint8_t *data_received, ADCS_Fine_Angular_Rates_Struct *result) {
    // formatted value (deg/s) = raw value * 0.001
	result->x = ((double)((int16_t)(data_received[1] << 8 | data_received[0]))) * 0.001;
    result->y = ((double)((int16_t)(data_received[3] << 8 | data_received[2]))) * 0.001;
    result->z = ((double)((int16_t)(data_received[5] << 8 | data_received[4]))) * 0.001;
    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Magnetometer_Config command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Get_Magnetometer_Config() {
	uint8_t data_length = 30;
	uint8_t data_received[data_length]; // define temp buffer

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_GET_MAGNETOMETER_CONFIG, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

	ADCS_Magnetometer_Config_Struct config;
	ADCS_Pack_to_Magnetometer_Config(data_received, &config);

	WRITE_STRUCT_TO_MEMORY(config) // memory module function

	return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
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

/// @brief Instruct the ADCS to execute the ADCS_Get_Commanded_Attitude_Angles command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Get_Commanded_Attitude_Angles() {
	uint8_t data_length = 6;
	uint8_t data_received[data_length]; // define temp buffer

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_GET_COMMANDED_ATTITUDE_ANGLES, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

	ADCS_Commanded_Angles_Struct angles;
	ADCS_Pack_to_Commanded_Attitude_Angles(data_received, &angles);

	WRITE_STRUCT_TO_MEMORY(angles) // memory module function

	return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Commanded_Attitude_Angles(uint8_t *data_received, ADCS_Commanded_Angles_Struct *result) {
    // Formatted value is obtained using the formula: (formatted value) [deg] = RAWVAL*0.01
	result->x = (double)((int16_t)(data_received[1] << 8 | data_received[0])) * 0.01;
    result->y = (double)((int16_t)(data_received[3] << 8 | data_received[2])) * 0.01;
    result->z = (double)((int16_t)(data_received[5] << 8 | data_received[4])) * 0.01;
    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Set_Commanded_Attitude_Angles command.
/// @param[in] x x attitude angle
/// @param[in] y y attitude angle
/// @param[in] z z attitude angle
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Set_Commanded_Attitude_Angles(double x, double y, double z) {
	// raw parameter value is obtained using the formula: (raw parameter) = (formatted value)*100.0
	// angle >> 8 gives upper byte, angle & 0x00FF gives lower byte
	uint8_t data_send[6];
	// swap low and high bytes and populate data_send
	ADCS_switch_order(data_send, ((int16_t) (x * 100)), 0);
	ADCS_switch_order(data_send, ((int16_t) (y * 100)), 2);
	ADCS_switch_order(data_send, ((int16_t) (z * 100)), 4);
	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_CUBEACP_SET_COMMANDED_ATTITUDE_ANGLES, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Set_Estimation_Params command.
/// @param[in] magnetometer_rate_filter_system_noise Magnetometer rate filter system noise parameter
/// @param[in] ekf_system_noise EKS system noise parameter
/// @param[in] css_measurement_noise CSS measurement noise parameter
/// @param[in] sun_sensor_measurement_noise sun sensor measurement noise parameter
/// @param[in] nadir_sensor_measurement_noise nadir sensor measurement noise parameter
/// @param[in] magnetometer_measurement_noise magnetometer measurement noise parameter
/// @param[in] star_tracker_measurement_noise star tracker measurement noise parameter
/// @param[in] use_sun_sensor whether or not to use the sun sensor measurement in EKF
/// @param[in] use_nadir_sensor whether or not to use the nadir sensor measurement in EKF
/// @param[in] use_css whether or not to use the CSS measurement in EKF
/// @param[in] use_star_tracker whether or not to use the star tracker measurement in EKF
/// @param[in] nadir_sensor_terminator_test select to ignore nadir sensor measurements when terminator is in FOV
/// @param[in] automatic_magnetometer_recovery select whether automatic switch to redundant magnetometer should occur in case of failure
/// @param[in] magnetometer_mode select magnetometer mode for estimation and control
/// @param[in] magnetometer_selection_for_raw_mtm_tlm select magnetometer mode for the second raw telemetry frame
/// @param[in] automatic_estimation_transition_due_to_rate_sensor_errors enable/disable automatic transition from MEMS rate estimation mode to RKF in case of rate sensor error
/// @param[in] wheel_30s_power_up_delay present in CubeSupport but not in the manual -- need to test
/// @param[in] cam1_and_cam2_sampling_period the manual calls it this, but CubeSupport calls it "error counter reset period" -- need to test
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Set_Estimation_Params(
								float magnetometer_rate_filter_system_noise, 
                                float ekf_system_noise, 
                                float css_measurement_noise, 
                                float sun_sensor_measurement_noise, 
                                float nadir_sensor_measurement_noise, 
                                float magnetometer_measurement_noise, 
                                float star_tracker_measurement_noise, 
                                bool use_sun_sensor, 
                                bool use_nadir_sensor, 
                                bool use_css, 
                                bool use_star_tracker, 
                                bool nadir_sensor_terminator_test, 
                                bool automatic_magnetometer_recovery, 
                                ADCS_Magnetometer_Mode magnetometer_mode, // this is actually the same one as for ID 56!
                                ADCS_Magnetometer_Mode magnetometer_selection_for_raw_mtm_tlm, // and so is this, actually!
                                bool automatic_estimation_transition_due_to_rate_sensor_errors, 
								bool wheel_30s_power_up_delay, // present in CubeSupport but not in the manual -- need to test
                                uint8_t cam1_and_cam2_sampling_period) { // the manual calls it this, but CubeSupport calls it "error counter reset period" -- need to test
	// float uses IEEE 754 float32, with all bytes reversed, so eg. 1.1 becomes [0xCD, 0xCC, 0x8C, 0x3F]
	// the float type should already be reversed, but need to test in implementation
	uint8_t data_send[31];

	// convert floats to reversed arrays of uint8_t
	memcpy(&data_send[0],  &magnetometer_rate_filter_system_noise, sizeof(magnetometer_rate_filter_system_noise));
	memcpy(&data_send[4],  &ekf_system_noise, sizeof(ekf_system_noise));
	memcpy(&data_send[8],  &css_measurement_noise, sizeof(css_measurement_noise));
	memcpy(&data_send[12], &sun_sensor_measurement_noise, sizeof(sun_sensor_measurement_noise));
	memcpy(&data_send[16], &nadir_sensor_measurement_noise, sizeof(nadir_sensor_measurement_noise));
	memcpy(&data_send[20], &magnetometer_measurement_noise, sizeof(magnetometer_measurement_noise));
	memcpy(&data_send[24], &star_tracker_measurement_noise, sizeof(star_tracker_measurement_noise));

	// convert bools to uint8
	data_send[28] = (magnetometer_mode << 6) | (automatic_magnetometer_recovery << 5) | (nadir_sensor_terminator_test << 4) | (use_star_tracker << 3) | (use_css << 2) | (use_nadir_sensor << 1) | (use_sun_sensor);
	data_send[29] = (wheel_30s_power_up_delay << 3) | (automatic_estimation_transition_due_to_rate_sensor_errors << 2) | (magnetometer_selection_for_raw_mtm_tlm);

	data_send[30] = cam1_and_cam2_sampling_period; // lower four bits are for cam1 and upper four are for cam2 if the manual is correct, not CubeSupport

	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_CUBEACP_SET_ESTIMATION_PARAMETERS, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Estimation_Params command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Get_Estimation_Params() {
	uint8_t data_length = 31;
	uint8_t data_received[data_length]; // define temp buffer

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_GET_ESTIMATION_PARAMETERS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

	ADCS_Estimation_Params_Struct params;
	ADCS_Pack_to_Estimation_Params(data_received, &params);

	WRITE_STRUCT_TO_MEMORY(params) // memory module function

	return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
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


/// @brief Set the Augmented SGP4 Parameters of the ADCS.
/// @param[in] incl_coefficient Set inclination filter coefficient
/// @param[in] raan_coefficient Set RAAN filter coefficient 
/// @param[in] ecc_coefficient Set eccentricity filter coefficient
/// @param[in] aop_coefficient Set argument of perigee filter coefficient
/// @param[in] time_coefficient Set time filter coefficient
/// @param[in] pos_coefficient Set position filter coefficient
/// @param[in] maximum_position_error Maximum position error for ASGP4 to continue working
/// @param[in] asgp4_filter The type of filter being used (enum)
/// @param[in] xp_coefficient Polar coefficient xp  
/// @param[in] yp_coefficient Polar coefficient yp 
/// @param[in] gps_roll_over GPS roll over number
/// @param[in] position_sd Maximum position standard deviation for ASGP4 to continue working
/// @param[in] velocity_sd Maximum velocity standard deviation for ASGP4 to continue working
/// @param[in] min_satellites Minimum satellites required for ASGP4 to continue working
/// @param[in] time_gain Time offset compensation gain
/// @param[in] max_lag Maximum lagged timestamp measurements to incorporate
/// @param[in] min_samples Minimum samples to use to get ASGP4
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Set_ASGP4_Params(double incl_coefficient, double raan_coefficient, double ecc_coefficient, double aop_coefficient, double time_coefficient, double pos_coefficient, double maximum_position_error, ADCS_ASGP4_Filter asgp4_filter, double xp_coefficient, double yp_coefficient, uint8_t gps_roll_over, double position_sd, double velocity_sd, uint8_t min_satellites, double time_gain, double max_lag, uint16_t min_samples) {
	uint8_t data_send[30]; // from Table 209

	// populate data_send
	ADCS_switch_order(data_send, (uint16_t) (incl_coefficient * 1000), 0);
	ADCS_switch_order(data_send, (uint16_t) (raan_coefficient * 1000), 2);
	ADCS_switch_order(data_send, (uint16_t) (ecc_coefficient * 1000), 4);
	ADCS_switch_order(data_send, (uint16_t) (aop_coefficient * 1000), 6);
	ADCS_switch_order(data_send, (uint16_t) (time_coefficient * 1000), 8);
	ADCS_switch_order(data_send, (uint16_t) (pos_coefficient * 1000), 10);
	data_send[12] = (uint8_t) (maximum_position_error * 10);
	data_send[13] = (uint8_t) asgp4_filter;
	ADCS_switch_order_32(data_send, (int32_t) (xp_coefficient * 10000000), 14);
	ADCS_switch_order_32(data_send, (int32_t) (yp_coefficient * 10000000), 18);
	data_send[22] = gps_roll_over;
	data_send[23] = (uint8_t) (position_sd * 10);
	data_send[24] = (uint8_t) (velocity_sd * 100);
	data_send[25] = min_satellites;
	data_send[26] = (uint8_t) (time_gain * 100);
	data_send[27] = (uint8_t) (max_lag * 100);
	ADCS_switch_order(data_send, min_samples, 28);

	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_CUBEACP_SET_AUGMENTED_SGP4_PARAMETERS, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_ASGP4_Params command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Get_ASGP4_Params() {
	uint8_t data_length = 30;
	uint8_t data_received[data_length]; // define temp buffer

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_GET_AUGMENTED_SGP4_PARAMETERS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

	ADCS_ASGP4_Params_Struct params;
	ADCS_Pack_to_ASGP4_Params(data_received, &params);

	WRITE_STRUCT_TO_MEMORY(params) // memory module function

	return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
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


/// @brief Instruct the ADCS to execute the ADCS_Set_Tracking_Controller_Target_Reference command.
/// @param[in] lon longitude
/// @param[in] lat latitude
/// @param[in] alt altitude
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Set_Tracking_Controller_Target_Reference(float lon, float lat, float alt) {
	uint8_t data_send[12];

	// float uses IEEE 754 float32, with all bytes reversed, so eg. 1.1 becomes [0xCD, 0xCC, 0x8C, 0x3F]
	// the float type should already be reversed, but need to test in implementation
	// convert floats to reversed arrays of uint8_t
	memcpy(&data_send[0],  &lon, sizeof(lon));
	memcpy(&data_send[4],  &lat, sizeof(lat));
	memcpy(&data_send[8],  &alt, sizeof(alt));

	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_CUBEACP_SET_TRACKING_CONTROLLER_TARGET_REFERENCE, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Tracking_Controller_Target_Reference command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Get_Tracking_Controller_Target_Reference() {
	uint8_t data_length = 12;
	uint8_t data_received[data_length]; // define temp buffer

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_GET_TRACKING_CONTROLLER_TARGET_REFERENCE, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

	ADCS_Tracking_Controller_Target_Struct ref;
	ADCS_Pack_to_Tracking_Controller_Target_Reference(data_received, &ref);

	WRITE_STRUCT_TO_MEMORY(ref);

	return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Tracking_Controller_Target_Reference(uint8_t* data_received, ADCS_Tracking_Controller_Target_Struct* ref) {
    // map temp buffer to struct
    memcpy(&ref->lon, &data_received[0], 4);
    memcpy(&ref->lat, &data_received[4], 4);
    memcpy(&ref->alt, &data_received[8], 4);

    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Set_Rate_Gyro_Config command.
/// @param[in] gyro1 Axis for Gyro #1 (options are pos/neg x, pos/neg y, pos/neg z)
/// @param[in] gyro2 Axis for Gyro #2 (options are pos/neg x, pos/neg y, pos/neg z)
/// @param[in] gyro3 Axis for Gyro #3 (options are pos/neg x, pos/neg y, pos/neg z)
/// @param[in] x_rate_offset x-rate sensor offset
/// @param[in] y_rate_offset y-rate sensor offset
/// @param[in] z_rate_offset z-rate sensor offset
/// @param[in] rate_sensor_mult multiplier of rate sensor measurement
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Set_Rate_Gyro_Config(ADCS_Axis_Select gyro1, ADCS_Axis_Select gyro2, ADCS_Axis_Select gyro3, double x_rate_offset, double y_rate_offset, double z_rate_offset, uint8_t rate_sensor_mult) {
	uint8_t data_send[10];

	data_send[0] = (uint8_t) gyro1;
	data_send[1] = (uint8_t) gyro2;
	data_send[2] = (uint8_t) gyro3;

	ADCS_switch_order(data_send, (int16_t) (x_rate_offset * 1000), 3);
	ADCS_switch_order(data_send, (int16_t) (y_rate_offset * 1000), 5);
	ADCS_switch_order(data_send, (int16_t) (z_rate_offset * 1000), 7);

	data_send[9] = rate_sensor_mult;

	uint8_t tc_status = ADCS_I2C_telecommand_wrapper(ADCS_COMMAND_CUBEACP_SET_RATE_GYRO_CONFIG, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
	return tc_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Rate_Gyro_Config command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Get_Rate_Gyro_Config() {
	uint8_t data_length = 12;
	uint8_t data_received[data_length]; // define temp buffer

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_GET_RATE_GYRO_CONFIG, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

	ADCS_Rate_Gyro_Config_Struct config;
	ADCS_Pack_to_Rate_Gyro_Config(data_received, &config);

	WRITE_STRUCT_TO_MEMORY(config);

	return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
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

/// @brief Instruct the ADCS to execute the ADCS_Estimated_Attitude_Angles command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Estimated_Attitude_Angles() {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_ESTIMATED_ATTITUDE_ANGLES, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Estimated_Attitude_Angles_Struct angles; 
    ADCS_Pack_to_Estimated_Attitude_Angles(&data_received[0], &angles);

    WRITE_STRUCT_TO_MEMORY(angles); // memory module function

    return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Estimated_Attitude_Angles(uint8_t *data_received, ADCS_Estimated_Attitude_Angles_Struct *angles) {
    angles->estimated_roll_angle = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 0.01;
    angles->estimated_pitch_angle = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 0.01;
    angles->estimated_yaw_angle = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 0.01;
	return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Magnetic_Field_Vector command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Magnetic_Field_Vector() {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_MAGNETIC_FIELD_VECTOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Magnetic_Field_Vector_Struct vector_components; 
    ADCS_Pack_to_Magnetic_Field_Vector(&data_received[0], &vector_components);

    WRITE_STRUCT_TO_MEMORY(vector_components); // memory module function

    return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Magnetic_Field_Vector(uint8_t *data_received, ADCS_Magnetic_Field_Vector_Struct *vector_components) {
	// gives vector components in uT (10^-6 Teslas)
    vector_components->x = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 0.01;
    vector_components->y = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 0.01;
    vector_components->z = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 0.01;
	return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Fine_Sun_Vector command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Fine_Sun_Vector() {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_FINE_SUN_VECTOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Fine_Sun_Vector_Struct vector_components; 
    ADCS_Pack_to_Fine_Sun_Vector(&data_received[0], &vector_components);

    WRITE_STRUCT_TO_MEMORY(vector_components); // memory module function

    return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Fine_Sun_Vector(uint8_t *data_received, ADCS_Fine_Sun_Vector_Struct *vector_components) {
    vector_components->x = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) / 10000.0;
    vector_components->y = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) / 10000.0;
    vector_components->z = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) / 10000.0;
	return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Nadir_Vector command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Nadir_Vector() {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_NADIR_VECTOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Nadir_Vector_Struct vector_components; 
    ADCS_Pack_to_Nadir_Vector(&data_received[0], &vector_components);

    WRITE_STRUCT_TO_MEMORY(vector_components); // memory module function

    return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Nadir_Vector(uint8_t *data_received, ADCS_Nadir_Vector_Struct *vector_components) {
    vector_components->x = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) / 10000.0;
    vector_components->y = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) / 10000.0;
    vector_components->z = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) / 10000.0;
	return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Commanded_Wheel_Speed command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Commanded_Wheel_Speed() {
	uint8_t data_length = 6;
	uint8_t data_received[data_length]; // define temp buffer

	uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_WHEEL_SPEED_COMMANDS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

	ADCS_Wheel_Speed_Struct speeds;
	ADCS_Pack_to_Commanded_Wheel_Speed(data_received, &speeds);

	WRITE_STRUCT_TO_MEMORY(speeds) // memory module function

	return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Commanded_Wheel_Speed(uint8_t *data_received, ADCS_Wheel_Speed_Struct *result) {
	// all values in rpm
    result->x = data_received[1] << 8 | data_received[0];
    result->y = data_received[3] << 8 | data_received[2];
    result->z = data_received[5] << 8 | data_received[4];
    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_IGRF_Magnetic_Field_Vector command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_IGRF_Magnetic_Field_Vector() {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_IGRF_MODELLED_MAGNETIC_FIELD_VECTOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Magnetic_Field_Vector_Struct vector_components; 
    ADCS_Pack_to_IGRF_Magnetic_Field_Vector(&data_received[0], &vector_components);

    WRITE_STRUCT_TO_MEMORY(vector_components); // memory module function

    return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_IGRF_Magnetic_Field_Vector(uint8_t *data_received, ADCS_Magnetic_Field_Vector_Struct *vector_components) {
	// gives vector components in uT (10^-6 Teslas)
    vector_components->x = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 0.01;
    vector_components->y = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 0.01;
    vector_components->z = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 0.01;
	return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Quaternion_Error_Vector command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Quaternion_Error_Vector() {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_QUATERNION_ERROR_VECTOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Quaternion_Error_Vector_Struct q_error;
    ADCS_Pack_to_Quaternion_Error_Vector(&data_received[0], &q_error);

    WRITE_STRUCT_TO_MEMORY(q_error) // memory module function

    return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Quaternion_Error_Vector(uint8_t *data_received, ADCS_Quaternion_Error_Vector_Struct *result) {
    result->quaternion_error_q1 = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 0.0001;
    result->quaternion_error_q2 = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 0.0001;
    result->quaternion_error_q3 = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 0.0001;

    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Estimated_Gyro_Bias command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Estimated_Gyro_Bias() {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_ESTIMATED_GYRO_BIAS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Estimated_Gyro_Bias_Struct gyro_bias;
    ADCS_Pack_to_Estimated_Gyro_Bias(&data_received[0], &gyro_bias);

    WRITE_STRUCT_TO_MEMORY(gyro_bias) // memory module function

    return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Estimated_Gyro_Bias(uint8_t* data_received, ADCS_Estimated_Gyro_Bias_Struct *result) {
    result->estimated_x_gyro_bias = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 0.001;
    result->estimated_y_gyro_bias = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 0.001;
    result->estimated_z_gyro_bias = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 0.001;

    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Estimation_Innovation_Vector command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Estimation_Innovation_Vector() {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_ESTIMATION_INNOVATION_VECTOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Estimation_Innovation_Vector_Struct innovation_vector;
    ADCS_Pack_to_Estimation_Innovation_Vector(&data_received[0], &innovation_vector);

    WRITE_STRUCT_TO_MEMORY(innovation_vector) // memory module function

    return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Estimation_Innovation_Vector(uint8_t* data_received, ADCS_Estimation_Innovation_Vector_Struct* result) {
    result->innovation_vector_x = ((double) ((int16_t) (data_received[1] << 8 | data_received[0]))) * 0.0001;
    result->innovation_vector_y = ((double) ((int16_t) (data_received[3] << 8 | data_received[2]))) * 0.0001;
    result->innovation_vector_z = ((double) ((int16_t) (data_received[5] << 8 | data_received[4]))) * 0.0001;

    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_Cam1_Sensor command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Raw_Cam1_Sensor() {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_RAW_CAM1_SENSOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Raw_Cam_Sensor_Struct raw_cam1_sensor;
    ADCS_Pack_to_Raw_Cam1_Sensor(&data_received[0], &raw_cam1_sensor);

    WRITE_STRUCT_TO_MEMORY(raw_cam1_sensor) // memory module function

    return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Raw_Cam1_Sensor(uint8_t* data_received, ADCS_Raw_Cam_Sensor_Struct* result) {
    result->which_sensor = ADCS_Cam1_Sensor;
	result->cam_centroid_x = (int16_t) (data_received[1] << 8 | data_received[0]);
    result->cam_centroid_y = (int16_t) (data_received[3] << 8 | data_received[2]);
    result->cam_capture_status = data_received[4];
    result->cam_detection_result = data_received[5];

    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_Cam2_Sensor command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Raw_Cam2_Sensor() {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_RAW_CAM2_SENSOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Raw_Cam_Sensor_Struct raw_cam2_sensor;
    ADCS_Pack_to_Raw_Cam2_Sensor(&data_received[0], &raw_cam2_sensor);

    WRITE_STRUCT_TO_MEMORY(raw_cam2_sensor) // memory module function

    return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Raw_Cam2_Sensor(uint8_t* data_received, ADCS_Raw_Cam_Sensor_Struct* result) {
    result->which_sensor = ADCS_Cam2_Sensor;
	result->cam_centroid_x = (int16_t) (data_received[1] << 8 | data_received[0]);
    result->cam_centroid_y = (int16_t) (data_received[3] << 8 | data_received[2]);
    result->cam_capture_status = data_received[4];
    result->cam_detection_result = data_received[5];

    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_CSS_1_to_6 command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Raw_CSS_1_to_6() {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_RAW_CSS_1_TO_6, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Raw_CSS_1_to_6_Struct raw_css;
    ADCS_Pack_to_Raw_CSS_1_to_6(&data_received[0], &raw_css);

    WRITE_STRUCT_TO_MEMORY(raw_css) // memory module function

    return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Raw_CSS_1_to_6(uint8_t* data_received, ADCS_Raw_CSS_1_to_6_Struct* result) {
    result->css1 = data_received[0];
    result->css2 = data_received[1];
    result->css3 = data_received[2];
    result->css4 = data_received[3];
    result->css5 = data_received[4];
    result->css6 = data_received[5];

    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_CSS_7_to_10 command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Raw_CSS_7_to_10() {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_RAW_CSS_7_TO_10, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Raw_CSS_7_to_10_Struct raw_css_7_to_10;
    ADCS_Pack_to_Raw_CSS_7_to_10(&data_received[0], &raw_css_7_to_10);

    WRITE_STRUCT_TO_MEMORY(raw_css_7_to_10) // memory module function

    return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Raw_CSS_7_to_10(uint8_t* data_received, ADCS_Raw_CSS_7_to_10_Struct* result) {
    result->css7 = data_received[0];
    result->css8 = data_received[1];
    result->css9 = data_received[2];
    result->css10 = data_received[3];

    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_CubeControl_Current command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_CubeControl_Current() {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_CUBECONTROL_CURRENT_MEASUREMENTS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_CubeControl_Current_Struct cubecontrol_current;
    ADCS_Pack_to_CubeControl_Current(&data_received[0], &cubecontrol_current);

    WRITE_STRUCT_TO_MEMORY(cubecontrol_current) // memory module function

    return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_CubeControl_Current(uint8_t* data_received, ADCS_CubeControl_Current_Struct* result) {
    // everything in mA after multiplying RAWVAL*0.48828125 (aka dividing by 2.048 exactly)
	result->cubecontrol_3v3_current = ((double) ((uint16_t) (data_received[1] << 8 | data_received[0]))) / 2.048;
    result->cubecontrol_5v_current = ((double) ((uint16_t) (data_received[3] << 8 | data_received[2]))) / 2.048;
    result->cubecontrol_vbat_current = ((double) ((uint16_t) (data_received[5] << 8 | data_received[4]))) / 2.048;

    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_GPS_Status command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Raw_GPS_Status() {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_RAW_GPS_STATUS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Raw_GPS_Status_Struct raw_gps_status;
    ADCS_Pack_to_Raw_GPS_Status(&data_received[0], &raw_gps_status);

    WRITE_STRUCT_TO_MEMORY(raw_gps_status) // memory module function

    return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Raw_GPS_Status(uint8_t* data_received, ADCS_Raw_GPS_Status_Struct* result) {
    result->gps_solution_status = (ADCS_GPS_Solution_Status) data_received[0];
    result->num_tracked_satellites = data_received[1];
    result->num_used_satellites = data_received[2];
    result->counter_xyz_log = data_received[3];
    result->counter_range_log = data_received[4];
    result->response_message_gps_log = data_received[5];

    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_GPS_Time command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Raw_GPS_Time() {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_RAW_GPS_TIME, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Raw_GPS_Time_Struct raw_gps_time;
    ADCS_Pack_to_Raw_GPS_Time(data_received, &raw_gps_time);

    WRITE_STRUCT_TO_MEMORY(raw_gps_time); // memory module function

    return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Raw_GPS_Time(uint8_t* data_received, ADCS_Raw_GPS_Time_Struct* result) {
    result->gps_reference_week = (uint16_t)(data_received[1] << 8 | data_received[0]);
    result->gps_time = ((uint32_t)(data_received[5] << 24 | data_received[4] << 16 | data_received[3] << 8 | data_received[2])) / 1000.0; // Convert milliseconds to seconds

    return 0; 
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_GPS_X command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Raw_GPS_X() {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_RAW_GPS_X, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Raw_GPS_Struct raw_gps_x;
    ADCS_Pack_to_Raw_GPS(ADCS_GPS_X, data_received, &raw_gps_x);

    WRITE_STRUCT_TO_MEMORY(raw_gps_x); // memory module function

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_GPS_Y command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Raw_GPS_Y() {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_RAW_GPS_Y, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Raw_GPS_Struct raw_gps_y;
    ADCS_Pack_to_Raw_GPS(ADCS_GPS_Y, data_received, &raw_gps_y);

    WRITE_STRUCT_TO_MEMORY(raw_gps_y); // memory module function

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_GPS_Z command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Raw_GPS_Z() {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_RAW_GPS_Z, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Raw_GPS_Struct raw_gps_z;
    ADCS_Pack_to_Raw_GPS(ADCS_GPS_Z, data_received, &raw_gps_z);

    WRITE_STRUCT_TO_MEMORY(raw_gps_z); // memory module function

    return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
uint8_t ADCS_Pack_to_Raw_GPS(ADCS_GPS_Axis axis, uint8_t *data_received, ADCS_Raw_GPS_Struct *result) {
    result->axis = axis;
    result->ecef_position = (int32_t)(data_received[3] << 24 | data_received[2] << 16 | 
                                               data_received[1] << 8 | data_received[0]); // ECEF Position Z [m]
	result->ecef_velocity = (int16_t)(data_received[5] << 8 | data_received[4]);  // ECEF Velocity Z [m/s]

    return 0;
}

/// @brief Instruct the ADCS to execute the ADCS_Measurements command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_Measurements() {
    uint8_t data_length = 72;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_I2C_telemetry_wrapper(ADCS_TELEMETRY_CUBEACP_ADCS_MEASUREMENTS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Measurements_Struct measurements;
    ADCS_Pack_to_Measurements(data_received, &measurements);

    WRITE_STRUCT_TO_MEMORY(measurements); // memory module function

    return tlm_status;
}

/// @brief Packs the ADCS received raw data into the appropriate structure for this command.
/// @param[in] data_received Raw data bytes obtained from the ADCS over I2C.
/// @param[out] result Structure containing the formated data for this command.
/// @return 0 once the function is finished running.
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


/* Basic Telecommand Functions */

/// @brief Sends a telecommand over I2C to the ADCS, checks that it's been acknowledged, and returns the ACK error flag.
/// @param[in] id Valid ADCS telecommand ID (see Firmware Reference Manual)
/// @param[in] data Data array to send the raw data bytes; length must be at least data_length (should contain the correct number of bytes for the given telecommand ID)
/// @param[in] data_length Length of the data array.
/// @param[in] include_checksum Tells the ADCS whether to use a CRC8 checksum; should be either ADCS_INCLUDE_CHECKSUM or ADCS_NO_CHECKSUM 
/// @return 0 if successful, 1 if invalid ID, 2 if incorrect parameter length, 3 if incorrect parameter value, and 4 if failed CRC
uint8_t ADCS_I2C_telecommand_wrapper(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum) {
	ADCS_TC_Ack_Struct ack;
	uint8_t tc_status;

    do {
		// Send telecommand
		tc_status = ADCS_send_I2C_telecommand(id, data, data_length, include_checksum);

		// Poll Acknowledge Telemetry Format until the Processed flag equals 1.
		while (!ack.processed) {
			ADCS_TC_Ack(&ack);
		} // TODO: this needs a timeout

		// Confirm telecommand validity by checking the TC Error flag of the last read TC Acknowledge Telemetry Format.
    } while (ack.error_flag == TC_Error_CRC);  // if the checksum doesn't check out, keep resending the request

	if (tc_status == 0) {
		return ack.error_flag; // if the HAL was successful and the ADCS had an error, tell us what it is
	} 

	return tc_status; // otherwise, if the HAL had an error, tell us what that is instead
}

/// @brief Sends a telemetry request over I2C to the ADCS, and resends repeatedly if the checksums don't match.
/// @param[in] id Valid ADCS telemetry request ID (see Firmware Reference Manual)
/// @param[in] include_checksum Tells the ADCS whether to use a CRC8 checksum; should be either ADCS_INCLUDE_CHECKSUM or ADCS_NO_CHECKSUM 
/// @param[in] data_length Length of the data array.
/// @param[out] data Data array to write the raw telemetry bytes to; length must be at least data_length (should contain the correct number of bytes for the given telemetry request ID)
/// @return 0 if successful, other numbers if the HAL failed to transmit or receive data. 
uint8_t ADCS_I2C_telemetry_wrapper(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum) {
    // Send telemetry request (data gets stored to input data array)
	uint8_t checksum_check = ADCS_send_I2C_telemetry_request(id, data, data_length, include_checksum);

	while (checksum_check == 4) {
		// if the checksum doesn't check out, keep resending the request
		checksum_check = ADCS_send_I2C_telemetry_request(id, data, data_length, include_checksum);
	}

	return checksum_check;
}


/// @brief Sends a telecommand over I2C to the ADCS.
/// @param[in] id Valid ADCS telecommand ID (see Firmware Reference Manual)
/// @param[in] data Data array to send the raw data bytes; length must be at least data_length (should contain the correct number of bytes for the given telecommand ID)
/// @param[in] data_length Length of the data array.
/// @param[in] include_checksum Tells the ADCS whether to use a CRC8 checksum; should be either ADCS_INCLUDE_CHECKSUM or ADCS_NO_CHECKSUM 
/// @return 0 if successful, 4 if the checksums don't match, other numbers if the HAL failed to transmit or receive data.
uint8_t ADCS_send_I2C_telecommand(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum) {
	// Telecommand Format:
	// ADCS_ESC_CHARACTER, ADCS_START_MESSAGE [uint8_t TLM/TC ID], ADCS_ESC_CHARACTER, ADCS_END_MESSAGE
	// The defines in adcs_types.h already include the 7th bit of the ID to distinguish TLM and TC

	uint8_t adcs_tc_status; 
	
	// allocate only required memory
	uint8_t buf[data_length + include_checksum]; // add additional bit for checksum if needed

	// fill buffer with data 
	for (uint8_t i = 0; i < data_length; i++) {
		buf[i] = data[i];
	}

	// include checksum following data if enabled
	if (include_checksum) {buf[data_length] = ADCS_COMMS_Crc8Checksum(data, data_length);}

	adcs_tc_status = HAL_I2C_Mem_Write(&hi2c1, ADCS_I2C_ADDRESS << 1, id, 1, buf, sizeof(buf), ADCS_HAL_TIMEOUT);

	/* When sending a command to the CubeACP, it is possible to include an 8-bit CRC checksum.
	For instance, when sending a command that has a length of 8 bytes, it is possible to include a
	9th byte that is computed from the previous 8 bytes. The extra byte will be interpreted as a
	checksum and used to validate the message. If the checksum fails, the command will be
	ignored. For I2C communication, the Tc Error Status in the Telecommand Acknowledge telemetry frame
	(Table 39: Telecommand Acknowledge Telemetry Format) will have a value of 4. */

	return adcs_tc_status;
}

/// @brief Sends a telemetry request over I2C to the ADCS.
/// @param[in] id Valid ADCS telemetry request ID (see Firmware Reference Manual)
/// @param[in] include_checksum Tells the ADCS whether to use a CRC8 checksum; should be either ADCS_INCLUDE_CHECKSUM or ADCS_NO_CHECKSUM 
/// @param[in] data_length Length of the data array.
/// @param[out] data Data array to write the raw telemetry bytes to; length must be at least data_length (should contain the correct number of bytes for the given telemetry request ID)
/// @return 0 if successful, 4 if the checksums don't match, other numbers if the HAL failed to transmit or receive data. 
uint8_t ADCS_send_I2C_telemetry_request(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum) {
	// Telemetry Request Format:
	// Note: requires a repeated start condition; data_length is number of bits to read.
	// [start], ADCS_I2C_WRITE, id, [start] ADCS_I2C_READ, [read all the data], [stop]
	// The defines in adcs_types.h already include the 7th bit of the ID to distinguish TLM and TC

	/* When requesting telemetry through I2C, it is possible to read one extra byte past the allowed
	length of the telemetry frame. In this case, the extra byte will also be an 8-bit checksum
	computed by the CubeACP and can be used by the interfacing OBC to validate the message.*/
	uint8_t adcs_tlm_status;

	//Allocate only required memory
	uint8_t temp_data[data_length + include_checksum];
		// temp data used for checksum checking

	adcs_tlm_status = HAL_I2C_Mem_Read(&hi2c1, ADCS_I2C_ADDRESS << 1, id, 1, temp_data, sizeof(temp_data), ADCS_HAL_TIMEOUT);

	for (uint8_t i = 0; i < data_length; i++) {
			// populate external data, except for checksum byte
			data[i] = temp_data[i];
	}

	if (include_checksum) {
		uint8_t checksum = temp_data[data_length];
		uint8_t checksum_test = ADCS_COMMS_Crc8Checksum(data, data_length);
		if (checksum != checksum_test) {
			return 0x04;
		}
	}

	return adcs_tlm_status;

}

uint8_t ADCS_send_UART_telecommand(UART_HandleTypeDef *huart, uint8_t id, uint8_t* data, uint32_t data_length) {
	// WARNING: DEPRECATED FUNCTION
	// This function is incomplete, and will not be updated.
	// USE AT YOUR OWN RISK.

	// Telemetry Request or Telecommand Format:
	// ADCS_ESC_CHARACTER, ADCS_START_MESSAGE [uint8_t TLM/TC ID], ADCS_ESC_CHARACTER, ADCS_END_MESSAGE
	// The defines in adcs_types.h already include the 7th bit of the ID to distinguish TLM and TC
	// data bytes can be up to a maximum of 8 bytes; data_length ranges from 0 to 8

	//Check id to identify if it's Telecommand or Telemetry Request
	uint8_t telemetry_request = id & 128; // 1 = TLM, 0 = TC

	//Allocate only required memory by checking first bit of ID
	uint8_t buf[5 + (!telemetry_request)*data_length];

	//Fill buffer with ESC, SOM and ID
	buf[0] = ADCS_ESC_CHARACTER;
	buf[1] = ADCS_START_MESSAGE;
	buf[2] = id;

	if (telemetry_request) {
		//If transmitting Telemetry Request
		//Fill buffer with ESC and EOM without data_length
		buf[3] = ADCS_ESC_CHARACTER;
		buf[4] = ADCS_END_MESSAGE;
	} else {
		//Fill buffer with Data if transmitting a Telecommand
		for (uint8_t i = 0; i < data_length; i++) {
			buf[i + 3] = data[i];
		}
		//Fill buffer with ESC and EOM
		buf[3 + data_length] = ADCS_ESC_CHARACTER;
		buf[4 + data_length] = ADCS_END_MESSAGE;
	}

	//Transmit the TLM or TC via UART
	HAL_UART_Transmit(huart, buf, strlen((char*)buf), HAL_MAX_DELAY);

	//receiving from telecommand: data is one byte exactly
	//receiving from telemetry request: data is up to 8 bytes

	//Allocate only required memory
	uint8_t buf_rec[6 + (telemetry_request)*(data_length-1)];

	//Start receiving acknowledgment or reply from the CubeComputer
	HAL_UART_Receive(huart, buf_rec, strlen((char*)buf_rec), HAL_MAX_DELAY);

	if (telemetry_request) {
		//Ignoring ESC, EOM, SOM and storing the rest of the values in data
		for (uint8_t i = 3; i < sizeof(buf_rec)-2; i++) {
			// put the data into the data array excluding TC ID or TLM ID
			data[i-3] = buf_rec[i];
		}

		return 0x00;
	}

	return buf_rec[3]; // buf_rec[3] contains the TC Error Flag

  // The reply will contain two data bytes, the last one being the TC Error flag.
  // The receipt of the acknowledge will indicate that another telecommand may be sent.
  // Sending another telecommand before the acknowledge will corrupt the telecommand buffer.
}

/// @brief Swap low and high bytes of uint16 to turn into uint8 and put into specified index of an array
/// @param[in] value Value to split and swap the order of.
/// @param[in] index Index in array to write the result. (Array must contain at least two bytes, with index pointing to the first)
/// @param[out] array Data array to write the two bytes to at the specified index and index + 1.
/// @return 0 once complete.
uint8_t ADCS_switch_order(uint8_t *array, uint16_t value, int index) {
    array[index] = (uint8_t)(value & 0xFF); // Insert the low byte of the value into the array at the specified index
    array[index + 1] = (uint8_t)(value >> 8); // Insert the high byte of the value into the array at the next index
	return 0;
}

// Swap low and high bytes of uint32 to turn into uint8 and put into specified index of an array
/// @brief Swap low and high bytes of uint32 to turn into uint8 and put into specified index of an array
/// @param[in] value Value to split and swap the order of.
/// @param[in] index Index in array to write the result. (Array must contain at least four bytes, with index pointing to the first)
/// @param[out] array Data array to write the four bytes to at the specified index and the three subsequent indices.
/// @return 0 once complete.
uint8_t ADCS_switch_order_32(uint8_t *array, uint32_t value, int index) {
    array[index] = (uint8_t)(value & 0xFF); // Insert the low byte of the value into the array at the specified index
    array[index + 1] = (uint8_t)((value >> 8) & 0xFF); // Insert the second byte of the value into the array at the next index
	array[index + 2] = (uint8_t)((value >> 16) & 0xFF); // Insert the third byte of the value into the array at the next next index
    array[index + 3] = (uint8_t)(value >> 24); // Insert the high byte of the value into the array at the next next next index
	return 0;
}

/// @brief Initialise the lookup table for 8-bit CRC calculation.
/// @return 0 once successful.
uint8_t CRC8Table[256];
uint8_t ADCS_COMMS_Crc8Init()
	{
	int val;
	for (int i = 0; i < 256; i++)
	{
		val = i;
		for (int j = 0; j < 8; j++)
		{
			if (val & 1)
			val ^= CRC_POLY;
			val >>= 1;
		}
		CRC8Table[i] = val;
	}
	return 0;
}

/// @brief Calculates an 8-bit CRC value
/// @param[in] buffer the buffer containing data for which to calculate the crc value
/// @param[in] len the number of bytes of valid data in the buffer
/// @return the CRC value calculated (which is 0xFF for an empty buffer)
uint8_t ADCS_COMMS_Crc8Checksum(uint8_t* buffer, uint16_t len)
{
	if (len == 0) return 0xff;

	uint16_t i;
	uint8_t crc = 0;

	for (i = 0; i < len; i++)
		crc = CRC8Table[crc ^ buffer[i]];

	return crc;
}