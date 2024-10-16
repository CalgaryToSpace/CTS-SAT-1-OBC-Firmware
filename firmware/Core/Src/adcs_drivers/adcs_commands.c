/*
 * adcs_types.c
 *
 *  Created on: Mar 7, 2024
 *      Author: Nadeem Moosa, Saksham Puri, Zachary Uy
 */

#include "adcs_drivers/adcs_types.h"
#include "adcs_drivers/adcs_command_ids.h"
#include "adcs_drivers/adcs_struct_packers.h"
#include "adcs_drivers/adcs_types_to_json.h"
#include "adcs_drivers/adcs_commands.h"
#include "adcs_drivers/adcs_internal_drivers.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "stm32l4xx_hal.h"

extern I2C_HandleTypeDef hi2c1; // allows not needing the parameters

/// @brief Instructs the ADCS to determine whether the last command succeeded. (Doesn't work for telemetry requests, by design.)
/// @param[out] ack Structure containing the formatted information about the last command sent.
/// @return 0 if successful, non-zero if an error occurred in transmission.
uint8_t ADCS_cmd_ack(ADCS_cmd_ack_struct_t *ack) {
    uint8_t data_received[8]; // define temp buffer
    uint8_t data_length = 4;

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_COMMAND_ACK, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

    // map temp buffer to Ack struct
    ADCS_Pack_to_Ack_Struct(&data_received[0], ack);

    if (tlm_status == 0) {
        return ack->error_flag;
    }

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Reset command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_reset() {
    // returns telecommand error flag
    uint8_t data_send[1] = {ADCS_MAGIC_NUMBER};
    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_RESET, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Identification command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_identification(ADCS_id_struct_t *output_struct) {

    uint8_t data_length = 8;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_IDENTIFICATION, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Identification_Struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Program_Status command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_program_status(ADCS_boot_running_status_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_BOOT_RUNNING_PROGRAM_STATUS, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

    ADCS_Pack_to_Program_Status_Struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Communication_Status command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_communication_status(ADCS_comms_status_struct_t *output_struct) {
    // returns I2C communication status of the ADCS (Table 37)
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_SATSTATE_COMM_STATUS, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

    ADCS_Pack_to_Comms_Status_Struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Deploy_Magnetometer command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_deploy_magnetometer(uint8_t deploy_timeout) {
    // Deploys the magnetometer boom, timeout in seconds
    uint8_t data_send[1] = {deploy_timeout};
    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_DEPLOY_MAGNETOMETER_BOOM, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Set_Run_Mode command.
/// @param[in] mode Run mode to set; can be can be off (0), enabled (1), triggered (2), or simulation (3)
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_set_run_mode(ADCS_run_mode_enum_t mode) {
    uint8_t data_send[1] = {mode};
    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_ADCS_RUN_MODE, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Clear_Errors command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_clear_errors() {
    // Clears error flags
    // NOTE: THERE IS ANOTHER, SEPARATE CLEAR ERROR FLAG TC FOR THE BOOTLODER (ADCS_COMMAND_BL_CLEAR_ERRORS)
    uint8_t data_send[1] = {192}; // 0b11000000
    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_CLEAR_ERRORS, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the titular command.
/// @param mode Control mode to set (Table 77 in Firmware Manual)
/// @param timeout Timeout to set control mode
/// @return 0 if successful, non-zero if an error occurred in transmission.
uint8_t ADCS_attitude_control_mode(ADCS_control_mode_enum_t mode, uint16_t timeout) {
    // Sets the ADCS attitude control mode
    // See User Manual, Section 4.4.3 Table 3 for requirements to switch control mode
    uint8_t data_send[3] = {mode, timeout & 0x00FF, timeout >> 8};
    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_SET_ATTITUDE_CONTROL_MODE, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Attitude_Estimation_Mode command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_attitude_estimation_mode(ADCS_estimation_mode_enum_t mode) {
    // Sets the ADCS attitude estimation mode
    // Possible values for mode given in Section 6.3 Table 80 of Firmware Reference Manual (ranges from 0 to 7)
    // needs power control to be on
    uint8_t data_send[1] = {mode};
    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_SET_ATTITUDE_ESTIMATION_MODE, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Run_Once command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_run_once() {
    // requires ADCS_Enable_Triggered to have run first
    // (if ADCS_Enable_On has run instead, then this is unnecessary)
    uint8_t data_send[1]; // 0-byte data (from manual) input into wrapper, but one-byte here to avoid warnings
    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_TRIGGER_ADCS_LOOP, data_send, 0, ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the titular command.
/// @param mode Mode to set (Table 89 in Firmware Manual)
/// @return 0 if successful, non-zero if an error occurred in transmission.
uint8_t ADCS_set_magnetometer_mode(ADCS_magnetometer_mode_enum_t mode) {
    uint8_t data_send[1] = {mode};
    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_SET_MODE_OF_MAGNETOMETER_OPERATION, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Set_Magnetorquer_Output command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_set_magnetorquer_output(double x_duty, double y_duty, double z_duty) {
    // only valid after ADCS_Enable_Manual_Control is run
    // for the duty equations, raw parameter value is obtained using the formula: (raw parameter) = (formatted value)*1000.0
    // duty >> 8 gives upper byte, duty & 0x00FF gives lower byte
    uint8_t data_send[6];
    // swap low and high bytes and populate data_send
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((uint16_t) (x_duty * 1000)), 0);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((uint16_t) (y_duty * 1000)), 2);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((uint16_t) (z_duty * 1000)), 4);
    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_SET_MAGNETORQUER_OUTPUT, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the titular command.
/// @param x_speed Wheel speed X value
/// @param y_speed Wheel speed Y value
/// @param z_speed Wheel speed Z value
/// @return 0 if successful, non-zero if an error occurred in transmission.
uint8_t ADCS_set_wheel_speed(int16_t x_speed, int16_t y_speed, int16_t z_speed) {
    // only valid after ADCS_Enable_Manual_Control is run
    // for the duty equations, raw parameter value is in rpm
    uint8_t data_send[6]; // 6-byte data to send
    // swap low and high bytes and populate data_send
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, x_speed, 0);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, y_speed, 2);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, z_speed, 4);
    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_SET_WHEEL_SPEED, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Set the power control mode of each component of the ADCS; for each, 0 turns the component off, 1 turns it on, and 2 keeps it the same as previously.
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
uint8_t ADCS_set_power_control(ADCS_power_select_enum_t cube_control_signal, ADCS_power_select_enum_t cube_control_motor, ADCS_power_select_enum_t cube_sense1,
        ADCS_power_select_enum_t cube_sense2, ADCS_power_select_enum_t cube_star_power, ADCS_power_select_enum_t cube_wheel1_power,
        ADCS_power_select_enum_t cube_wheel2_power, ADCS_power_select_enum_t cube_wheel3_power, ADCS_power_select_enum_t motor_power,
        ADCS_power_select_enum_t gps_power) {
    uint8_t data_send[3]; // 3-byte data (from manual)
    // within the byte, everything goes in reverse order!!
    data_send[0] = (cube_control_signal) | (cube_control_motor << 2) | (cube_sense1 << 4) | (cube_sense2 << 6);
    data_send[1] = (cube_star_power) | (cube_wheel1_power << 2) | (cube_wheel2_power << 4) | (cube_wheel3_power << 6);
    data_send[2] = (motor_power) | (gps_power << 2);
    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_CUBEACP_SET_ADCS_POWER_CONTROL, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Power_Control command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_power_control(ADCS_power_control_struct_t *output_struct) {
    uint8_t data_length = 3;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_GET_ADCS_POWER_CONTROL, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

    ADCS_Pack_to_Power_Control_Struct(data_received, output_struct);

    return tlm_status;
}/// @brief Instruct the ADCS to set the magnetometer configuration.
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
uint8_t ADCS_set_magnetometer_config(
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
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((int16_t) (mounting_transform_alpha_angle * 100)), 0);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((int16_t) (mounting_transform_beta_angle * 100)), 2);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((int16_t) (mounting_transform_gamma_angle * 100)), 4);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((int16_t) (channel_1_offset * 1000)), 6);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((int16_t) (channel_2_offset * 1000)), 8);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((int16_t) (channel_3_offset * 1000)), 10);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((int16_t) (sensitivity_matrix_s11 * 1000)), 12);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((int16_t) (sensitivity_matrix_s22 * 1000)), 14);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((int16_t) (sensitivity_matrix_s33 * 1000)), 16);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((int16_t) (sensitivity_matrix_s12 * 1000)), 18);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((int16_t) (sensitivity_matrix_s13 * 1000)), 20);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((int16_t) (sensitivity_matrix_s21 * 1000)), 22);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((int16_t) (sensitivity_matrix_s23 * 1000)), 24);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((int16_t) (sensitivity_matrix_s31 * 1000)), 26);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((int16_t) (sensitivity_matrix_s32 * 1000)), 28);

    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_CUBEACP_SET_MAGNETOMETER_CONFIG, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;

}

/// @brief Instruct the ADCS to execute the ADCS_Save_Config command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_save_config() {
    uint8_t data_send[1]; // 0-byte data (from manual) input into wrapper, but one-byte here to avoid warnings
    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_SAVE_CONFIG, data_send, 0, ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Save_Orbit_Params command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_save_orbit_params() {
    uint8_t data_send[1]; // 0-byte data (from manual) input into wrapper, but one-byte here to avoid warnings
    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_SAVE_ORBIT_PARAMS, data_send, 0, ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Estimate_Angular_Rates command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_estimate_angular_rates(ADCS_angular_rates_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_ESTIMATED_ANGULAR_RATES, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

    ADCS_Pack_to_Angular_Rates_Struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_LLH_Position command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_llh_position(ADCS_llh_position_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_SATELLITE_POSITION_LLH, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

    ADCS_Pack_to_LLH_Position_Struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Bootloader_Clear_Errors command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_bootloader_clear_errors() {
    uint8_t data_send[1]; // 0-byte data (from manual) input into wrapper, but one-byte here to avoid warnings
    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_BOOTLOADER_CLEAR_ERRORS, data_send, 0, ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Set_Unix_Time_Save_Mode command.
/// @param[in] save_now whether to save the current Unix time immediately (bool passed as int; 1 = save immediately, 0 = don't save immediately)
/// @param[in] save_on_update whether to save the current Unix time whenever a command is used to update it (bool passed as int; 1 = save on command, 0 = don't)
/// @param[in] save_periodic whether to save the current Unix time periodically (bool passed as int; 1 = save periodically, 0 = don't)
/// @param[in] period the period of saving the current Unix time
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_set_unix_time_save_mode(bool save_now, bool save_on_update, bool save_periodic, uint8_t period) {
    uint8_t data_send[2] = { (save_now | (save_on_update << 1) | (save_periodic << 2) ) , period}; // 2-byte data (from manual)
    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_SET_UNIX_TIME_SAVE_TO_FLASH, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Unix_Time_Save_Mode command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_unix_time_save_mode(ADCS_set_unix_time_save_mode_struct_t *output_struct) {
    uint8_t data_length = 2;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_GET_UNIX_TIME_SAVE_TO_FLASH, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

    ADCS_Pack_to_Unix_Time_Save_Mode_Struct(data_received, output_struct);

    return tlm_status;
}

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
uint8_t ADCS_set_sgp4_orbit_params(double inclination, double eccentricity, double ascending_node_right_ascension,
                                //         degrees,               dimensionless,             degrees
        double perigee_argument, double b_star_drag_term, double mean_motion, double mean_anomaly, double epoch) {
        // degrees,                    dimensionless,            orbits/day,            degrees,            years.days

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

    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_CUBEACP_SET_SGP4_ORBIT_PARAMETERS, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_SGP4_Orbit_Params command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_sgp4_orbit_params(ADCS_orbit_params_struct_t *output_struct) {
    uint8_t data_length = 64;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_GET_SGP4_ORBIT_PARAMETERS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Orbit_Params_Struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Rate_Sensor_Rates command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_rate_sensor_rates(ADCS_rated_sensor_rates_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RATE_SENSOR_RATES, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Rated_Sensor_Rates_Struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Wheel_Speed command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_wheel_speed(ADCS_wheel_speed_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_WHEEL_SPEED, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Wheel_Speed_Struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Magnetorquer_Command command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_magnetorquer_command(ADCS_magnetorquer_command_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_MAGNETORQUER_COMMAND, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Magnetorquer_Command_Struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Raw_Magnetometer_Values command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_magnetometer_values(ADCS_raw_magnetometer_values_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_MAGNETOMETER, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Raw_Magnetometer_Values_Struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Estimate_Fine_Angular_Rates command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_estimate_fine_angular_rates(ADCS_fine_angular_rates_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_FINE_ESTIMATED_ANGULAR_RATES, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Fine_Angular_Rates_Struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Magnetometer_Config command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_magnetometer_config(ADCS_magnetometer_config_struct_t *output_struct) {
    uint8_t data_length = 30;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_GET_MAGNETOMETER_CONFIG, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Magnetometer_Config_Struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Commanded_Attitude_Angles command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_commanded_attitude_angles(ADCS_commanded_angles_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_GET_COMMANDED_ATTITUDE_ANGLES, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Commanded_Attitude_Angles_Struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Set_Commanded_Attitude_Angles command.
/// @param[in] x x attitude angle
/// @param[in] y y attitude angle
/// @param[in] z z attitude angle
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_set_commanded_attitude_angles(double x, double y, double z) {
    // raw parameter value is obtained using the formula: (raw parameter) = (formatted value)*100.0
    // angle >> 8 gives upper byte, angle & 0x00FF gives lower byte
    uint8_t data_send[6];
    // swap low and high bytes and populate data_send
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((int16_t) (x * 100)), 0);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((int16_t) (y * 100)), 2);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ((int16_t) (z * 100)), 4);
    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_CUBEACP_SET_COMMANDED_ATTITUDE_ANGLES, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Set_Estimation_Params command.
/// @param[in] magnetometer_rate_filter_system_noise Magnetometer rate filter system noise parameter
/// @param[in] extended_kalman_filter_system_noise EKS system noise parameter
/// @param[in] coarse_sun_sensor_measurement_noise CSS measurement noise parameter
/// @param[in] sun_sensor_measurement_noise sun sensor measurement noise parameter
/// @param[in] nadir_sensor_measurement_noise nadir sensor measurement noise parameter
/// @param[in] magnetometer_measurement_noise magnetometer measurement noise parameter
/// @param[in] star_tracker_measurement_noise star tracker measurement noise parameter
/// @param[in] use_sun_sensor whether or not to use the sun sensor measurement in extended kalman filter
/// @param[in] use_nadir_sensor whether or not to use the nadir sensor measurement in extended kalman filter
/// @param[in] use_css whether or not to use the CSS measurement in extended kalman filter
/// @param[in] use_star_tracker whether or not to use the star tracker measurement in extended kalman filter
/// @param[in] nadir_sensor_terminator_test select to ignore nadir sensor measurements when terminator is in FOV
/// @param[in] automatic_magnetometer_recovery select whether automatic switch to redundant magnetometer should occur in case of failure
/// @param[in] magnetometer_mode select magnetometer mode for estimation and control
/// @param[in] magnetometer_selection_for_raw_magnetometer_telemetry select magnetometer mode for the second raw telemetry frame
/// @param[in] automatic_estimation_transition_due_to_rate_sensor_errors enable/disable automatic transition from MEMS rate estimation mode to RKF in case of rate sensor error
/// @param[in] wheel_30s_power_up_delay present in CubeSupport but not in the manual -- need to test
/// @param[in] cam1_and_cam2_sampling_period the manual calls it this, but CubeSupport calls it "error counter reset period" -- need to test
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_set_estimation_params(
                                float magnetometer_rate_filter_system_noise, 
                                float extended_kalman_filter_system_noise, 
                                float coarse_sun_sensor_measurement_noise, 
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
                                ADCS_magnetometer_mode_enum_t magnetometer_mode, // this is actually the same one as for ID 56!
                                ADCS_magnetometer_mode_enum_t magnetometer_selection_for_raw_magnetometer_telemetry, // and so is this, actually!
                                bool automatic_estimation_transition_due_to_rate_sensor_errors, 
                                bool wheel_30s_power_up_delay, // present in CubeSupport but not in the manual -- need to test
                                uint8_t cam1_and_cam2_sampling_period) { // the manual calls it this, but CubeSupport calls it "error counter reset period" -- need to test
    // float uses IEEE 754 float32, with all bytes reversed, so eg. 1.1 becomes [0xCD, 0xCC, 0x8C, 0x3F]
    // the float type should already be reversed, but need to test in implementation
    uint8_t data_send[31];

    // convert floats to reversed arrays of uint8_t
    memcpy(&data_send[0],  &magnetometer_rate_filter_system_noise, sizeof(magnetometer_rate_filter_system_noise));
    memcpy(&data_send[4],  &extended_kalman_filter_system_noise, sizeof(extended_kalman_filter_system_noise));
    memcpy(&data_send[8],  &coarse_sun_sensor_measurement_noise, sizeof(coarse_sun_sensor_measurement_noise));
    memcpy(&data_send[12], &sun_sensor_measurement_noise, sizeof(sun_sensor_measurement_noise));
    memcpy(&data_send[16], &nadir_sensor_measurement_noise, sizeof(nadir_sensor_measurement_noise));
    memcpy(&data_send[20], &magnetometer_measurement_noise, sizeof(magnetometer_measurement_noise));
    memcpy(&data_send[24], &star_tracker_measurement_noise, sizeof(star_tracker_measurement_noise));

    // convert bools to uint8
    data_send[28] = (magnetometer_mode << 6) | (automatic_magnetometer_recovery << 5) | (nadir_sensor_terminator_test << 4) | (use_star_tracker << 3) | (use_css << 2) | (use_nadir_sensor << 1) | (use_sun_sensor);
    data_send[29] = (wheel_30s_power_up_delay << 3) | (automatic_estimation_transition_due_to_rate_sensor_errors << 2) | (magnetometer_selection_for_raw_magnetometer_telemetry);

    data_send[30] = cam1_and_cam2_sampling_period; // lower four bits are for cam1 and upper four are for cam2 if the manual is correct, not CubeSupport

    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_CUBEACP_SET_ESTIMATION_PARAMETERS, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Estimation_Params command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_estimation_params(ADCS_estimation_params_struct_t *output_struct) {
    uint8_t data_length = 31;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_GET_ESTIMATION_PARAMETERS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Estimation_Params_Struct(data_received, output_struct);

    return tlm_status;
}/// @brief Set the Augmented SGP4 Parameters of the ADCS.
/// @param[in] incl_coefficient Set inclination filter coefficient
/// @param[in] raan_coefficient Set RAAN filter coefficient 
/// @param[in] ecc_coefficient Set eccentricity filter coefficient
/// @param[in] aop_coefficient Set argument of perigee filter coefficient
/// @param[in] time_coefficient Set time filter coefficient
/// @param[in] pos_coefficient Set position filter coefficient
/// @param[in] maximum_position_error Maximum position error for Augmented_SGP4 to continue working
/// @param[in] augmented_sgp4_filter The type of filter being used (enum)
/// @param[in] xp_coefficient Polar coefficient xp  
/// @param[in] yp_coefficient Polar coefficient yp 
/// @param[in] gps_roll_over GPS roll over number
/// @param[in] position_sd Maximum position standard deviation for Augmented_SGP4 to continue working
/// @param[in] velocity_sd Maximum velocity standard deviation for Augmented_SGP4 to continue working
/// @param[in] min_satellites Minimum satellites required for Augmented_SGP4 to continue working
/// @param[in] time_gain Time offset compensation gain
/// @param[in] max_lag Maximum lagged timestamp measurements to incorporate
/// @param[in] min_samples Minimum samples to use to get Augmented_SGP4
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_set_augmented_sgp4_params(double incl_coefficient, double raan_coefficient, double ecc_coefficient, double aop_coefficient, double time_coefficient, double pos_coefficient, double maximum_position_error, ADCS_augmented_sgp4_filter_enum_t augmented_sgp4_filter, double xp_coefficient, double yp_coefficient, uint8_t gps_roll_over, double position_sd, double velocity_sd, uint8_t min_satellites, double time_gain, double max_lag, uint16_t min_samples) {
    uint8_t data_send[30]; // from Table 209

    // populate data_send
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, (uint16_t) (incl_coefficient * 1000), 0);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, (uint16_t) (raan_coefficient * 1000), 2);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, (uint16_t) (ecc_coefficient * 1000), 4);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, (uint16_t) (aop_coefficient * 1000), 6);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, (uint16_t) (time_coefficient * 1000), 8);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, (uint16_t) (pos_coefficient * 1000), 10);
    data_send[12] = (uint8_t) (maximum_position_error * 10);
    data_send[13] = (uint8_t) augmented_sgp4_filter;
    ADCS_convert_uint32_to_reversed_uint8_array_members(data_send, (int32_t) (xp_coefficient * 10000000), 14);
    ADCS_convert_uint32_to_reversed_uint8_array_members(data_send, (int32_t) (yp_coefficient * 10000000), 18);
    data_send[22] = gps_roll_over;
    data_send[23] = (uint8_t) (position_sd * 10);
    data_send[24] = (uint8_t) (velocity_sd * 100);
    data_send[25] = min_satellites;
    data_send[26] = (uint8_t) (time_gain * 100);
    data_send[27] = (uint8_t) (max_lag * 100);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, min_samples, 28);

    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_CUBEACP_SET_AUGMENTED_SGP4_PARAMETERS, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Augmented_SGP4_Params command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_augmented_sgp4_params(ADCS_augmented_sgp4_params_struct_t *output_struct) {
    uint8_t data_length = 30;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_GET_AUGMENTED_SGP4_PARAMETERS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Augmented_SGP4_Params_Struct(data_received, output_struct);

    return tlm_status;
}/// @brief Instruct the ADCS to execute the ADCS_Set_Tracking_Controller_Target_Reference command.
/// @param[in] lon longitude
/// @param[in] lat latitude
/// @param[in] alt altitude
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_set_tracking_controller_target_reference(float lon, float lat, float alt) {
    uint8_t data_send[12];

    // float uses IEEE 754 float32, with all bytes reversed, so eg. 1.1 becomes [0xCD, 0xCC, 0x8C, 0x3F]
    // the float type should already be reversed, but need to test in implementation
    // convert floats to reversed arrays of uint8_t
    memcpy(&data_send[0],  &lon, sizeof(lon));
    memcpy(&data_send[4],  &lat, sizeof(lat));
    memcpy(&data_send[8],  &alt, sizeof(alt));

    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_CUBEACP_SET_TRACKING_CONTROLLER_TARGET_REFERENCE, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Tracking_Controller_Target_Reference command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_tracking_controller_target_reference(ADCS_tracking_controller_target_struct_t *output_struct) {
    uint8_t data_length = 12;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_GET_TRACKING_CONTROLLER_TARGET_REFERENCE, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Tracking_Controller_Target_Reference_Struct(data_received, output_struct);

    return tlm_status;
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
uint8_t ADCS_set_rate_gyro_config(ADCS_axis_select_enum_t gyro1, ADCS_axis_select_enum_t gyro2, ADCS_axis_select_enum_t gyro3, double x_rate_offset, double y_rate_offset, double z_rate_offset, uint8_t rate_sensor_mult) {
    uint8_t data_send[10];

    data_send[0] = (uint8_t) gyro1;
    data_send[1] = (uint8_t) gyro2;
    data_send[2] = (uint8_t) gyro3;

    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, (int16_t) (x_rate_offset * 1000), 3);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, (int16_t) (y_rate_offset * 1000), 5);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, (int16_t) (z_rate_offset * 1000), 7);

    data_send[9] = rate_sensor_mult;

    uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_CUBEACP_SET_RATE_GYRO_CONFIG, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Rate_Gyro_Config command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_rate_gyro_config(ADCS_rate_gyro_config_struct_t *output_struct) {
    uint8_t data_length = 12;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_GET_RATE_GYRO_CONFIG, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Rate_Gyro_Config_Struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Estimated_Attitude_Angles command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_estimated_attitude_angles(ADCS_estimated_attitude_angles_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_ESTIMATED_ATTITUDE_ANGLES, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Estimated_Attitude_Angles_Struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Magnetic_Field_Vector command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_magnetic_field_vector(ADCS_magnetic_field_vector_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_MAGNETIC_FIELD_VECTOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Magnetic_Field_Vector_Struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Fine_Sun_Vector command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_fine_sun_vector(ADCS_fine_sun_vector_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_FINE_SUN_VECTOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Fine_Sun_Vector_Struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Nadir_Vector command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_nadir_vector(ADCS_nadir_vector_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_NADIR_VECTOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Nadir_Vector_Struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Commanded_Wheel_Speed command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_commanded_wheel_speed(ADCS_wheel_speed_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_WHEEL_SPEED_COMMANDS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Commanded_Wheel_Speed_Struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_IGRF_Magnetic_Field_Vector command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_igrf_magnetic_field_vector(ADCS_magnetic_field_vector_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_IGRF_MODELLED_MAGNETIC_FIELD_VECTOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_IGRF_Magnetic_Field_Vector_Struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Quaternion_Error_Vector command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_quaternion_error_vector(ADCS_quaternion_error_vector_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_QUATERNION_ERROR_VECTOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Quaternion_Error_Vector_Struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Estimated_Gyro_Bias command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_estimated_gyro_bias(ADCS_estimated_gyro_bias_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_ESTIMATED_GYRO_BIAS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Estimated_Gyro_Bias_Struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Estimation_Innovation_Vector command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_estimation_innovation_vector(ADCS_estimation_innovation_vector_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_ESTIMATION_INNOVATION_VECTOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Estimation_Innovation_Vector_Struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_Cam1_Sensor command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_cam1_sensor(ADCS_raw_cam_sensor_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_CAM1_SENSOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Raw_Cam1_Sensor_Struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_Cam2_Sensor command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_cam2_sensor(ADCS_raw_cam_sensor_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_CAM2_SENSOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Raw_Cam2_Sensor_Struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_Coarse_Sun_Sensor_1_to_6 command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_coarse_sun_sensor_1_to_6(ADCS_raw_coarse_sun_sensor_1_to_6_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_COARSE_SUN_SENSOR_1_TO_6, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Raw_Coarse_Sun_Sensor_1_to_6_Struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_Coarse_Sun_Sensor_7_to_10 command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_coarse_sun_sensor_7_to_10(ADCS_raw_coarse_sun_sensor_7_to_10_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_COARSE_SUN_SENSOR_7_TO_10, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Raw_Coarse_Sun_Sensor_7_to_10_Struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_CubeControl_Current command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_cubecontrol_current(ADCS_cubecontrol_current_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_CUBECONTROL_CURRENT_MEASUREMENTS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_CubeControl_Current_Struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_GPS_Status command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_gps_status(ADCS_raw_gps_status_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_GPS_STATUS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Raw_GPS_Status_Struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_GPS_Time command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_gps_time(ADCS_raw_gps_time_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_GPS_TIME, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Raw_GPS_Time_Struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_GPS_X command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_gps_x(ADCS_raw_gps_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_GPS_X, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Raw_GPS_Struct(ADCS_GPS_AXIS_X, data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_GPS_Y command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_gps_y(ADCS_raw_gps_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_GPS_Y, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Raw_GPS_Struct(ADCS_GPS_AXIS_Y, data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_GPS_Z command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_gps_z(ADCS_raw_gps_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_GPS_Z, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Raw_GPS_Struct(ADCS_GPS_AXIS_Z, data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Measurements command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_measurements(ADCS_measurements_struct_t *output_struct) {
    uint8_t data_length = 72;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_ADCS_MEASUREMENTS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_Measurements_Struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_execution_state command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_acp_execution_state(ADCS_acp_execution_state_struct_t *output_struct) {
    uint8_t data_length = 3;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_EXECUTION_STATE, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_Pack_to_ACP_Execution_State_Struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_get_current_state_1 command. (There's an ADCS_current_state_2 command which is presently not implemented)
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_current_state_1(ADCS_current_state_1_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_ADCS_STATE, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_pack_to_current_state_1_struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_get_raw_star_tracker_data command. 
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_star_tracker_data(ADCS_raw_star_tracker_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; // define temp buffer

    uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_STAR_TRACKER, data_received, data_length, ADCS_INCLUDE_CHECKSUM); // populate buffer

    ADCS_pack_to_raw_star_tracker_struct(data_received, output_struct);

    return tlm_status;
}