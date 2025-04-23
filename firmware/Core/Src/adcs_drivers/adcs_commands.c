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
#include "timekeeping/timekeeping.h"
#include "littlefs/littlefs_helper.h"
#include "log/log.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "stm32l4xx_hal.h"

extern I2C_HandleTypeDef hi2c1; // allows not needing the parameters

/// @brief Initialize the ADCS CRC, timestamp, and file system directory. 
/// @return 0 when successful
uint8_t ADCS_initialize() {

    // TODO: LittleFS must be mounted, power must be provided to the ADCS, and OBC time must be set in order to initialize the ADCS
    ADCS_synchronize_unix_time();
    ADCS_initialize_crc8_checksum();
    LFS_make_directory("ADCS");

    return 0;
}

/// @brief Instructs the ADCS to determine whether the last command succeeded. (Doesn't work for telemetry requests, by design.)
/// @param[out] ack Structure containing the formatted information about the last command sent.
/// @return 0 if successful, non-zero if an error occurred in transmission.
uint8_t ADCS_cmd_ack(ADCS_cmd_ack_struct_t *ack) {
    uint8_t data_received[8]; 
    uint8_t data_length = 4;

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_COMMAND_ACK, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

    // map temp buffer to Ack struct
    ADCS_pack_to_ack_struct(&data_received[0], ack);

    if (tlm_status == 0) {
        return ack->error_flag;
    }

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Reset command.
/// @return 0 if successful, non-zero if a HAL error occurred in transmission.
uint8_t ADCS_reset() {
    uint8_t data_send[1] = {ADCS_MAGIC_NUMBER};
    const uint8_t cmd_status = ADCS_send_i2c_telecommand(ADCS_COMMAND_RESET, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
        // note: because the ADCS will become unresponsive afterward for at least 15 seconds, do not poll for a response
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Identification command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_identification(ADCS_id_struct_t *output_struct) {

    uint8_t data_length = 8;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_IDENTIFICATION, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_identification_struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Program_Status command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_program_status(ADCS_boot_running_status_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_BOOT_RUNNING_PROGRAM_STATUS, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

    ADCS_pack_to_program_status_struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Communication_Status command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_communication_status(ADCS_comms_status_struct_t *output_struct) {
    // returns I2C communication status of the ADCS (Table 37)
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_SATSTATE_COMM_STATUS, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

    ADCS_pack_to_comms_status_struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Deploy_Magnetometer command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_deploy_magnetometer(uint8_t deploy_timeout) {
    // Deploys the magnetometer boom, timeout in seconds
    uint8_t data_send[1] = {deploy_timeout};
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_DEPLOY_MAGNETOMETER_BOOM, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Set_Run_Mode command.
/// @param[in] mode Run mode to set; can be can be off (0), enabled (1), triggered (2), or simulation (3)
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_set_run_mode(ADCS_run_mode_enum_t mode) {
    uint8_t data_send[1] = {mode};
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_ADCS_RUN_MODE, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Clear_Errors command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_clear_errors() {
    // NOTE: THERE IS ANOTHER, SEPARATE CLEAR ERROR FLAG TC FOR THE BOOTLODER (ADCS_COMMAND_BL_CLEAR_ERRORS)
    uint8_t data_send[1] = {192}; // 0b11000000
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_CLEAR_ERRORS, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
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
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_SET_ATTITUDE_CONTROL_MODE, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Attitude_Estimation_Mode command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_attitude_estimation_mode(ADCS_estimation_mode_enum_t mode) {
    // Sets the ADCS attitude estimation mode
    // Possible values for mode given in Section 6.3 Table 80 of Firmware Reference Manual (ranges from 0 to 7)
    // needs power control to be on
    uint8_t data_send[1] = {mode};
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_SET_ATTITUDE_ESTIMATION_MODE, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Run_Once command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_run_once() {
    // requires ADCS_Enable_Triggered to have run first
    // (if ADCS_Enable_On has run instead, then this is unnecessary)
    uint8_t data_send[1]; // 0-byte data (from manual) input into wrapper, but one-byte here to avoid warnings
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_TRIGGER_ADCS_LOOP, data_send, 0, ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the titular command.
/// @param mode Mode to set (Table 89 in Firmware Manual)
/// @return 0 if successful, non-zero if an error occurred in transmission.
uint8_t ADCS_set_magnetometer_mode(ADCS_magnetometer_mode_enum_t mode) {
    uint8_t data_send[1] = {mode};
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_SET_MODE_OF_MAGNETOMETER_OPERATION, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
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
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_SET_MAGNETORQUER_OUTPUT, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
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
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_SET_WHEEL_SPEED, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
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
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_CUBEACP_SET_ADCS_POWER_CONTROL, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Power_Control command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_power_control(ADCS_power_control_struct_t *output_struct) {
    uint8_t data_length = 3;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_GET_ADCS_POWER_CONTROL, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

    ADCS_pack_to_power_control_struct(data_received, output_struct);

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

    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_CUBEACP_SET_MAGNETOMETER_CONFIG, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;

}

/// @brief Instruct the ADCS to execute the ADCS_Save_Config command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_save_config() {
    uint8_t data_send[1]; // 0-byte data (from manual) input into wrapper, but one-byte here to avoid warnings
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_SAVE_CONFIG, data_send, 0, ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Save_Orbit_Params command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_save_orbit_params() {
    uint8_t data_send[1]; // 0-byte data (from manual) input into wrapper, but one-byte here to avoid warnings
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_SAVE_ORBIT_PARAMS, data_send, 0, ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Estimate_Angular_Rates command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_estimate_angular_rates(ADCS_angular_rates_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_ESTIMATED_ANGULAR_RATES, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

    ADCS_pack_to_angular_rates_struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_LLH_Position command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_llh_position(ADCS_llh_position_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_SATELLITE_POSITION_LLH, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

    ADCS_pack_to_llh_position_struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Bootloader_Clear_Errors command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_bootloader_clear_errors() {
    uint8_t data_send[1]; // 0-byte data (from manual) input into wrapper, but one-byte here to avoid warnings
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_BOOTLOADER_CLEAR_ERRORS, data_send, 0, ADCS_INCLUDE_CHECKSUM);
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
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_SET_UNIX_TIME_SAVE_TO_FLASH, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Unix_Time_Save_Mode command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_unix_time_save_mode(ADCS_set_unix_time_save_mode_struct_t *output_struct) {
    uint8_t data_length = 2;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_GET_UNIX_TIME_SAVE_TO_FLASH, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

    ADCS_pack_to_unix_time_save_mode_struct(data_received, output_struct);

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

    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_CUBEACP_SET_SGP4_ORBIT_PARAMETERS, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_SGP4_Orbit_Params command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_sgp4_orbit_params(ADCS_orbit_params_struct_t *output_struct) {
    uint8_t data_length = 64;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_GET_SGP4_ORBIT_PARAMETERS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_orbit_params_struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Rate_Sensor_Rates command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_rate_sensor_rates(ADCS_rated_sensor_rates_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RATE_SENSOR_RATES, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_rated_sensor_rates_struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Wheel_Speed command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_wheel_speed(ADCS_wheel_speed_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_WHEEL_SPEED, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_wheel_speed_struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Magnetorquer_Command command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_magnetorquer_command(ADCS_magnetorquer_command_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_MAGNETORQUER_COMMAND, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_magnetorquer_command_struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Raw_Magnetometer_Values command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_magnetometer_values(ADCS_raw_magnetometer_values_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_MAGNETOMETER, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_raw_magnetometer_values_struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Estimate_Fine_Angular_Rates command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_estimate_fine_angular_rates(ADCS_fine_angular_rates_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_FINE_ESTIMATED_ANGULAR_RATES, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_fine_angular_rates_struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Magnetometer_Config command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_magnetometer_config(ADCS_magnetometer_config_struct_t *output_struct) {
    uint8_t data_length = 30;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_GET_MAGNETOMETER_CONFIG, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_magnetometer_config_struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Commanded_Attitude_Angles command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_commanded_attitude_angles(ADCS_commanded_angles_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_GET_COMMANDED_ATTITUDE_ANGLES, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_commanded_attitude_angles_struct(data_received, output_struct);

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
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_CUBEACP_SET_COMMANDED_ATTITUDE_ANGLES, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
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

    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_CUBEACP_SET_ESTIMATION_PARAMETERS, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Estimation_Params command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_estimation_params(ADCS_estimation_params_struct_t *output_struct) {
    uint8_t data_length = 31;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_GET_ESTIMATION_PARAMETERS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_estimation_params_struct(data_received, output_struct);

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

    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_CUBEACP_SET_AUGMENTED_SGP4_PARAMETERS, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Augmented_SGP4_Params command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_augmented_sgp4_params(ADCS_augmented_sgp4_params_struct_t *output_struct) {
    uint8_t data_length = 30;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_GET_AUGMENTED_SGP4_PARAMETERS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_augmented_sgp4_params_struct(data_received, output_struct);

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

    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_CUBEACP_SET_TRACKING_CONTROLLER_TARGET_REFERENCE, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Tracking_Controller_Target_Reference command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_tracking_controller_target_reference(ADCS_tracking_controller_target_struct_t *output_struct) {
    uint8_t data_length = 12;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_GET_TRACKING_CONTROLLER_TARGET_REFERENCE, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_tracking_controller_target_reference_struct(data_received, output_struct);

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

    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_CUBEACP_SET_RATE_GYRO_CONFIG, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Get_Rate_Gyro_Config command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_rate_gyro_config(ADCS_rate_gyro_config_struct_t *output_struct) {
    uint8_t data_length = 12;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_GET_RATE_GYRO_CONFIG, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_rate_gyro_config_struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Estimated_Attitude_Angles command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_estimated_attitude_angles(ADCS_estimated_attitude_angles_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_ESTIMATED_ATTITUDE_ANGLES, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_estimated_attitude_angles_struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Magnetic_Field_Vector command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_magnetic_field_vector(ADCS_magnetic_field_vector_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_MAGNETIC_FIELD_VECTOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_magnetic_field_vector_struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Fine_Sun_Vector command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_fine_sun_vector(ADCS_fine_sun_vector_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_FINE_SUN_VECTOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_fine_sun_vector_struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Nadir_Vector command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_nadir_vector(ADCS_nadir_vector_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_NADIR_VECTOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_nadir_vector_struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Commanded_Wheel_Speed command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_commanded_wheel_speed(ADCS_wheel_speed_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_WHEEL_SPEED_COMMANDS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_commanded_wheel_speed_struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_IGRF_Magnetic_Field_Vector command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_igrf_magnetic_field_vector(ADCS_magnetic_field_vector_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_IGRF_MODELLED_MAGNETIC_FIELD_VECTOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_igrf_magnetic_field_vector_struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Quaternion_Error_Vector command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_quaternion_error_vector(ADCS_quaternion_error_vector_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_QUATERNION_ERROR_VECTOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_quaternion_error_vector_struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Estimated_Gyro_Bias command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_estimated_gyro_bias(ADCS_estimated_gyro_bias_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_ESTIMATED_GYRO_BIAS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_estimated_gyro_bias_struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Estimation_Innovation_Vector command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_estimation_innovation_vector(ADCS_estimation_innovation_vector_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_ESTIMATION_INNOVATION_VECTOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_estimation_innovation_vector_struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_Cam1_Sensor command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_cam1_sensor(ADCS_raw_cam_sensor_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_CAM1_SENSOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_raw_cam1_sensor_struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_Cam2_Sensor command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_cam2_sensor(ADCS_raw_cam_sensor_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_CAM2_SENSOR, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_raw_cam2_sensor_struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_Coarse_Sun_Sensor_1_to_6 command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_coarse_sun_sensor_1_to_6(ADCS_raw_coarse_sun_sensor_1_to_6_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_COARSE_SUN_SENSOR_1_TO_6, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_raw_coarse_sun_sensor_1_to_6_struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_Coarse_Sun_Sensor_7_to_10 command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_coarse_sun_sensor_7_to_10(ADCS_raw_coarse_sun_sensor_7_to_10_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_COARSE_SUN_SENSOR_7_TO_10, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_raw_coarse_sun_sensor_7_to_10_struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_CubeControl_Current command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_cubecontrol_current(ADCS_cubecontrol_current_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_CUBECONTROL_CURRENT_MEASUREMENTS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_cubecontrol_current_struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_GPS_Status command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_gps_status(ADCS_raw_gps_status_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_GPS_STATUS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_raw_gps_status_struct(&data_received[0], output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_GPS_Time command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_gps_time(ADCS_raw_gps_time_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_GPS_TIME, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_raw_gps_time_struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_GPS_X command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_gps_x(ADCS_raw_gps_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_GPS_X, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_raw_gps_struct(ADCS_GPS_AXIS_X, data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_GPS_Y command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_gps_y(ADCS_raw_gps_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_GPS_Y, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_raw_gps_struct(ADCS_GPS_AXIS_Y, data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Raw_GPS_Z command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_gps_z(ADCS_raw_gps_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_GPS_Z, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_raw_gps_struct(ADCS_GPS_AXIS_Z, data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Measurements command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_measurements(ADCS_measurements_struct_t *output_struct) {
    uint8_t data_length = 72;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_ADCS_MEASUREMENTS, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_measurements_struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_reset_file_list_read_pointer command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_reset_file_list_read_pointer() {
    uint8_t data_send[1]; // 0-byte data (from manual) input into wrapper, but one-byte here to avoid warnings
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_RESET_FILELIST_READ_PTR, data_send, 0, ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_advance_file_list_read_pointer command.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_advance_file_list_read_pointer() {
    uint8_t data_send[1]; // 0-byte data (from manual) input into wrapper, but one-byte here to avoid warnings
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_ADVANCE_FILELIST_READ_PTR, data_send, 0, ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Request file information telemetry from the ADCS.
/// @param[out] output_struct Pointer to the struct to store parsed telemetry data.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_file_info_telemetry(ADCS_file_info_struct_t *output_struct) {
    uint8_t data_length = 12;
    uint8_t data_received[data_length]; // Temporary buffer for raw telemetry data.

    // Request telemetry data
    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_FILE_INFO, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

    // Parse the raw telemetry data into the struct
    ADCS_pack_to_file_info_struct(data_received, output_struct);

    return tlm_status; 
}

/// @brief Instruct the ADCS to load a file download block into the download buffer.
/// @param[in] file_type File type to load (e.g., telemetry log, JPG, etc.).
/// @param[in] counter Counter value for file block.
/// @param[in] offset Offset in the file from which to start downloading.
/// @param[in] block_length Number of packets to send.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_load_file_download_block(ADCS_file_type_enum_t file_type, uint8_t counter, uint32_t offset, uint16_t block_length) {
    // Command data array (8 bytes as per Table 17)
    uint8_t data_send[8];

    // Pack File Type (8 bits)
    data_send[0] = (uint8_t)file_type;

    // Pack Counter (8 bits)
    data_send[1] = counter;

    // Pack Offset (32 bits, reverse byte order)
    ADCS_convert_uint32_to_reversed_uint8_array_members(data_send, offset, 2);

    // Pack Block Length (16 bits, reverse byte order)
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, block_length, 6);

    // Send the command
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_LOAD_FILE_DOWNLOAD_BLOCK, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);

    return cmd_status;
}

/// @brief Request Download Block Ready telemetry from the ADCS.
/// @param[out] output_struct Pointer to the struct to populate with telemetry data.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_download_block_ready_telemetry(ADCS_download_block_ready_struct_t *output_struct) {

    uint8_t data_length = 5;
    uint8_t data_received[data_length]; // Buffer to store telemetry data

    // Request telemetry data from the ADCS
    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_DOWNLOAD_BLOCK_READY, data_received, data_length, ADCS_INCLUDE_CHECKSUM);

    // Pack telemetry data into the struct
    ADCS_pack_to_download_block_ready_struct(data_received, output_struct);
    
    return tlm_status;
}
/// @brief Instruct the ADCS to execute the ADCS_execution_state command.
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_acp_execution_state(ADCS_acp_execution_state_struct_t *output_struct) {
    uint8_t data_length = 3;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_EXECUTION_STATE, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_acp_execution_state_struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Send the Initiate Download Burst command to the ADCS.
/// @param[in] ignore_hole_map Boolean flag to ignore the hole map.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_initiate_download_burst(bool ignore_hole_map) {
    uint8_t data_send[2]; // Command requires 2 bytes

    // Populate the command buffer
    data_send[0] = 20;                                  // First byte: Message Length (unused parameter according to CubeSpace)
    data_send[1] = (ignore_hole_map ? 1 : 0);          // Second byte: Ignore Hole Map as a single bit

    // Send the command via I2C and check the result
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_INIT_DOWNLOAD_BURST, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);

    return cmd_status; // Return the result
}

/// @brief Send a File Upload Hole Map command to the ADCS.
/// @param[in] hole_map Pointer to a 16-byte array representing the hole map.
/// @param[in] which_map Number between 1 and 8 to choose the hole map to upload.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_set_hole_map(uint8_t *hole_map, uint8_t which_map) {

    if (which_map < 1 || which_map > 8) {
        // which_map is number 1-8, added to ID 119 gives IDs 120 to 127 for the hole maps
        return 30;
    }

    const uint8_t cmd_status = ADCS_i2c_send_command_and_check((ADCS_COMMAND_INIT_DOWNLOAD_BURST + which_map), hole_map, 16, ADCS_INCLUDE_CHECKSUM);
                                                            
    return cmd_status; // Return the result
}

/// @brief Retrieve a File Upload Hole Map telemetry from the ADCS.
/// @param[out] hole_map_struct Pointer to an array of uint8 to store the retrieved hole map.
/// @param[in] which_map Number between 1 and 8 to choose the hole map to upload.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_hole_map(uint8_t *hole_map_struct, uint8_t which_map) {
    
    if (which_map < 1 || which_map > 8) {
        // which_map is number 1-8, added to ID 246 gives IDs 247 to 254 for the hole maps
        return 30;
    }

    // Request the telemetry data
    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check((ADCS_TELEMETRY_BLOCK_CHECKSUM + which_map), &hole_map_struct[0], 16, ADCS_INCLUDE_CHECKSUM);

    return tlm_status; // Return the result
}

/// @brief Instruct the ADCS to format the SD card.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_format_sd() {
    // returns telecommand error flag
    uint8_t data_send[1] = {ADCS_MAGIC_NUMBER};
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_FORMAT_SD, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Send the Erase File command to the ADCS.
/// @param[in] erase_file_command Pointer to a struct containing the file type, file counter, and erase all flag.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_erase_file(ADCS_file_type_enum_t filetype, uint8_t filecounter, bool erase_all) {

    uint8_t data_send[3] = {(uint8_t)filetype, filecounter, (uint8_t)erase_all};

    // Send the command with the packed parameters
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_ERASE_FILE, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);

    return cmd_status; // Return the result
}

/// @brief Retrieve the SD card format/erase progress telemetry from the ADCS.
/// @param[out] output_struct Pointer to the struct where the telemetry data will be stored.
/// @return 0 if successful, non-zero if an error occurred in transmission or processing.
uint8_t ADCS_get_sd_card_format_erase_progress(ADCS_sd_card_format_erase_progress_struct_t *output_struct) {

    uint8_t data_received[1]; // Array to hold the telemetry data
    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_FORMAT_SD, data_received, sizeof(data_received), ADCS_INCLUDE_CHECKSUM);

    // Parse the received data
    ADCS_pack_to_sd_card_format_erase_progress_struct(data_received, output_struct);

    return tlm_status; // Success
}

/// @brief Retrieve the File Download Buffer telemetry from the ADCS.
/// @param[out] output_struct Pointer to the struct where the telemetry data will be stored.
/// @return 0 if successful, non-zero if an error occurred in transmission or processing.
uint8_t ADCS_get_file_download_buffer(ADCS_file_download_buffer_struct_t *output_struct) {

    uint8_t data_received[22]; // Buffer to hold the received telemetry data
    
    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_FILE_DOWNLOAD_BUFFER, data_received, sizeof(data_received), ADCS_INCLUDE_CHECKSUM);

    // Parse the received data
    ADCS_pack_to_file_download_buffer_struct(data_received, output_struct);

    return tlm_status; // Success
}
/// @brief Instruct the ADCS to execute the ADCS_get_current_state_1 command. (There's an ADCS_current_state_2 command which is presently not implemented)
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_current_state_1(ADCS_current_state_1_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_ADCS_STATE, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_current_state_1_struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_get_raw_star_tracker_data command. 
/// @param output_struct Pointer to struct in which to place packed ADCS telemetry data
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_raw_star_tracker_data(ADCS_raw_star_tracker_struct_t *output_struct) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_CUBEACP_RAW_STAR_TRACKER, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_raw_star_tracker_struct(data_received, output_struct);

    return tlm_status;
}

/// @brief Instruct the ADCS to save an image to the SD card.
/// @param[in] camera_select (int) Which camera to save the image from; can be Camera 1 (0), Camera 2 (1), or Star (2)
/// @param[in] image_size (int) Resolution of the image to save; can be 1024x1024 (0), 512x512 (1), 256x256 (2), 128x128 (3), or 64x64 (4)
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_save_image_to_sd(ADCS_camera_select_enum_t camera_select, ADCS_image_size_enum_t image_size) {
    uint8_t data_send[2] = {camera_select, image_size};
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_SAVE_IMAGE, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to synchronize its Unix epoch time to the current OBC Unix time.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_synchronize_unix_time() {
    uint64_t current_unix_time_ms = TIM_get_current_unix_epoch_time_ms();
    
    uint32_t s_component = current_unix_time_ms  / 1000;
    uint16_t ms_component = current_unix_time_ms % 1000;

    uint8_t data_send[6];
    ADCS_convert_uint32_to_reversed_uint8_array_members(data_send, s_component, 0);
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, ms_component, 4);

    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(ADCS_COMMAND_SET_CURRENT_UNIX_TIME, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_get_current_unix_time command.
/// @param epoch_time_ms Pointer to uint64 to store the time since the Unix epoch in ms, according to the ADCS
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_current_unix_time(uint64_t* epoch_time_ms) {
    uint8_t data_length = 6;
    uint8_t data_received[data_length]; 

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(ADCS_TELEMETRY_GET_CURRENT_UNIX_TIME, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_unix_time_ms(data_received, epoch_time_ms);

    return tlm_status;
}

/// @brief Instruct the ADCS to execute the ADCS_Set_Run_Mode command.
/// @param[in] which_log 1 or 2; which specific log number to log to the SD card
/// @param[in] log_array Pointer to list of bitmasks to set the log config
/// @param[in] log_array_len Number of things to log
/// @param[in] log_period Period to log data to the SD card; if zero, then disable logging
/// @param[in] which_sd Which SD card to log to; 0 for primary, 1 for secondary 
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_set_sd_log_config(uint8_t which_log, const uint8_t **log_array, uint8_t log_array_len, uint16_t log_period, ADCS_sd_log_destination_enum_t which_sd) {
    uint8_t data_send[13];
    ADCS_combine_sd_log_bitmasks(log_array, log_array_len, data_send); // saves to the first 10 bytes of data_send
    ADCS_convert_uint16_to_reversed_uint8_array_members(data_send, log_period, 10);
    data_send[12] = which_sd;
    
    uint8_t command_id;
    switch (which_log) {
        case 1:
            command_id = ADCS_COMMAND_CUBEACP_SET_SD_LOG1_CONFIG;
            break;
        case 2:
            command_id = ADCS_COMMAND_CUBEACP_SET_SD_LOG2_CONFIG;
            break;
        default:
            return 7; // invalid log to log
    }
    
    const uint8_t cmd_status = ADCS_i2c_send_command_and_check(command_id, data_send, sizeof(data_send), ADCS_INCLUDE_CHECKSUM);
    return cmd_status;
}

/// @brief Instruct the ADCS to execute the ADCS_get_sd_log_config command.
/// @param[in] config Pointer to struct to store the config data
/// @param[in] which_log 1 or 2; which specific log number to log to the SD card
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
uint8_t ADCS_get_sd_log_config(uint8_t which_log, ADCS_sd_log_config_struct* config) {
    uint8_t data_length = 13;
    uint8_t data_received[data_length]; 

    uint8_t command_id;
    switch (which_log) {
        case 1:
            command_id = ADCS_TELEMETRY_CUBEACP_GET_SD_LOG1_CONFIG;
            break;
        case 2:
            command_id = ADCS_TELEMETRY_CUBEACP_GET_SD_LOG2_CONFIG;
            break;
        default:
            return 7; // invalid log to log
    }

    const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(command_id, data_received, data_length, ADCS_INCLUDE_CHECKSUM); 

    ADCS_pack_to_sd_log_config_struct(data_received, which_log, config);

    return tlm_status;
}

uint8_t adcs_download_buffer[20480]; // static buffer to hold the 20 KiB from the download

/// @brief Save one block of the ADCS SD card file pointed to by the file pointer into the ADCS download buffer, then append it to the file in LittleFS.
/// @param[in] file_info A struct containing information about the currently-pointed-to file
/// @param[in] current_block The block to load into the download buffer
/// @param[in] filename Pointer to string containing filename to save as
/// @param[in] filename_length Length of filename
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission.
/// Specifically: bytes 0-2 are the ADCS error, bytes 3-10 are which command failed, bytes 11-16 are the index of the failure if applicable
int16_t ADCS_load_sd_file_block_to_filesystem(ADCS_file_info_struct_t file_info, uint8_t current_block, char* filename_string, uint8_t filename_length) {
    ADCS_file_download_buffer_struct_t download_packet;
    uint16_t hole_map[64] = {0, 0, 0, 0, 0, 0, 0, 0}; // this array is 1024 bits, one for each packet up to the maximum

    ADCS_load_file_download_block(file_info.file_type, file_info.file_counter, current_block * 20480, 1024);
    // per CubeSpace, the counter is the nth file of the same type, not the block counter
    
    // to avoid interference from the EPS, do a separate ack for these commands
    ADCS_cmd_ack_struct_t ack_status;
    ADCS_cmd_ack(&ack_status);
    if (ack_status.error_flag != 0) {
        return ack_status.error_flag | (1 << 3);
    }
    
    // Wait until the download block is ready
    ADCS_download_block_ready_struct_t ready_struct;
    uint8_t download_block_tries = 0;
    do {
        const uint8_t block_ready_status = ADCS_get_download_block_ready_telemetry(&ready_struct);
        if (block_ready_status != 0) {
            return block_ready_status | (1 << 4);
        }
        download_block_tries++;
        if (download_block_tries > 100) {
            return 44;
        }
    } while (ready_struct.ready != true);

    // Initiate download burst, ignoring the hole map
    const uint8_t download_burst_status = ADCS_initiate_download_burst(true);
    if (download_burst_status != 0) {
        return download_burst_status | (1 << 5);
    }

    HAL_Delay(100); // need to allow some time for it to initiate the burst, or else the first packet may be garbage
    
    for (uint16_t i = 0; i < 1024; i++) {
        // load 20 bytes at a time into the download buffer
        const uint8_t download_buffer_status = ADCS_get_file_download_buffer(&(download_packet));
        if (download_buffer_status != 0) {
            return download_buffer_status | (1 << 6);
        }

        const uint16_t mini_packet_counter = download_packet.packet_counter % 1024;
        // when download_packet.packet_counter >= 1024, we access memory outside the buffer, but we want to overwrite it instead
        
        for (uint8_t j = 0; j < 20; j++) {
            adcs_download_buffer[20 * mini_packet_counter + j] = download_packet.file_bytes[j];
        }

        // generate hole map to determine any missing packets
        hole_map[mini_packet_counter / 16] |= (1 << (mini_packet_counter % 16));

    }
    
    /* HOLE MAP STUFF STARTS HERE */

    uint16_t required_packets;
    if (ceil(file_info.file_size / 20480.0) == (current_block + 1)) {
        // if this is the final block, we may need to load fewer packets than the maximum
        required_packets = ceil((file_info.file_size - (20480 * current_block)) / 20.0);
    } else {
        required_packets = 1024; 
    }        
    
    uint16_t packets_received = 0;
    
    for (uint8_t i = 0; i < 64; i++) {
        
        // check if we've received all the packets
        uint16_t temp_hole_map = hole_map[i];
        
        while (temp_hole_map) {
            // count the number of ones in the uint16 (Kernighan algorithm)
            temp_hole_map &= (temp_hole_map - 1);
            packets_received++;
        }
        
    }

    uint8_t hole_map_attempts = 0;

    while (required_packets < packets_received) {

        for (uint8_t i = 0; i < 8; i++) {
            uint8_t hole_map_slice[16];

            for (uint8_t j = 0; j < 8; j++) {
                // for each slice, create the hole map slice pertaining to it
                ADCS_convert_uint16_to_reversed_uint8_array_members(&hole_map_slice[((i*8) + j) / 2], hole_map[((i*8) + j)], 0);
            }

            // now send the hole map to the ADCS, one-indexed
            const uint8_t hole_map_status = ADCS_set_hole_map(hole_map_slice, (i + 1));
            if (hole_map_status != 0) {
                return hole_map_status | (1 << 7) | ((i + 1) << 11);
            }
        }
        
        // now, using the hole map as a guide, give us the missing packets
        const uint8_t download_burst_status = ADCS_initiate_download_burst(false);
        if (download_burst_status != 0) {
            return download_burst_status | (1 << 8);
        }
        
        HAL_Delay(100);

        for (uint16_t i = 0; i < 1024; i++) {
            // load 20 bytes at a time into the download buffer
            const uint8_t download_buffer_status = ADCS_get_file_download_buffer(&(download_packet));
            if (download_buffer_status != 0) {
                return download_buffer_status | (1 << 9);
            }

            const uint16_t mini_packet_counter = download_packet.packet_counter % 1024;
            // when download_packet.packet_counter >= 1024, we access memory outside the buffer, but we want to overwrite it instead

            for (uint8_t j = 0; j < 20; j++) {
                adcs_download_buffer[20 * mini_packet_counter + j] = download_packet.file_bytes[j];
            }

            // generate hole map to determine any missing packets
            hole_map[mini_packet_counter / 16] |= (1 << (mini_packet_counter % 16));
        }

        packets_received = 0;

        for (uint8_t i = 0; i < 64; i++) {
        
            // check if we've received all the packets
            uint16_t temp_hole_map = hole_map[i];
            
            while (temp_hole_map) {
                // count the number of ones in the uint16 (Kernighan algorithm)
                temp_hole_map &= (temp_hole_map - 1);
                packets_received++;
            }
            
        }

        hole_map_attempts++;
        if (hole_map_attempts >= 5) {
            if (file_info.file_type == ADCS_FILE_TYPE_INDEX) {
                break; // there's no way to ask the ADCS how large the index file is, so do this arbitrarily
            }
            return 7; // hole map timeout
        }

    }

    /* HOLE MAP STUFF ENDS HERE */
    
    // Write to the index file in the filesystem
    uint8_t lfs_status;
    if (current_block == 0) {
        lfs_status = LFS_write_file(filename_string, &adcs_download_buffer[0], 20480); 
    } else {
        lfs_status = LFS_append_file(filename_string, &adcs_download_buffer[0], 20480);
    }
    if (lfs_status != 0) {
        return lfs_status;
    }

    return 0;

}


/// @brief Save a specified file from the ADCS SD card to the ADCS subfolder in LittleFS.
/// @param[in] index_file_bool Whether this is the index file or not
/// @param[in] file_index Index of the file in the SD card; only used if index_file_bool is false
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred in transmission, negative if an LFS or snprintf error code occurred. 
/// Specifically, assuming no HAL or LFS error: bytes 0-2 are the ADCS error, bytes 3-10 are which command failed, bytes 11-16 are the index of the failure if applicable
int16_t ADCS_save_sd_file_to_lfs(bool index_file_bool, uint16_t file_index) {

    ADCS_file_info_struct_t file_info;
    int16_t snprintf_ret;

    char filename_string[17];

    if (!index_file_bool) {
        // get the data about the file to download

        /*
        The file is uniquely identified by the File Type and Counter parameters. The Offset and Block
        Length parameters indicate which part of the file to buffer in memory. The maximum Block Length is [20 KiB]. 
        [20480 bytes (20 bytes * 1024 packets). Some files may require multiple blocks, such as image files]
        */
            
        const uint8_t reset_pointer_status = ADCS_reset_file_list_read_pointer();
        HAL_Delay(100);
        if (reset_pointer_status != 0) {
            // to avoid interference from the EPS, do a separate ack for these commands
            ADCS_cmd_ack_struct_t ack_status;
            ADCS_cmd_ack(&ack_status);
            if (ack_status.error_flag != 0) {
                return ack_status.error_flag;
            }
        }

        for (uint16_t i = 0; i < file_index; i++) {
            const uint8_t advance_pointer_status = ADCS_advance_file_list_read_pointer();
            HAL_Delay(100);
            if (advance_pointer_status != 0) {
                // to avoid interference from the EPS, do a separate ack for these commands
                ADCS_cmd_ack_struct_t ack_status;
                ADCS_cmd_ack(&ack_status);
                if (ack_status.error_flag != 0) {
                    return ack_status.error_flag;
                }
            }
        }
        
        const uint8_t file_info_status = ADCS_get_file_info_telemetry(&file_info);
        if (file_info_status != 0) {
            return file_info_status;
        }

        if (file_info.file_counter != file_index) {
            return 12;
        } // make sure that we've properly advanced the file list read pointer

        // name file based on type and timestamp
        switch(file_info.file_type) {

            case ADCS_FILE_TYPE_TELEMETRY_LOG:
                snprintf_ret = snprintf(filename_string, 17, "ADCS/log%d.TLM", file_index);
                if (snprintf_ret < 0) {
                    return snprintf_ret;
                };
                break;
            case ADCS_FILE_TYPE_JPG_IMAGE:
                snprintf_ret = snprintf(filename_string, 17, "ADCS/img%d.jpg", file_index);
                if (snprintf_ret < 0) {
                    return snprintf_ret;
                };
                break;    
            case ADCS_FILE_TYPE_BMP_IMAGE:
                snprintf_ret = snprintf(filename_string, 17, "ADCS/img%d.bmp", file_index);
                if (snprintf_ret < 0) {
                    return snprintf_ret;
                };
                break;    
            case ADCS_FILE_TYPE_INDEX:
                snprintf_ret = snprintf(filename_string, 17, "ADCS/index_file");
                if (snprintf_ret < 0) {
                    return snprintf_ret;
                };
                break;    
            default:
                return 1;
        }

    } else {
        snprintf_ret = snprintf(filename_string, 16, "ADCS/index_file");
        if (snprintf_ret < 0) {
            return snprintf_ret;
        };
        file_info.file_size = 20479; // for the index file, we should only need a single block
        file_info.file_type = ADCS_FILE_TYPE_INDEX;
    }

    uint8_t total_blocks = ceil(file_info.file_size / 20480.0);
    uint8_t current_block = 0; 

    while (current_block < total_blocks) {

        LOG_message(LOG_SYSTEM_ADCS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL, "Loading block %d from ADCS to LittleFS", current_block);
        const uint8_t load_block_status = ADCS_load_sd_file_block_to_filesystem(file_info, current_block, filename_string, 17); // load the block
        if (load_block_status != 0) {
            return load_block_status;
        }

        current_block++;
    }

    return 0;
}
/// @brief Disable all active ADCS SD card logs.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred.
uint8_t ADCS_disable_SD_logging() {
    // Disable SD card logging
    const uint8_t* temp_data_pointer[1] = {ADCS_SD_LOG_MASK_NONE};
    const uint8_t sd_log_1_stop_status = ADCS_set_sd_log_config(1, temp_data_pointer, 0, 0, 0);                     
    if (sd_log_1_stop_status != 0) {
        return sd_log_1_stop_status;
    }
    const uint8_t sd_log_2_stop_status = ADCS_set_sd_log_config(2, temp_data_pointer, 0, 0, 0);                     
    return sd_log_2_stop_status;
}

/// @brief Disable all ADCS peripherals and active SD card logs.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred.
uint8_t ADCS_disable_peripherals_and_SD_logs_without_stabilisation() {
    const uint8_t power_status = ADCS_set_power_control(ADCS_POWER_SELECT_SAME, ADCS_POWER_SELECT_SAME, ADCS_POWER_SELECT_OFF, ADCS_POWER_SELECT_OFF, ADCS_POWER_SELECT_OFF, ADCS_POWER_SELECT_OFF, ADCS_POWER_SELECT_OFF, ADCS_POWER_SELECT_OFF, ADCS_POWER_SELECT_OFF, ADCS_POWER_SELECT_OFF);
    if (power_status != 0) {
        return power_status;
    }
    const uint8_t sd_status = ADCS_disable_SD_logging();
    return sd_status;
}

/// @brief Disable all ADCS peripherals and active SD card logs except the CubeSense Motor and Signal power, required for attitude stabilisation.
/// @note If CubeSense Motor and/or Signal Power are already off, they will remain off.
/// @note Compared to without_stabilisation, this costs average 250 mW, maximum 1 W extra.
/// @return 0 if successful, non-zero if a HAL or ADCS error occurred.
uint8_t ADCS_disable_peripherals_and_SD_logs_with_stabilisation() {
    const uint8_t power_status = ADCS_set_power_control(ADCS_POWER_SELECT_SAME, ADCS_POWER_SELECT_SAME, ADCS_POWER_SELECT_OFF, ADCS_POWER_SELECT_OFF, ADCS_POWER_SELECT_OFF, ADCS_POWER_SELECT_OFF, ADCS_POWER_SELECT_OFF, ADCS_POWER_SELECT_OFF, ADCS_POWER_SELECT_OFF, ADCS_POWER_SELECT_OFF);
    if (power_status != 0) {
        return power_status;
    }
    const uint8_t sd_status = ADCS_disable_SD_logging();
    return sd_status;
}