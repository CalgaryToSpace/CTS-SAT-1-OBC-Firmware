/*
 * adcs_types.c
 *
 *  Created on: Mar 7, 2024
 *      Author: Nadeem Moosa, Saksham Puri, Zachary Uy
 */

#include "adcs_types.h"
#include "adcs_ids.h"
#include <string.h>

/*
 * Summary list of required command IDs:

	TODO: (Commands to write)
	Commands: 
	Telemetry: 155, 156, 157, 170, 201, 204

	Done:
	Untested: 7, 9, 145, 45, 207, 64 [next to make: 155]
	Tested: 10, 11, 13, 14, 17, 26, 63, 147, 150, 197, 240

	- within a byte, use the opposite endian-ness (first towards the end, last towards the beginning of the byte)
	- check INT vs UINT (int is signed, uint is unsigned)!
 *  */

ADCS_TC_Ack_Struct ADCS_TC_Ack(I2C_HandleTypeDef *hi2c) {
	//note return value -- this is a special case of telemetry request for telecommands
	ADCS_TC_Ack_Struct ack;
	uint8_t data_received[8]; // define temp buffer
	uint8_t data_length = 4;

	I2C_telemetry_wrapper(hi2c, TLF_TC_ACK, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

	// map temp buffer to Ack struct
	ack.last_id = data_received[0];
	ack.processed = data_received[1];
	ack.error_flag = data_received[2];
	ack.error_index = data_received[3];

	WRITE_STRUCT_TO_MEMORY(ack) // memory module function
	return ack;
}

void ADCS_Reset(I2C_HandleTypeDef *hi2c) {
	// returns telecommand error flag
	uint8_t data_send[1] = {ADCS_MAGIC_NUMBER};
	I2C_telecommand_wrapper(hi2c, TC_RESET, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
}

void ADCS_Identification(I2C_HandleTypeDef *hi2c) {
	// TODO: Follow this format for all other telemetry requests!
	ADCS_ID_Struct id;
	uint8_t data_length = 8;
	uint8_t data_received[data_length]; // define temp buffer

	I2C_telemetry_wrapper(hi2c, TLF_IDENTIFICATION, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

	// map temp buffer to Identification struct
	id.node_type = data_received[0];
	id.interface_version = data_received[1];
	id.major_firmware_version = data_received[2];
	id.minor_firmware_version = data_received[3];
	id.seconds_since_startup = data_received[5] << 8 | data_received[4]; // uint16_t
	id.ms_past_second = data_received[7] << 8 | data_received[6]; // uint16_t

	WRITE_STRUCT_TO_MEMORY(id) // memory module function
}

void ADCS_Program_Status(I2C_HandleTypeDef *hi2c) {
	ADCS_Boot_Running_Status_Struct status;
	uint8_t data_length = 6;
	uint8_t data_received[data_length]; // define temp buffer

	I2C_telemetry_wrapper(hi2c, TLF_BOOT_RUNNING_PROGRAM_STATUS, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

	// map to struct
	status.reset_cause = data_received[0] & 0xF0; // takes upper four bits of byte 0
	status.boot_cause = data_received[0] & 0x0F; // take upper four bits of byte 0
	status.boot_counter = data_received[2] << 8 | data_received[1]; // uint16_t
	status.boot_program_index = data_received[3];
	status.major_firmware_version = data_received[2]; // uint8_t
	status.minor_firmware_version = data_received[3]; // uint8_t

	WRITE_STRUCT_TO_MEMORY(status) // memory module function
}

void ADCS_Communication_Status(I2C_HandleTypeDef *hi2c) {
	// returns I2C communication status of the ADCS (Table 37)
	ADCS_Comms_Status_Struct status;
	uint8_t data_length = 6;
	uint8_t data_received[data_length]; // define temp buffer

	I2C_telemetry_wrapper(hi2c, TLF_SATSTATE_COMM_STATUS, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

	// map to struct
	status.tc_counter = data_received[1] << 8 | data_received[0]; // uint16_t
	status.tlm_counter = data_received[3] << 8 | data_received[2]; // uint16_t
	status.tc_buffer_overrun = data_received[4] & 0b10000000; // bit 0 is 1 if TC buffer was overrun while receiving a telecommand
	status.i2c_tlm_error = data_received[4] & 0b00010000; // bit 3 is 1 if the number of data clocked out was more than the telemetry package
	status.i2c_tc_error = data_received[4] & 0b00001000; // bit 4 is 1 if the telecommand sent exceeded the buffer size
	// other bits are for UART and CAN; ignore them

	WRITE_STRUCT_TO_MEMORY(status) // memory module function
}

void ADCS_Deploy_Magnetometer(I2C_HandleTypeDef *hi2c, uint8_t deploy_timeout) {
	// Deploys the magnetometer boom, timeout in seconds
	uint8_t data_send[1] = {deploy_timeout};
	I2C_telecommand_wrapper(hi2c, TC_DEPLOY_MAGNETOMETER_BOOM, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
}

void ADCS_Set_Run_Mode(I2C_HandleTypeDef *hi2c, ADCS_Run_Mode mode) {
	// Disables the ADCS
	uint8_t data_send[1] = {mode};
	I2C_telecommand_wrapper(hi2c, TC_ADCS_RUN_MODE, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
}

void ADCS_Clear_Errors(I2C_HandleTypeDef *hi2c) {
	// Clears error flags
	// NOTE: THERE IS ANOTHER, SEPARATE CLEAR ERROR FLAG TC FOR THE BOOTLODER (TC_BL_CLEAR_ERRORS)
	uint8_t data_send[1] = {0b11000000};
	I2C_telecommand_wrapper(hi2c, TC_CLEAR_ERRORS, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
}

void ADCS_Attitude_Control_Mode(I2C_HandleTypeDef *hi2c, ADCS_Control_Mode mode, uint16_t timeout) {
	// Sets the ADCS attitude control mode
	// See User Manual, Section 4.4.3 Table 3 for requirements to switch control mode
	uint8_t data_send[3] = {mode, timeout & 0x00FF, timeout >> 8};
	I2C_telecommand_wrapper(hi2c, TC_SET_ATTITUDE_CONTROL_MODE, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
}

void ADCS_Attitude_Estimation_Mode(I2C_HandleTypeDef *hi2c, ADCS_Estimation_Mode mode) {
	// Sets the ADCS attitude estimation mode
	// Possible values for mode given in Section 6.3 Table 80 of Firmware Reference Manual (ranges from 0 to 7)
	// needs power control to be on
	uint8_t data_send[1] = {mode};
	I2C_telecommand_wrapper(hi2c, TC_SET_ATTITUDE_ESTIMATION_MODE, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
}

void ADCS_Run_Once(I2C_HandleTypeDef *hi2c) {
	// requires ADCS_Enable_Triggered to have run first
	// (if ADCS_Enable_On has run instead, then this is unnecessary)
	uint8_t data_send[0];
	I2C_telecommand_wrapper(hi2c, TC_TRIGGER_ADCS_LOOP, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
}

void ADCS_Set_Magnetometer_Mode(I2C_HandleTypeDef *hi2c, ADCS_Magnetometer_Mode mode) {
	uint8_t data_send[1] = {mode};
	I2C_telecommand_wrapper(hi2c, TC_SET_MODE_OF_MAGNETOMETER_OPERATION, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
}

void ADCS_Set_Magnetorquer_Output(I2C_HandleTypeDef *hi2c, uint16_t x_duty, uint16_t y_duty, uint16_t z_duty) {
	// only valid after ADCS_Enable_Manual_Control is run
	// for the duty equations, raw parameter value is obtained using the formula: (raw parameter) = (formatted value)*1000.0
	// duty >> 8 gives upper byte, duty & 0x00FF gives lower byte
	uint8_t data_send[6];
	// swap low and high bytes and populate data_send
	switch_order(data_send, x_duty, 0);
	switch_order(data_send, y_duty, 2);
	switch_order(data_send, z_duty, 4);
	I2C_telecommand_wrapper(hi2c, TC_SET_MAGNETORQUER_OUTPUT, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
}

void ADCS_Set_Wheel_Speed(I2C_HandleTypeDef *hi2c, uint16_t x_speed, uint16_t y_speed, uint16_t z_speed) {
	// only valid after ADCS_Enable_Manual_Control is run
	// for the duty equations, raw parameter value is in rpm
	uint8_t data_send[6]; // 6-byte data to send
	// swap low and high bytes and populate data_send
	switch_order(data_send, x_speed, 0);
	switch_order(data_send, y_speed, 2);
	switch_order(data_send, z_speed, 4);
	I2C_telecommand_wrapper(hi2c, TC_SET_WHEEL_SPEED, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
}

void ADCS_Set_Power_Control(I2C_HandleTypeDef *hi2c, ADCS_Power_Select cube_control_signal, ADCS_Power_Select cube_control_motor, ADCS_Power_Select cube_sense1,
        ADCS_Power_Select cube_sense2, ADCS_Power_Select cube_star_power, ADCS_Power_Select cube_wheel1_power,
        ADCS_Power_Select cube_wheel2_power, ADCS_Power_Select cube_wheel3_power, ADCS_Power_Select motor_power,
        ADCS_Power_Select gps_power) {
	uint8_t data_send[3]; // 3-byte data (from manual)
	// within the byte, everything goes in reverse order!!
	data_send[0] = (cube_control_signal) | (cube_control_motor << 2) | (cube_sense1 << 4) | (cube_sense2 << 6);
	data_send[1] = (cube_star_power) | (cube_wheel1_power << 2) | (cube_wheel2_power << 4) | (cube_wheel3_power << 6);
	data_send[2] = (motor_power) | (gps_power << 2);
	I2C_telecommand_wrapper(hi2c, TC_CUBEACP_SET_ADCS_POWER_CONTROL, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
}

void ADCS_Get_Power_Control(I2C_HandleTypeDef *hi2c) {
	// returns power control status (Table 184)
	ADCS_Power_Control_Struct power;
	uint8_t data_length = 3;
	uint8_t data_received[data_length]; // define temp buffer

	I2C_telemetry_wrapper(hi2c, TLF_CUBEACP_GET_ADCS_POWER_CONTROL, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

	// map to struct; all of these are two-bit enums
	// within the byte, everything goes in reverse order!!
	power.cube_control_signal = (data_received[0]) & 0x03;
	power.cube_control_motor = (data_received[0] >> 2) & 0x03;
	power.cube_sense1 = (data_received[0] >> 4) & 0x03;
	power.cube_sense2 = (data_received[0] >> 6) & 0x03;

	power.cube_star_power = (data_received[1]) & 0x03;
	power.cube_wheel1_power = (data_received[1] >> 2) & 0x03;
	power.cube_wheel2_power =  (data_received[1] >> 4) & 0x03;
	power.cube_wheel3_power = (data_received[1] >> 6) & 0x03;

	power.motor_power = (data_received[2]) & 0x03;
	power.gps_power = (data_received[2] >> 2) & 0x03;

	WRITE_STRUCT_TO_MEMORY(status) // memory module function
}

void ADCS_Set_Magnetometer_Config(I2C_HandleTypeDef *hi2c,
		uint16_t mounting_transform_alpha_angle,
        uint16_t mounting_transform_beta_angle,
        uint16_t mounting_transform_gamma_angle,
        uint16_t channel_1_offset,
        uint16_t channel_2_offset,
        uint16_t channel_3_offset,
        uint16_t sensitivity_matrix_s11,
        uint16_t sensitivity_matrix_s22,
        uint16_t sensitivity_matrix_s33,
        uint16_t sensitivity_matrix_s12,
        uint16_t sensitivity_matrix_s13,
        uint16_t sensitivity_matrix_s21,
        uint16_t sensitivity_matrix_s23,
        uint16_t sensitivity_matrix_s31,
        uint16_t sensitivity_matrix_s32) {

	uint8_t data_send[30]; // 30-byte data (from manual)

	// reorder uint8_t bytes to be low-byte then high-byte
	// actual config is raw value divided by 100 for mounting transform angles
	// and raw value divided by 1000 for everything else
	// these are all INT, not UINT! 
	// TODO: change this after testing
	switch_order(data_send, mounting_transform_alpha_angle, 0);
	switch_order(data_send, mounting_transform_beta_angle, 2);
	switch_order(data_send, mounting_transform_gamma_angle, 4);
	switch_order(data_send, channel_1_offset, 6);
	switch_order(data_send, channel_2_offset, 8);
	switch_order(data_send, channel_3_offset, 10);
	switch_order(data_send, sensitivity_matrix_s11, 12);
	switch_order(data_send, sensitivity_matrix_s22, 14);
	switch_order(data_send, sensitivity_matrix_s33, 16);
	switch_order(data_send, sensitivity_matrix_s12, 18);
	switch_order(data_send, sensitivity_matrix_s13, 20);
	switch_order(data_send, sensitivity_matrix_s21, 22);
	switch_order(data_send, sensitivity_matrix_s23, 24);
	switch_order(data_send, sensitivity_matrix_s31, 26);
	switch_order(data_send, sensitivity_matrix_s32, 28);

	I2C_telecommand_wrapper(hi2c, TC_CUBEACP_SET_MAGNETOMETER_CONFIG, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);

}

void ADCS_Save_Config(I2C_HandleTypeDef *hi2c) {
	uint8_t data_send[0]; // 0-byte data (from manual)
	I2C_telecommand_wrapper(hi2c, TC_SAVE_CONFIG, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
}

void ADCS_Save_Orbit_Params(I2C_HandleTypeDef *hi2c) {
	uint8_t data_send[0]; // 0-byte data (from manual)
	I2C_telecommand_wrapper(hi2c, TC_SAVE_ORBIT_PARAMS, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
}

void ADCS_Estimate_Angular_Rates(I2C_HandleTypeDef *hi2c) {
	// gets estimated angular rates of the ADCS (Table 103)
	ADCS_Angular_Rates_Struct rates;

	uint8_t data_length = 6;
	uint8_t data_received[data_length]; // define temp buffer

	I2C_telemetry_wrapper(hi2c, TLF_CUBEACP_ESTIMATED_ANGULAR_RATES, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

	// map to struct
	// Formatted value is obtained using the formula: (formatted value) [deg/s] = RAWVAL*0.01
	rates.x_rate = data_received[1] << 8 | data_received[0]; // uint16_t
	rates.y_rate = data_received[3] << 8 | data_received[2]; // uint16_t
	rates.z_rate = data_received[5] << 8 | data_received[4]; // uint16_t

	WRITE_STRUCT_TO_MEMORY(rates) // memory module function
}

void ADCS_Get_LLH_Position(I2C_HandleTypeDef *hi2c) {
	// gets ADCS position in WGS-84 coords (Table 106)
	ADCS_LLH_Position_Struct pos;

	uint8_t data_length = 6;
	uint8_t data_received[data_length]; // define temp buffer

	I2C_telemetry_wrapper(hi2c, TLF_CUBEACP_SATELLITE_POSITION_LLH, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

	// map to struct
	// Formatted value is obtained using the formula: (formatted value) [deg] or [km] = RAWVAL*0.01
	pos.latitude = data_received[1] << 8 | data_received[0]; // int16_t (signed)
	pos.longitude = data_received[3] << 8 | data_received[2]; // int16_t (signed)
	pos.altitude = data_received[5] << 8 | data_received[4]; // uint16_t (unsigned)

	WRITE_STRUCT_TO_MEMORY(rates) // memory module function
}

void ADCS_Bootloader_Clear_Errors(I2C_HandleTypeDef *hi2c) {
	uint8_t data_send[0]; // 0-byte data (from manual)
	I2C_telecommand_wrapper(hi2c, TC_BL_CLEAR_ERRORS, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
}

void ADCS_Set_Unix_Time_Save_Mode(I2C_HandleTypeDef *hi2c, bool save_now, bool save_on_update, bool save_periodic, uint8_t period) {
	uint8_t data_send[2] = { (save_now | (save_on_update << 1) | (save_periodic << 2) ) , period}; // 2-byte data (from manual)
	I2C_telecommand_wrapper(hi2c, TC_SET_UNIX_TIME_SAVE_TO_FLASH, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
}

void ADCS_Get_Unix_Time_Save_Mode(I2C_HandleTypeDef *hi2c) {
	ADCS_Set_Unix_Time_Save_Mode_Struct mode;

	uint8_t data_length = 2;
	uint8_t data_received[data_length]; // define temp buffer

	I2C_telemetry_wrapper(hi2c, TLF_GET_UNIX_TIME_SAVE_TO_FLASH, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

	mode.save_now = data_received[0] & 0b00000001;
	mode.save_on_update = data_received[0] & 0b00000010;
	mode.save_periodic = data_received[0] & 0b00000100;

	mode.period = data_received[1];

	WRITE_STRUCT_TO_MEMORY(mode) // memory module function
}

void ADCS_Set_SGP4_Orbit_Params(I2C_HandleTypeDef *hi2c, double inclination, double eccentricity, double ascending_node_right_ascension,
														//  degrees,					dimensionless, 		degrees
		double perigee_argument, double b_star_drag_term, double mean_motion, double mean_anomaly, double epoch) {
		// degrees,					dimensionless,			orbits/day,			degrees,			years.days

	uint8_t data_send[64] = {};

	// convert doubles to arrays of uint8_t
	memcpy(&data_send[0], &inclination, sizeof(inclination));
	memcpy(&data_send[8], &eccentricity, sizeof(eccentricity));
	memcpy(&data_send[16], &ascending_node_right_ascension, sizeof(ascending_node_right_ascension));
	memcpy(&data_send[24], &perigee_argument, sizeof(perigee_argument));
	memcpy(&data_send[32], &b_star_drag_term, sizeof(b_star_drag_term));
	memcpy(&data_send[40], &mean_motion, sizeof(mean_motion));
	memcpy(&data_send[48], &mean_anomaly, sizeof(mean_anomaly));
	memcpy(&data_send[56], &epoch, sizeof(epoch));

	I2C_telecommand_wrapper(hi2c, TC_CUBEACP_SET_SGP4_ORBIT_PARAMETERS, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
}

void ADCS_Get_SGP4_Orbit_Params(I2C_HandleTypeDef *hi2c) {
	ADCS_Orbit_Params_Struct params;
	uint8_t data_length = 64;
	uint8_t data_received[data_length]; // define temp buffer

	I2C_telemetry_wrapper(hi2c, TLF_CUBEACP_GET_SGP4_ORBIT_PARAMETERS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

	// map temp buffer to orbit params struct
	memcpy(&params.inclination, &data_received[0] ,sizeof(double));
	memcpy(&params.eccentricity, &data_received[8] ,sizeof(double));
	memcpy(&params.ascending_node_right_ascension, &data_received[16] ,sizeof(double));
	memcpy(&params.perigee_argument, &data_received[24] ,sizeof(double));
	memcpy(&params.b_star_drag_term, &data_received[32] ,sizeof(double));
	memcpy(&params.mean_motion, &data_received[40] ,sizeof(double));
	memcpy(&params.mean_anomaly, &data_received[48] ,sizeof(double));
	memcpy(&params.epoch, &data_received[56] ,sizeof(double));

	WRITE_STRUCT_TO_MEMORY(params) // memory module function
}


/**
 * I2C_telecommand_wrapper
 * REQUIRES:
 *  - hi2c points to valid I2C handle (ex. &hi2c1)
 *  - id is a valid telecommand ID (see Firmware Reference Manual)
 *  - data points to array of uint8_t with length at least data_length (should contain the correct number of bytes for the given telecommand ID)
 *  - include_checksum is either ADCS_INCLUDE_CHECKSUM or ADCS_NO_CHECKSUM
 * PROMISES:
 * 	- sends telecommand to ADCS and checks that it has been acknowledged
 * 	- returns the error flag from the result of the transmission
 */
void I2C_telecommand_wrapper(I2C_HandleTypeDef *hi2c, uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum) {
	ADCS_TC_Ack_Struct ack;

    do {
		// Send telecommand
		send_I2C_telecommand(hi2c, id, data, data_length, include_checksum);

		// Poll Acknowledge Telemetry Format until the Processed flag equals 1.
		while (!ack.processed) {
			ack = ADCS_TC_Ack(hi2c);
		}

		// Confirm telecommand validity by checking the TC Error flag of the last read TC Acknowledge Telemetry Format.
    } while (ack.error_flag == TC_Error_CRC);  // if the checksum doesn't check out, keep resending the request

}

/**
 * I2C_telemetry_wrapper
 * REQUIRES:
 *  - hi2c points to valid I2C handle (ex. &hi2c1)
 *  - id is a valid telemetry request ID (see Firmware Reference Manual)
 *  - data points to array of uint8_t with length at least data_length (should contain the correct number of bytes for the given telemetry request ID)
 *  - include_checksum is either ADCS_INCLUDE_CHECKSUM or ADCS_NO_CHECKSUM
 * PROMISES:
 * 	- Return value is 1 if checksum value doesn't match, 0 for any other successful transmission (including with no checksum)
 * 	- data array filled with result of transmission
 */
void I2C_telemetry_wrapper(I2C_HandleTypeDef *hi2c, uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum) {
    // Send telemetry request (data gets stored to input data array)
	uint8_t checksum_check = send_I2C_telemetry_request(hi2c, id, data, data_length, include_checksum);

	while (checksum_check == 1) {
		// if the checksum doesn't check out, keep resending the request
		checksum_check = send_I2C_telemetry_request(hi2c, id, data, data_length, include_checksum);
	}
}


/**
 * send_I2C_telecommand
 * REQUIRES:
 *  - hi2c points to valid I2C handle (ex. &hi2c1)
 *  - id is a valid telecommand ID (see Firmware Reference Manual)
 *  - data points to array of uint8_t with length at least data_length (should contain the correct number of bytes for the given telecommand ID)
 *  - include_checksum is either ADCS_INCLUDE_CHECKSUM or ADCS_NO_CHECKSUM
 * PROMISES:
 * 	- Transmits data to the ADCS over I2C
 */
void send_I2C_telecommand(I2C_HandleTypeDef *hi2c, uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum) {
	// Telecommand Format:
	// ADCS_ESC_CHARACTER, ADCS_START_MESSAGE [uint8_t TLM/TC ID], ADCS_ESC_CHARACTER, ADCS_END_MESSAGE
	// The defines in adcs_types.h already include the 7th bit of the ID to distinguish TLM and TC

	//Allocate only required memory
	uint8_t buf[data_length + include_checksum]; // add additional bit for checksum if needed

	// Fill buffer with Data if transmitting a Telecommand
	for (int i = 0; i < data_length; i++) {
		buf[i] = data[i];
	}

	// include checksum following data if enabled
	if (include_checksum) {buf[data_length] = COMMS_Crc8Checksum(data, data_length);}

	while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY) {} // wait until ready
	HAL_I2C_Mem_Write_IT(hi2c, ADCS_I2C_ADDRESS << 1, id, 1, buf, sizeof(buf));
	while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY) {} // wait until finished
	//HAL_I2C_Master_Transmit(hi2c, ADCS_I2C_ADDRESS, buf, sizeof(buf), HAL_MAX_DELAY);

	/* When sending a command to the CubeACP, it is possible to include an 8-bit CRC checksum.
	For instance, when sending a command that has a length of 8 bytes, it is possible to include a
	9th byte that is computed from the previous 8 bytes. The extra byte will be interpreted as a
	checksum and used to validate the message. If the checksum fails, the command will be
	ignored. For I2C communication, the Tc Error Status in the Telecommand Acknowledge telemetry frame
	(Table 39: Telecommand Acknowledge Telemetry Format) will have a value of 4. */

}

/**
 * send_I2C_telemetry_request
 * REQUIRES:
 *  - hi2c points to valid I2C handle (ex. &hi2c1)
 *  - id is a valid telemetry request ID (see Firmware Reference Manual)
 *  - data points to array of uint8_t with length at least data_length (should contain the correct number of bytes for the given telemetry request ID)
 *  - include_checksum is either ADCS_INCLUDE_CHECKSUM or ADCS_NO_CHECKSUM
 * PROMISES:
 * 	- Return value is 1 if checksum value doesn't match, 0 for any other successful transmission (including with no checksum)
 * 	- data array filled with result of transmission
 */
uint8_t send_I2C_telemetry_request (I2C_HandleTypeDef *hi2c, uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum) {
	// Telemetry Request Format:
	// Note: requires a repeated start condition; data_length is number of bits to read.
	// [start], ADCS_I2C_WRITE, id, [start] ADCS_I2C_READ, [read all the data], [stop]
	// The defines in adcs_types.h already include the 7th bit of the ID to distinguish TLM and TC

	/* When requesting telemetry through I2C, it is possible to read one extra byte past the allowed
	length of the telemetry frame. In this case, the extra byte will also be an 8-bit checksum
	computed by the CubeACP and can be used by the interfacing OBC to validate the message.*/

	//Allocate only required memory
	uint8_t temp_data[data_length + include_checksum];
		// temp data used for checksum checking

	while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY) {} // wait until ready
	HAL_I2C_Mem_Read_IT(hi2c, ADCS_I2C_ADDRESS << 1, id, 1, temp_data, sizeof(temp_data));
	// read the data using the EEPROM protocol (handled by built-in Mem_Read function)
		// ADCS_I2C_ADDRESS << 1 = ADCS_I2C_WRITE, and (ADCS_I2C_ADDRESS << 1) | 0x01 = ADCS_I2C_READ
	while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY) {} // wait until finished

	for (int i = 0; i < data_length; i++) {
			// populate external data, except for checksum byte
			data[i] = temp_data[i];
	}

	if (include_checksum) {
		uint8_t checksum = temp_data[data_length];
		uint8_t checksum_test = COMMS_Crc8Checksum(data, data_length);
		if (checksum != checksum_test) {
			return 1;
		}
	}

	return 0;

}

uint8_t send_UART_telecommand(UART_HandleTypeDef *huart, uint8_t id, uint8_t* data, uint32_t data_length) {
	// WARNING: DEPRECATED FUNCTION
	// This function is incomplete, and will not be updated.
	// USE AT YOUR OWN RISK.

	// Telemetry Request or Telecommand Format:
	// ADCS_ESC_CHARACTER, ADCS_START_MESSAGE [uint8_t TLM/TC ID], ADCS_ESC_CHARACTER, ADCS_END_MESSAGE
	// The defines in adcs_types.h already include the 7th bit of the ID to distinguish TLM and TC
	// data bytes can be up to a maximum of 8 bytes; data_length ranges from 0 to 8

	//Check id to identify if it's Telecommand or Telemetry Request
	uint8_t telemetry_request = id & 0b10000000; // 1 = TLM, 0 = TC

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
		for (int i = 0; i < data_length; i++) {
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
		for (int i = 3; i < sizeof(buf_rec)-2; i++) {
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


// CRC initialisation
// init lookup table for 8-bit crc calculation

uint8_t CRC8Table[256];

void COMMS_Crc8Init()
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
}


// Swap low and high bytes of uint16 to turn into uint8 and put into specified index of an array
void switch_order(uint8_t *array, uint16_t value, int index) {
    array[index] = (uint8_t)(value & 0xFF); // Insert the low byte of the value into the array at the specified index
    array[index + 1] = (uint8_t)(value >> 8); // Insert the high byte of the value into the array at the next index
}


/***************************************************************************//**
* Calculates an 8-bit CRC value
*
* @param[in] buffer
* the buffer containing data for which to calculate the crc value
* @param[in] len
* the number of bytes of valid data in the buffer
******************************************************************************/
uint8_t COMMS_Crc8Checksum(uint8_t* buffer, uint16_t len)
{
	if (len == 0) return 0xff;

	uint16_t i;
	uint8_t crc = 0;

	for (i = 0; i < len; i++)
		crc = CRC8Table[crc ^ buffer[i]];

	return crc;
}


//Debug function to print a new line (\n) in UART
void PRINT_NEW_LINE(UART_HandleTypeDef *huart) {
    char buf[] = "\r\n";
    HAL_UART_Transmit(huart, (uint8_t*) buf, strlen(buf), 100);
}

//Debug function to print a given string to UART
void PRINT_STRING_UART(UART_HandleTypeDef *huart, void *string) {

//    char *buff = (char*) string;
    HAL_UART_Transmit(huart, (uint8_t*) string, strlen((char*) string), 100);
    PRINT_NEW_LINE(huart);
//    memset(string, 0, strlen((char*) string));
}

