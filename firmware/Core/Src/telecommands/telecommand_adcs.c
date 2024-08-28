#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_args_helpers.h"
#include "transforms/arrays.h"
#include "unit_tests/unit_test_executor.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include "adcs_drivers/adcs_types.h"
#include "adcs_drivers/adcs_commands.h"
#include "adcs_drivers/adcs_struct_packers.h"
#include "adcs_drivers/adcs_types_to_json.h"

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_ack(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_CMD_Ack_Struct ack;
    ADCS_CMD_Ack(&ack);
    return ack.error_flag;
}

/// @brief Telecommand: Set the wheel speed of the ADCS
/// @param args_str 
///     - Arg 0: wheel speed x value
///     - Arg 1: wheel speed y value
///     - Arg 2: wheel speed z value
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_wheel_speed(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    // parse arguments: first into int64_t, then convert to correct form for input
    const uint8_t num_args = 3;
    int64_t arguments[num_args]; 
    int16_t args_16[num_args];
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_int64_arg(args_str, strlen(args_str), i, &arguments[i]);
        args_16[i] = (int16_t) arguments[i];
    }
    
    uint8_t status = ADCS_Set_Wheel_Speed(args_16[0], args_16[1], args_16[2]); 
    return status;
}

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_reset(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                            char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Reset(); 
    return status;
}                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_identification(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_ID_Struct packed_struct;
    uint8_t status = ADCS_Get_Identification(&packed_struct); 
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_ID_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_program_status(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Boot_Running_Status_Struct packed_struct;
    uint8_t status = ADCS_Get_Program_Status(&packed_struct); 
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Boot_Running_Status_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_communication_status(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                           char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Comms_Status_Struct packed_struct;
    uint8_t status = ADCS_Get_Communication_Status(&packed_struct); 
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Comms_Status_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_deploy_magnetometer(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                          char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t timeout;
    TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &timeout);
    uint8_t status = ADCS_Deploy_Magnetometer((uint8_t) timeout);
    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_run_mode(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t run_mode;
    TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &run_mode);
    uint8_t status = ADCS_Set_Run_Mode((ADCS_Run_Mode) run_mode); 
    return status;
}                        

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_clear_errors(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Clear_Errors();
    return status;
}                        

/// @brief Telecommand: Set the attitude control mode of the ADCS; needs Power Control to be set before working
/// @param args_str 
///     - Arg 0: Control mode to set (Table 77 in Firmware Manual)
///     - Arg 1: Timeout to set control mode
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_attitude_control_mode(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    // parse arguments into uint64_t
    const uint8_t num_args = 2;
    uint64_t arguments[num_args]; 
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_uint64_arg(args_str, strlen(args_str), i, &arguments[i]);
    }
    // then convert to correct form for input
    uint8_t status = ADCS_Attitude_Control_Mode((ADCS_Control_Mode) arguments[0], (uint16_t) arguments[1]);
    return status;
}                                                         

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_attitude_estimation_mode(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                               char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t estimation_mode;
    TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &estimation_mode);
    uint8_t status = ADCS_Attitude_Estimation_Mode((ADCS_Estimation_Mode) estimation_mode); 
    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_run_once(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                               char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Run_Once();
    return status;
}                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - Arg 0: magnetometer mode to set
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_magnetometer_mode(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t mode;
    TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &mode);
    uint8_t status = ADCS_Set_Magnetometer_Mode((ADCS_Magnetometer_Mode) mode);
    return status;
}                                

/// @brief Telecommand: Set the magnetorquer output values
/// @param args_str 
///     - Arg 0: magnetorquer x duty cycle
///     - Arg 1: magnetorquer y duty cycle
///     - Arg 2: magnetorquer z duty cycle
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_magnetorquer_output(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
     // parse arguments into doubles
    const uint8_t num_args = 3;
    double arguments[num_args]; 
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_double_arg(args_str, strlen(args_str), i, &arguments[i]);
    }
    
    uint8_t status = ADCS_Set_Magnetorquer_Output(arguments[0], arguments[1], arguments[2]);
    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_save_config(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                  char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Save_Config();
    return status;
}                        

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_estimate_angular_rates(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                             char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Angular_Rates_Struct packed_struct;
    uint8_t status = ADCS_Get_Estimate_Angular_Rates(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Angular_Rates_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_llh_position(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                       char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_LLH_Position_Struct packed_struct;
    uint8_t status = ADCS_Get_LLH_Position(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_LLH_Position_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_power_control(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                        char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Power_Control_Struct packed_struct;
    uint8_t status = ADCS_Get_Power_Control(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Power_Control_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

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
uint8_t TCMDEXEC_adcs_set_power_control(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                        char *response_output_buf, uint16_t response_output_buf_len) {

    // parse arguments: first into uint64_t, then convert to correct form for input
    const uint8_t num_args = 10;
    uint64_t arguments[num_args]; 
    uint8_t args_8[num_args];
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_uint64_arg(args_str, strlen(args_str), i, &arguments[i]);
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
///     - Arg 6: Value (1, 1) of the magnetometer sensitivity matrix
///     - Arg 7: Value (2, 2) of the magnetometer sensitivity matrix
///     - Arg 8: Value (3, 3) of the magnetometer sensitivity matrix
///     - Arg 9: Value (1, 2) of the magnetometer sensitivity matrix
///     - Arg 10: Value (1, 3) of the magnetometer sensitivity matrix
///     - Arg 11: Value (2, 1) of the magnetometer sensitivity matrix
///     - Arg 12: Value (2, 3) of the magnetometer sensitivity matrix
///     - Arg 13: Value (3, 1) of the magnetometer sensitivity matrix
///     - Arg 14: Value (3, 2) of the magnetometer sensitivity matrix
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_magnetometer_config(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {

    // parse arguments into doubles
    const uint8_t num_args = 15;
    double arguments[num_args]; 
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_double_arg(args_str, strlen(args_str), i, &arguments[i]);
    }
    
    uint8_t status = ADCS_Set_Magnetometer_Config(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14]);
    return status;
}     

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_bootloader_clear_errors(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Bootloader_Clear_Errors(); 
    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - Arg 0: whether to save the current Unix time immediately
///     - Arg 1: whether to save the current Unix time whenever a command is used to update it
///     - Arg 2: whether to save the current Unix time periodically
///     - Arg 3: the period of saving the current Unix time
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_unix_time_save_mode(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t bools[3];
    uint64_t uint_arg;
    for (uint8_t i = 0; i < 3; i++) {
        TCMD_extract_uint64_arg(args_str, strlen(args_str), i, &bools[i]);
    }
    
    TCMD_extract_uint64_arg(args_str, strlen(args_str), 3, &uint_arg);

    uint8_t status = ADCS_Set_Unix_Time_Save_Mode((bool) bools[0], (bool) bools[1], (bool) bools[2], (uint8_t) uint_arg);
    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_unix_time_save_mode(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Set_Unix_Time_Save_Mode_Struct packed_struct;
    uint8_t status = ADCS_Get_Unix_Time_Save_Mode(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Set_Unix_Time_Save_Mode_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                                    

/// @brief Telecommand: Set the ADCS Simplified General Perturbations (SGP4) orbit parameters
/// @param args_str 
///     - Arg 0: inclination (degrees)
///     - Arg 1: eccentricity (dimensionless)
///     - Arg 2: right ascension of the ascending node (degrees)
///     - Arg 3: argument of perigee (degrees)
///     - Arg 4: b-star drag term (dimensionless)
///     - Arg 5: mean motion (orbits per day)
///     - Arg 6: mean anomaly (degrees)
///     - Arg 7: epoch (double; integer component is year, decimal component is day)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_sgp4_orbit_params(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    // parse arguments into doubles
    const uint8_t num_args = 8;
    double arguments[num_args]; 
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_double_arg(args_str, strlen(args_str), i, &arguments[i]);
    }

    uint8_t status = ADCS_Set_SGP4_Orbit_Params(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7]);
    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_sgp4_orbit_params(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Orbit_Params_Struct packed_struct;
    uint8_t status = ADCS_Get_SGP4_Orbit_Params(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Orbit_Params_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_save_orbit_params(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_Save_Orbit_Params();
    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_rate_sensor_rates(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                        char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Rated_Sensor_Rates_Struct packed_struct;
    uint8_t status = ADCS_Get_Rate_Sensor_Rates(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Rated_Sensor_Rates_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_wheel_speed(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Wheel_Speed_Struct packed_struct;
    uint8_t status = ADCS_Get_Wheel_Speed(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Wheel_Speed_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_magnetorquer_command(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                               char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Magnetorquer_Command_Struct packed_struct;
    uint8_t status = ADCS_Get_Magnetorquer_Command(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Magnetorquer_Command_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_raw_magnetometer_values(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                  char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Raw_Magnetometer_Values_Struct packed_struct;
    uint8_t status = ADCS_Get_Raw_Magnetometer_Values(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Raw_Magnetometer_Values_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                                                                 

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_estimate_fine_angular_rates(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                  char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Fine_Angular_Rates_Struct packed_struct;
    uint8_t status = ADCS_Get_Estimate_Fine_Angular_Rates(&packed_struct); 
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Fine_Angular_Rates_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                                        

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_magnetometer_config(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Magnetometer_Config_Struct packed_struct;
    uint8_t status = ADCS_Get_Magnetometer_Config(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Magnetometer_Config_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_commanded_attitude_angles(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                    char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Commanded_Angles_Struct packed_struct;
    uint8_t status = ADCS_Get_Commanded_Attitude_Angles(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Commanded_Angles_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                                        

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - Arg 0: x attitude angle
///     - Arg 1: y attitude angle
///     - Arg 2: z attitude angle
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_commanded_attitude_angles(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                    char *response_output_buf, uint16_t response_output_buf_len) {
    // parse arguments into doubles
    const uint8_t num_args = 3;
    double arguments[num_args]; 
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_double_arg(args_str, strlen(args_str), i, &arguments[i]);
    }

    uint8_t status = ADCS_Set_Commanded_Attitude_Angles(arguments[0], arguments[1], arguments[2]); 
    return status;
}          

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - Arg 0: magnetometer_rate_filter_system_noise (Magnetometer rate filter system noise covariance parameter)
///     - Arg 1: ekf_system_noise (EKS system noise covariance parameter)
///     - Arg 2: css_measurement_noise (CSS measurement noise covariance parameter)
///     - Arg 3: sun_sensor_measurement_noise (sun sensor measurement noise covariance parameter)
///     - Arg 4: nadir_sensor_measurement_noise (nadir sensor measurement noise covariance parameter)
///     - Arg 5: magnetometer_measurement_noise (magnetometer measurement noise covariance parameter)
///     - Arg 6: star_tracker_measurement_noise (star tracker measurement noise covariance parameter)
///     - Arg 7: use_sun_sensor (whether or not to use the sun sensor measurement in EKF)
///     - Arg 8: use_nadir_sensor (whether or not to use the nadir sensor measurement in EKF)
///     - Arg 9: use_css (whether or not to use the CSS measurement in EKF)
///     - Arg 10: use_star_tracker (whether or not to use the star tracker measurement in EKF)
///     - Arg 11: nadir_sensor_terminator_test (select to ignore nadir sensor measurements when terminator is in FOV)
///     - Arg 12: automatic_magnetometer_recovery (select whether automatic switch to redundant magnetometer should occur in case of failure)
///     - Arg 13: magnetometer_mode (select magnetometer mode for estimation and control)
///     - Arg 14: magnetometer_selection_for_raw_mtm_tlm (select magnetometer mode for the second raw telemetry frame)
///     - Arg 15: automatic_estimation_transition_due_to_rate_sensor_errors (enable/disable automatic transition from MEMS rate estimation mode to RKF in case of rate sensor error)
///     - Arg 16: wheel_30s_power_up_delay (present in CubeSupport but not in the manual -- need to test)
///     - Arg 17: cam1_and_cam2_sampling_period (the manual calls it this, but CubeSupport calls it "error counter reset period" -- need to test)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_estimation_params(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    // the first seven are floats (0-6)
    // the next six are bools (0-5)
    // after that there are two enums (6-7)
    // and then two more bools (8-9)
    // followed by a uint8 (10)

    // in other words, seven double-types followed by eleven uint64-types
    
    const uint8_t num_args = 7;
    double double_type_arguments[num_args]; 
    float float_args[num_args];
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_double_arg(args_str, strlen(args_str), i, &double_type_arguments[i]);
        float_args[i] = double_type_arguments[i];
    }

    uint8_t new_num_args = 11;
    uint64_t uint_type_arguments[new_num_args]; 
    bool bool_args[8];
    ADCS_Magnetometer_Mode enum_args[2];
    uint8_t uint8_arg;

    for (uint8_t i = 0; i < new_num_args; i++) {
        TCMD_extract_uint64_arg(args_str, strlen(args_str), i + 7, &uint_type_arguments[i]);
        if (i < 6 || i == 8 || i == 9) {
            bool_args[i] = (bool) uint_type_arguments[i];
        }
    }
    enum_args[0] = (ADCS_Magnetometer_Mode) uint_type_arguments[6];
    enum_args[1] = (ADCS_Magnetometer_Mode) uint_type_arguments[7];
    uint8_arg = (uint8_t) uint_type_arguments[10];
    
    uint8_t status = ADCS_Set_Estimation_Params(float_args[0], float_args[1], float_args[2], float_args[3], float_args[4], float_args[5], float_args[6],
                                                bool_args[0], bool_args[1], bool_args[2], bool_args[3], bool_args[4], bool_args[5], 
                                                enum_args[0], enum_args[1],
                                                bool_args[6], bool_args[7], 
                                                uint8_arg); 
    
    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_estimation_params(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Estimation_Params_Struct packed_struct;
    uint8_t status = ADCS_Get_Estimation_Params(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Estimation_Params_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - Arg 0: incl_coefficient (Set inclination filter coefficient)
///     - Arg 1: raan_coefficient (Set RAAN filter coefficient)
///     - Arg 2: ecc_coefficient (Set eccentricity filter coefficient)
///     - Arg 3: aop_coefficient (Set argument of perigee filter coefficient)
///     - Arg 4: time_coefficient (Set time filter coefficient)
///     - Arg 5: pos_coefficient (Set position filter coefficient)
///     - Arg 6: maximum_position_error (Maximum position error for ASGP4 to continue working)
///     - Arg 7: asgp4_filter (The type of filter being used (enum))
///     - Arg 8: xp_coefficient (Polar coefficient xp)
///     - Arg 9: yp_coefficient (Polar coefficient yp)
///     - Arg 10: gps_roll_over (GPS roll over number)
///     - Arg 11: position_sd (Maximum position standard deviation for ASGP4 to continue working)
///     - Arg 12: velocity_sd (Maximum velocity standard deviation for ASGP4 to continue working)
///     - Arg 13: min_satellites (Minimum satellites required for ASGP4 to continue working)
///     - Arg 14: time_gain (Time offset compensation gain)
///     - Arg 15: max_lag (Maximum lagged timestamp measurements to incorporate)
///     - Arg 16: min_samples (Minimum samples to use to get ASGP4)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_asgp4_params(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                       char *response_output_buf, uint16_t response_output_buf_len) {
    // seven doubles, then enum, then two doubles, uint8, two doubles, uint8, two doubles, uint16
    //  0-6             7           8-9             10      11-12       13      14-15       16
    uint8_t total_doubles = 14;
    uint8_t total_uints = 4;
    
    double doubles_params[total_doubles];
    uint64_t uint_params[total_uints]; // includes enum

    const uint8_t num_args = 17;

    uint8_t double_counter = 0;
    uint8_t uint_counter = 0;

    for (uint8_t i = 0; i < num_args; i++) {
        if (i < 7 || (i == 8 || i == 9) || (i == 11 || i == 12) || (i == 14 || i == 15)) {
            TCMD_extract_double_arg(args_str, strlen(args_str), i, &doubles_params[double_counter]);
            double_counter++;
        }
        else if (i == 7 || i == 10 || i == 13 || i == 16) {
            TCMD_extract_uint64_arg(args_str, strlen(args_str), i, &uint_params[uint_counter]);
            uint_counter++;
        }
        else {
            return 5; // this return should never be reached, but it is here for debugging
        }
    }

    uint8_t status = ADCS_Set_ASGP4_Params(doubles_params[0], doubles_params[1], doubles_params[2], doubles_params[3], doubles_params[4], doubles_params[5], doubles_params[6],
                                        (ADCS_ASGP4_Filter) uint_params[0],
                                        doubles_params[7], doubles_params[8],
                                        (uint8_t) uint_params[1],
                                        doubles_params[9], doubles_params[10],
                                        (uint8_t) uint_params[2],
                                        doubles_params[11], doubles_params[12],
                                        (uint16_t) uint_params[3]); 
    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_asgp4_params(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                       char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_ASGP4_Params_Struct packed_struct;
    uint8_t status = ADCS_Get_ASGP4_Params(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_ASGP4_Params_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                            

/// @brief Telecommand: Set the ADCS tracking controller target reference (location on Earth to point towards)
/// @param args_str 
///     - Arg 0: longitude
///     - Arg 1: latitude
///     - Arg 2: altitude
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_tracking_controller_target_reference(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                                char *response_output_buf, uint16_t response_output_buf_len) {
    // parse arguments into doubles
    const uint8_t num_args = 3;
    double arguments[num_args]; 
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_double_arg(args_str, strlen(args_str), i, &arguments[i]);
    }

    uint8_t status = ADCS_Set_Tracking_Controller_Target_Reference((float) arguments[0], (float) arguments[1], (float) arguments[2]); 
    
    return status;
}                                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_tracking_controller_target_reference(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                                char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Tracking_Controller_Target_Struct packed_struct;
    uint8_t status = ADCS_Get_Tracking_Controller_Target_Reference(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Tracking_Controller_Target_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - Arg 0: gyro1 (Axis for Gyro #1; enum, options are pos/neg x, pos/neg y, pos/neg z)
///     - Arg 1: gyro2 (Axis for Gyro #2; enum, options are pos/neg x, pos/neg y, pos/neg z)
///     - Arg 2: gyro3 (Axis for Gyro #3; enum, options are pos/neg x, pos/neg y, pos/neg z)
///     - Arg 3: x_rate_offset (x-rate sensor offset)
///     - Arg 4: y_rate_offset (y-rate sensor offset)
///     - Arg 5: z_rate_offset (z-rate sensor offset)
///     - Arg 6: rate_sensor_mult (multiplier of rate sensor measurement)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_rate_gyro_config(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                           char *response_output_buf, uint16_t response_output_buf_len) {
    
    // parse axis select arguments into uint64s
    uint8_t num_axis_args = 3;
    uint64_t axis_arguments[num_axis_args]; 
    for (uint8_t i = 0; i < num_axis_args; i++) {
        TCMD_extract_uint64_arg(args_str, strlen(args_str), i, &axis_arguments[i]);
    }

    // parse axis select arguments into doubles
    uint8_t num_offset_args = 3;
    double offset_arguments[num_offset_args]; 
    for (uint8_t i = 0; i < num_offset_args; i++) {
        TCMD_extract_double_arg(args_str, strlen(args_str), i + 3, &offset_arguments[i]);
    }

    uint64_t rate_sensor_mult;
    TCMD_extract_uint64_arg(args_str, strlen(args_str), 6, &rate_sensor_mult);
    
    uint8_t status = ADCS_Set_Rate_Gyro_Config((ADCS_Axis_Select) axis_arguments[0], (ADCS_Axis_Select) axis_arguments[1], (ADCS_Axis_Select) axis_arguments[2],  offset_arguments[0], offset_arguments[1], offset_arguments[2], (uint8_t) rate_sensor_mult); 
    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_rate_gyro_config(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                           char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Rate_Gyro_Config_Struct packed_struct;
    uint8_t status = ADCS_Get_Rate_Gyro_Config(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Rate_Gyro_Config_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_estimated_attitude_angles(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Estimated_Attitude_Angles_Struct packed_struct;
    uint8_t status = ADCS_Get_Estimated_Attitude_Angles(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Estimated_Attitude_Angles_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_magnetic_field_vector(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Magnetic_Field_Vector_Struct packed_struct;
    uint8_t status = ADCS_Get_Magnetic_Field_Vector(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Magnetic_Field_Vector_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_fine_sun_vector(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Fine_Sun_Vector_Struct packed_struct;
    uint8_t status = ADCS_Get_Fine_Sun_Vector(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Fine_Sun_Vector_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_nadir_vector(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Nadir_Vector_Struct packed_struct;
    uint8_t status = ADCS_Get_Nadir_Vector(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Nadir_Vector_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                        

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_commanded_wheel_speed(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Wheel_Speed_Struct packed_struct;
    uint8_t status = ADCS_Get_Commanded_Wheel_Speed(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Wheel_Speed_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_igrf_magnetic_field_vector(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                 char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Magnetic_Field_Vector_Struct packed_struct;
    uint8_t status = ADCS_Get_IGRF_Magnetic_Field_Vector(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Magnetic_Field_Vector_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                                        

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_quaternion_error_vector(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Quaternion_Error_Vector_Struct packed_struct;
    uint8_t status = ADCS_Get_Quaternion_Error_Vector(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Quaternion_Error_Vector_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_estimated_gyro_bias(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                          char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Estimated_Gyro_Bias_Struct packed_struct;
    uint8_t status = ADCS_Get_Estimated_Gyro_Bias(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Estimated_Gyro_Bias_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_estimation_innovation_vector(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                   char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Estimation_Innovation_Vector_Struct packed_struct;
    uint8_t status = ADCS_Get_Estimation_Innovation_Vector(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Estimation_Innovation_Vector_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                                        

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_raw_cam1_sensor(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Raw_Cam_Sensor_Struct packed_struct;
    uint8_t status = ADCS_Get_Raw_Cam1_Sensor(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Raw_Cam_Sensor_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_raw_cam2_sensor(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Raw_Cam_Sensor_Struct packed_struct;
    uint8_t status = ADCS_Get_Raw_Cam2_Sensor(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Raw_Cam_Sensor_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_raw_css_1_to_6(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Raw_CSS_1_to_6_Struct packed_struct;
    uint8_t status = ADCS_Get_Raw_CSS_1_to_6(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Raw_CSS_1_to_6_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_raw_css_7_to_10(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Raw_CSS_7_to_10_Struct packed_struct;
    uint8_t status = ADCS_Get_Raw_CSS_7_to_10(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Raw_CSS_7_to_10_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_cubecontrol_current(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                          char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_CubeControl_Current_Struct packed_struct;
    uint8_t status = ADCS_Get_CubeControl_Current(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_CubeControl_Current_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_raw_gps_status(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Raw_GPS_Status_Struct packed_struct;
    uint8_t status = ADCS_Get_Raw_GPS_Status(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Raw_GPS_Status_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_raw_gps_time(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Raw_GPS_Time_Struct packed_struct;
    uint8_t status = ADCS_Get_Raw_GPS_Time(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Raw_GPS_Time_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                        

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_raw_gps_x(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Raw_GPS_Struct packed_struct;
    uint8_t status = ADCS_Get_Raw_GPS_X(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Raw_GPS_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_raw_gps_y(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Raw_GPS_Struct packed_struct;
    uint8_t status = ADCS_Get_Raw_GPS_Y(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Raw_GPS_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_raw_gps_z(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Raw_GPS_Struct packed_struct;
    uint8_t status = ADCS_Get_Raw_GPS_Z(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Raw_GPS_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_measurements(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_Measurements_Struct packed_struct;
    uint8_t status = ADCS_Get_Measurements(&packed_struct); 
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_Measurements_Struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}                        

// TODO: create TCMEXEC_adcs_generic_command and TCMEXEC_adcs_generic_telemetry functions in order to execute unlisted commands from firmware manual
    // adcs_generic_command: parameters are command ID, number of data bytes to send, then the bytes themselves
        // this is easy! Just call telecommand_wrapper() with the inputs (which should already be extracted to uint8s as hex array)!
        // confirm ID is a command, then TCMD_extract_hex_array_arg, then telecommand_wrapper()
    // adcs_generic_telemetry: parameter is command ID, receive number of data bytes equal to the largest telemetry request
        // longest telemetry request is almost certainly Get ADCS Configuration (ID 204) at 504 bytes
        // which means we need a TO_json function for this one

/// @brief Telecommand: execute a generic command on the ADCS
/// @param args_str 
///     - Arg 0: ID of the telecommand to send (see Firmware Reference Manual)
///     - Arg 1: number of data bytes to send to the ADCS (up to 504)
///     - Arg 2: hex array of data bytes of length given in Arg 1
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_generic_command(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    // parse command ID argument: first into uint64_t, then convert to correct form for input
    uint64_t command_id;
    TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &command_id);

    if (command_id > 255) {
        snprintf(response_output_buf, response_output_buf_len,
            "Invalid ADCS command or telemetry request ID (err 4)");
        return 4; // invalid ID
    } else if (command_id > 127) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request ID received, not command ID (err 3)");
        return 3; // command_id is a telemetry request, not a command
    }
    
    // parse hex array arguments
    uint8_t hex_data_array[504]; 
    uint16_t data_length;
    TCMD_extract_hex_array_arg(args_str, 2, &hex_data_array[0], data_length, &data_length);
        // TODO: check whether this actually works this way
    
    uint8_t status = ADCS_I2C_telecommand_wrapper((uint8_t) command_id, &hex_data_array[0], (uint32_t) data_length, ADCS_INCLUDE_CHECKSUM);
    return status;
}

/// @brief Telecommand: obtain generic telemetry from the ADCS
/// @param args_str 
///     - Arg 0: ID of the telemetry request to send (see Firmware Reference Manual)
///     - Arg 1: number of data bytes expected to receive from the ADCS (also see Firmware Reference Manual)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_generic_telemetry_request(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    // TODO: check what the ADCS does if data_length is incorrect
    
    // parse telemetry request ID argument: first into uint64_t
    uint64_t telemetry_request_id;
    TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &telemetry_request_id);

    if (telemetry_request_id < 128) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS command ID received, not telemetry request ID (err 3)");
        return 3;
    } else if (telemetry_request_id > 255) {
        snprintf(response_output_buf, response_output_buf_len,
            "Invalid ADCS command or telemetry request ID (err 4)");
        return 4; // invalid ID
    }

    // parse data length argument: first into uint64_t
    uint64_t data_length;
    TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &data_length);

    uint8_t data_received[data_length];
    
    uint8_t status = ADCS_I2C_telemetry_wrapper((uint8_t) telemetry_request_id, &data_received[0], (uint8_t) data_length, ADCS_INCLUDE_CHECKSUM); 
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_generic_telemetry_uint8_array_TO_json(
        &data_received[0], data_length, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}