#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_args_helpers.h"
#include "transforms/arrays.h"
#include "unit_tests/unit_test_executor.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include "adcs_types.h"

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_ack(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_TC_Ack_Struct ack;
    ADCS_TC_Ack(&ack);
    return ack.error_flag;
}

/// @brief Telecommand: Set the wheel speed of the ADCS
/// @param args_str 
///     - Arg 0: Wheel speed X value
///     - Arg 1: Wheel speed Y value
///     - Arg 2: Wheel speed Z value
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_set_wheel_speed(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    // parse arguments: first into uint64_t, then convert to correct form for input
    uint8_t num_args = 3;
    uint64_t arguments[num_args]; 
    uint16_t args_16[num_args];
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_uint64_arg(args_str, strlen((char*)args_str), i, &arguments[i]);
        args_16[i] = (uint16_t) arguments[i];
    }
    
    uint8_t status = ADCS_Set_Wheel_Speed(args_16[0], args_16[1], args_16[2]); 
    return status;
}

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_reset(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                            char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Reset(); 
    return status;
}                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_identification(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Identification(); 
    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_program_status(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Program_Status(); 
    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_communication_status(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                           char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Communication_Status(); 
    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_deploy_magnetometer(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                          char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t timeout;
    TCMD_extract_uint64_arg(args_str, strlen((char*)args_str), 0, &timeout);
    uint8_t status = ADCS_Deploy_Magnetometer((uint8_t) timeout);
    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_set_run_mode(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t run_mode;
    TCMD_extract_uint64_arg(args_str, strlen((char*)args_str), 0, &run_mode);
    uint8_t status = ADCS_Set_Run_Mode((ADCS_Run_Mode) run_mode); 
    return status;
}                        

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_clear_errors(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Clear_Errors();
    return status;
}                        

/// @brief Telecommand: Set the attitude control mode of the ADCS
/// @param args_str 
///     - Arg 0: Control mode to set (Table 77 in Firmware Manual)
///     - Arg 1: Timeout to set control mode
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_attitude_control_mode(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    // parse arguments into uint64_t
    uint8_t num_args = 2;
    uint64_t arguments[num_args]; 
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_uint64_arg(args_str, strlen((char*)args_str), i, &arguments[i]);
    }
    // then convert to correct form for input
    uint8_t status = ADCS_Attitude_Control_Mode((ADCS_Control_Mode) arguments[0], (uint16_t) arguments[1]);
    return status;
}                                                         

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_attitude_estimation_mode(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                               char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t estimation_mode;
    TCMD_extract_uint64_arg(args_str, strlen((char*)args_str), 0, &estimation_mode);
    uint8_t status = ADCS_Attitude_Estimation_Mode((ADCS_Estimation_Mode) estimation_mode); 
    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_run_once(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                               char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Run_Once();
    return status;
}                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - Arg 0: magnetometer mode to set
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_set_magnetometer_mode(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t mode;
    TCMD_extract_uint64_arg(args_str, strlen((char*)args_str), 0, &mode);
    uint8_t status = ADCS_Set_Magnetometer_Mode((ADCS_Magnetometer_Mode) mode);
    return status;
}                                

/// @brief Telecommand: Set the magnetorquer output values
/// @param args_str 
///     - Arg 0: Magnetorquer X duty cycle
///     - Arg 1: Magnetorquer Y duty cycle
///     - Arg 2: Magnetorquer Z duty cycle
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_set_magnetorquer_output(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
     // parse arguments into doubles
    uint8_t num_args = 3;
    double arguments[num_args]; 
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_double_arg(args_str, strlen((char*)args_str), i, &arguments[i]);
    }
    
    uint8_t status = ADCS_Set_Magnetorquer_Output(arguments[0], arguments[1], arguments[2]);
    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_save_config(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                  char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Save_Config();
    return status;
}                        

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_estimate_angular_rates(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                             char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Estimate_Angular_Rates();
    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_get_llh_position(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                       char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Get_LLH_Position();
    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_get_power_control(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Get_Power_Control();
    return status;
}   

/// @brief Telecommand: Request the given telemetry data from the ADCS (power off mode = 0, power on mode = 1, power same mode = 2)
/// @param args_str 
///     - Arg 0: Power control mode for cube control signal
///     - Arg 1: Power control mode for cube control motor
///     - Arg 2: Power control mode for cube sense 1
///     - Arg 3: Power control mode for cube sense 2
///     - Arg 4: Power control mode for cube star
///     - Arg 5: Power control mode for cube wheel 1
///     - Arg 6: Power control mode for cube wheel 2
///     - Arg 7: Power control mode for cube wheel 3
///     - Arg 8: Power control mode for motor
///     - Arg 9: Power control mode for gps
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_set_power_control(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                        char *response_output_buf, uint16_t response_output_buf_len) {

    // parse arguments: first into uint64_t, then convert to correct form for input
    uint8_t num_args = 10;
    uint64_t arguments[num_args]; 
    uint8_t args_8[num_args];
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_uint64_arg(args_str, strlen((char*)args_str), i, &arguments[i]);
        args_8[i] = (uint8_t) arguments[i];
    }
    
    uint8_t status = ADCS_Set_Power_Control((ADCS_Power_Select) args_8[0], (ADCS_Power_Select) args_8[1], (ADCS_Power_Select) args_8[2], (ADCS_Power_Select) args_8[3], (ADCS_Power_Select) args_8[4], (ADCS_Power_Select) args_8[5], (ADCS_Power_Select) args_8[6], (ADCS_Power_Select) args_8[7], (ADCS_Power_Select) args_8[8], (ADCS_Power_Select) args_8[9]);
    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - Arg 0: Mounting transform alpha angle [deg] 
///     - Arg 1: Mounting transform beta angle [deg]
///     - Arg 2: Mounting transform gamma angle [deg]
///     - Arg 3: Channel 1 offset value
///     - Arg 4: Channel 2 offset value
///     - Arg 5: Channel 3 offset value
///     - Arg 0: Value (1, 1) of the magnetometer sensitivity matrix
///     - Arg 0: Value (2, 2) of the magnetometer sensitivity matrix
///     - Arg 0: Value (3, 3) of the magnetometer sensitivity matrix
///     - Arg 0: Value (1, 2) of the magnetometer sensitivity matrix
///     - Arg 0: Value (1, 3) of the magnetometer sensitivity matrix
///     - Arg 0: Value (2, 1) of the magnetometer sensitivity matrix
///     - Arg 0: Value (2, 3) of the magnetometer sensitivity matrix
///     - Arg 0: Value (3, 1) of the magnetometer sensitivity matrix
///     - Arg 0: Value (3, 2) of the magnetometer sensitivity matrix
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_set_magnetometer_config(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {

    // parse arguments into doubles
    uint8_t num_args = 15;
    double arguments[num_args]; 
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_double_arg(args_str, strlen((char*)args_str), i, &arguments[i]);
    }
    
    uint8_t status = ADCS_Set_Magnetometer_Config(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14]);
    return status;
}     

// TODO: 7 telecommands remaining! (Find the placeholder 255 values and deal with them)

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_bootloader_clear_errors(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Bootloader_Clear_Errors(); 
    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_set_unix_time_save_mode(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_get_unix_time_save_mode(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Get_Unix_Time_Save_Mode();
    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_set_sgp4_orbit_params(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_get_sgp4_orbit_params(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Get_SGP4_Orbit_Params();
    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_save_orbit_params(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Save_Orbit_Params();
    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_rate_sensor_rates(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Rate_Sensor_Rates();
    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_get_wheel_speed(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Get_Wheel_Speed();
    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_get_magnetorquer_command(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                               char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Get_Magnetorquer_Command();
    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_get_raw_magnetometer_values(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                  char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Get_Raw_Magnetometer_Values();
    return status;
}                                        

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_fine_angular_rates(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                         char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Fine_Angular_Rates();
    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_estimate_fine_angular_rates(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                  char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Estimate_Fine_Angular_Rates(); 
    return status;
}                                        

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_get_magnetometer_config(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Get_Magnetometer_Config();
    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_get_commanded_attitude_angles(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                    char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Get_Commanded_Attitude_Angles();
    return status;
}                                        

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_set_commanded_attitude_angles(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                    char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                        

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_set_estimation_params(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_get_estimation_params(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Get_Estimation_Params();
    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_set_asgp4_params(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                       char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_get_asgp4_params(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                       char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Get_ASGP4_Params();
    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_set_tracking_controller_target_reference(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                                char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_get_tracking_controller_target_reference(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                                char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Get_Tracking_Controller_Target_Reference();
    return status;
}                                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_set_rate_gyro_config(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                           char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = 255; // this is a placeholder for now;
    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_get_rate_gyro_config(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                           char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Get_Rate_Gyro_Config();
    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_estimated_attitude_angles(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Estimated_Attitude_Angles();
    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_magnetic_field_vector(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Magnetic_Field_Vector();
    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_fine_sun_vector(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Fine_Sun_Vector();
    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_nadir_vector(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Nadir_Vector();
    return status;
}                        

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_commanded_wheel_speed(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Commanded_Wheel_Speed();
    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_igrf_magnetic_field_vector(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                 char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_IGRF_Magnetic_Field_Vector();
    return status;
}                                        

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_quaternion_error_vector(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Quaternion_Error_Vector();
    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_estimated_gyro_bias(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                          char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Estimated_Gyro_Bias();
    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_estimation_innovation_vector(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                   char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Estimation_Innovation_Vector();
    return status;
}                                        

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_raw_cam1_sensor(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Raw_Cam1_Sensor();
    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_raw_cam2_sensor(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Raw_Cam2_Sensor();
    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_raw_css_1_to_6(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Raw_CSS_1_to_6();
    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_raw_css_7_to_10(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Raw_CSS_7_to_10();
    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_cubecontrol_current(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                          char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_CubeControl_Current();
    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_raw_gps_status(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Raw_GPS_Status();
    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_raw_gps_time(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Raw_GPS_Time();
    return status;
}                        

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_raw_gps_x(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Raw_GPS_X();
    return status;
}                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_raw_gps_y(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Raw_GPS_Y();
    return status;
}                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_raw_gps_z(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Raw_GPS_Z();
    return status;
}                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ADCS_measurements(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Measurements(); 
    return status;
}                        

// TODO: telecommand definitions
// TODO also: all the docstrings from telecommands with more than 1 arg should be ported to adcs_types.c