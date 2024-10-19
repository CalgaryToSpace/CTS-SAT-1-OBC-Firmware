#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_args_helpers.h"
#include "transforms/arrays.h"
#include "unit_tests/unit_test_executor.h"
#include "timekeeping/timekeeping.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include "adcs_drivers/adcs_types.h"
#include "adcs_drivers/adcs_commands.h"
#include "adcs_drivers/adcs_struct_packers.h"
#include "adcs_drivers/adcs_types_to_json.h"

/// @brief Telecommand: execute a generic command on the ADCS
/// @param args_str 
///     - Arg 0: ID of the telecommand to send (see Firmware Reference Manual)
///     - Arg 1: hex array of data bytes of length up to 504 (longest command is almost ADCS Configuration (ID 26/204) at 504 bytes)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_generic_command(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    // parse command ID argument: first into uint64_t, then convert to correct form for input
    uint64_t command_id;
    TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &command_id);

    if (command_id > 255) {
        snprintf(response_output_buf, response_output_buf_len,
            "Invalid ADCS command or telemetry request ID (err 6)");
        return 6; // invalid ID
    } else if (command_id > 127) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request ID received, not command ID (err 5)");
        return 5; // command_id is a telemetry request, not a command
    }
    
    // parse hex array arguments
    uint8_t hex_data_array[504]; 
    uint16_t data_length;
    TCMD_extract_hex_array_arg(args_str, 1, &hex_data_array[0], data_length, &data_length);
    
    uint8_t status = ADCS_i2c_send_command_and_check((uint8_t) command_id, &hex_data_array[0], (uint32_t) data_length, ADCS_INCLUDE_CHECKSUM);
    return status;
}

/// @brief Telecommand: obtain generic telemetry from the ADCS
/// @param args_str 
///     - Arg 0: ID of the telemetry request to send (see Firmware Reference Manual)
///     - Arg 1: number of data bytes expected to receive from the ADCS (also see Firmware Reference Manual, up to 504)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_generic_telemetry_request(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    // parse telemetry request ID argument: first into uint64_t
    uint64_t telemetry_request_id;
    TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &telemetry_request_id);

    if (telemetry_request_id < 128) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS command ID received, not telemetry request ID (err 5)");
        return 5;
    } else if (telemetry_request_id > 255) {
        snprintf(response_output_buf, response_output_buf_len,
            "Invalid ADCS command or telemetry request ID (err 6)");
        return 6; // invalid ID
    }

    // parse data length argument: first into uint64_t
    uint64_t data_length;
    TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &data_length);

    uint8_t data_received[data_length];
    // if data_length is longer then it should be for a given command, 
    // then the byte after the command bytes end is the checksum byte
    // and the rest should be all zeroes.
    
    uint8_t status = ADCS_i2c_request_telemetry_and_check((uint8_t) telemetry_request_id, &data_received[0], (uint32_t) data_length, ADCS_INCLUDE_CHECKSUM); 

    // there's no built in method to get an error for this, so check communications status for telemetry error bit
    // if data_length is too short, the checksum will in almost all cases fail before this point is reached
    // but if it by chance succeeds, this will catch the error
    ADCS_comms_status_struct_t comms_status;
    uint8_t comms_status_status = ADCS_get_communication_status(&comms_status);

    if (comms_status_status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS failed to check communications status telemetry (err %d)", comms_status_status);
        return 1;
    } else if (comms_status.i2c_tlm_error) {
        // if number of data clocked out by ADCS was more than telemetry package, we need to reset the error flags then return an error
        uint8_t clear_error_status = ADCS_clear_errors();
        snprintf(response_output_buf, response_output_buf_len,
            "Insufficient number of bits receieved for generic ADCS telemetry request (err %d)", comms_status.i2c_tlm_error);
        if (clear_error_status != 0) {
                snprintf(response_output_buf, response_output_buf_len,
                    "ADCS failed to send ADCS_Clear_Status command (err %d)", clear_error_status);
            }
        return 3;
    }

    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    uint8_t result_json = ADCS_generic_telemetry_uint8_array_TO_json(
        &data_received[0], data_length, response_output_buf, response_output_buf_len);

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
uint8_t TCMDEXEC_adcs_ack(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_cmd_ack_struct_t ack;
    uint8_t status = ADCS_cmd_ack(&ack);

    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_cmd_ack_struct_TO_json(
        &ack, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;

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
    
    uint8_t status = ADCS_set_wheel_speed(args_16[0], args_16[1], args_16[2]); 
    return status;
}

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_reset(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                            char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_reset(); 
    return status;
}                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_identification(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_id_struct_t packed_struct;
    uint8_t status = ADCS_get_identification(&packed_struct); 
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_id_struct_TO_json(
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
    ADCS_boot_running_status_struct_t packed_struct;
    uint8_t status = ADCS_get_program_status(&packed_struct); 
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_boot_running_status_struct_TO_json(
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
    ADCS_comms_status_struct_t packed_struct;
    uint8_t status = ADCS_get_communication_status(&packed_struct); 
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_comms_status_struct_TO_json(
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
///     - Arg 0: timeout for deployment [seconds]
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_deploy_magnetometer(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                          char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t timeout;
    TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &timeout);
    uint8_t status = ADCS_deploy_magnetometer((uint8_t) timeout);
    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - Arg 0: run mode to set; can be can be off (0), enabled (1), triggered (2), or simulation (3)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_run_mode(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t run_mode;
    TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &run_mode);
    uint8_t status = ADCS_set_run_mode((ADCS_run_mode_enum_t) run_mode); 
    return status;
}                        

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_clear_errors(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_clear_errors();
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
    uint8_t status = ADCS_attitude_control_mode((ADCS_control_mode_enum_t) arguments[0], (uint16_t) arguments[1]);
    return status;
}                                                         

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - Arg 0: Attitude estimation mode to set (Table 79 in Firmware Manual)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_attitude_estimation_mode(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                               char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t estimation_mode;
    TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &estimation_mode);
    uint8_t status = ADCS_attitude_estimation_mode((ADCS_estimation_mode_enum_t) estimation_mode); 
    return status;
}                                    

/// @brief Telecommand: If ADCS run mode is Triggered, run the ADCS sensor loop
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_run_once(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                               char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_run_once();
    return status;
}                    

/// @brief Telecommand: Set the magnetometer mode of the ADCS
/// @param args_str 
///     - Arg 0: magnetometer mode to set
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_magnetometer_mode(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t mode;
    TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &mode);
    uint8_t status = ADCS_set_magnetometer_mode((ADCS_magnetometer_mode_enum_t) mode);
    return status;
}                                

/// @brief Telecommand: Set the magnetorquer output values
/// @param args_str 
///     - Arg 0: magnetorquer x duty cycle (double)
///     - Arg 1: magnetorquer y duty cycle (double)
///     - Arg 2: magnetorquer z duty cycle (double)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_magnetorquer_output(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
     // parse arguments into doubles
    const uint8_t num_args = 3;
    double arguments[num_args]; 
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_double_arg(args_str, strlen(args_str), i, &arguments[i]);
    }
    
    uint8_t status = ADCS_set_magnetorquer_output(arguments[0], arguments[1], arguments[2]);
    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_save_config(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                  char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_save_config();
    return status;
}                        

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_estimate_angular_rates(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                             char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_angular_rates_struct_t packed_struct;
    uint8_t status = ADCS_get_estimate_angular_rates(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_angular_rates_struct_TO_json(
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
    ADCS_llh_position_struct_t packed_struct;
    uint8_t status = ADCS_get_llh_position(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_llh_position_struct_TO_json(
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
    ADCS_power_control_struct_t packed_struct;
    uint8_t status = ADCS_get_power_control(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_power_control_struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}   

/// @brief Telecommand: Set the power control mode of each component of the ADCS; for each, 0 turns the component off, 1 turns it on, and 2 keeps it the same as previously.
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
    
    uint8_t status = ADCS_set_power_control((ADCS_power_select_enum_t) args_8[0], (ADCS_power_select_enum_t) args_8[1], (ADCS_power_select_enum_t) args_8[2], (ADCS_power_select_enum_t) args_8[3], (ADCS_power_select_enum_t) args_8[4], (ADCS_power_select_enum_t) args_8[5], (ADCS_power_select_enum_t) args_8[6], (ADCS_power_select_enum_t) args_8[7], (ADCS_power_select_enum_t) args_8[8], (ADCS_power_select_enum_t) args_8[9]);
    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - Arg 0: Mounting transform alpha angle [deg] (double) 
///     - Arg 1: Mounting transform beta angle [deg] (double)
///     - Arg 2: Mounting transform gamma angle [deg] (double)
///     - Arg 3: Channel 1 offset value (double)
///     - Arg 4: Channel 2 offset value (double)
///     - Arg 5: Channel 3 offset value (double)
///     - Arg 6: Value (1, 1) of the magnetometer sensitivity matrix (double)
///     - Arg 7: Value (2, 2) of the magnetometer sensitivity matrix (double)
///     - Arg 8: Value (3, 3) of the magnetometer sensitivity matrix (double)
///     - Arg 9: Value (1, 2) of the magnetometer sensitivity matrix (double)
///     - Arg 10: Value (1, 3) of the magnetometer sensitivity matrix (double)
///     - Arg 11: Value (2, 1) of the magnetometer sensitivity matrix (double)
///     - Arg 12: Value (2, 3) of the magnetometer sensitivity matrix (double)
///     - Arg 13: Value (3, 1) of the magnetometer sensitivity matrix (double)
///     - Arg 14: Value (3, 2) of the magnetometer sensitivity matrix (double)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_magnetometer_config(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {

    // parse arguments into doubles
    const uint8_t num_args = 15;
    double arguments[num_args]; 
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_double_arg(args_str, strlen(args_str), i, &arguments[i]);
    }
    
    uint8_t status = ADCS_set_magnetometer_config(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[11], arguments[12], arguments[13], arguments[14]);
    return status;
}     

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_bootloader_clear_errors(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_bootloader_clear_errors(); 
    return status;
}                                    

/// @brief Telecommand: Choose the circumstances to save the current Unix time
/// @param args_str 
///     - Arg 0: whether to save the current Unix time immediately (bool passed as int; 1 = save immediately, 0 = don't save immediately)
///     - Arg 1: whether to save the current Unix time whenever a command is used to update it (bool passed as int; 1 = save on command, 0 = don't)
///     - Arg 2: whether to save the current Unix time periodically (bool passed as int; 1 = save periodically, 0 = don't)
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

    uint8_t status = ADCS_set_unix_time_save_mode((bool) bools[0], (bool) bools[1], (bool) bools[2], (uint8_t) uint_arg);
    return status;
}                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_unix_time_save_mode(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_set_unix_time_save_mode_struct_t packed_struct;
    uint8_t status = ADCS_get_unix_time_save_mode(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_set_unix_time_save_mode_struct_TO_json(
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
///     - Arg 0: inclination (degrees) (double)
///     - Arg 1: eccentricity (dimensionless) (double)
///     - Arg 2: right ascension of the ascending node (degrees) (double)
///     - Arg 3: argument of perigee (degrees) (double)
///     - Arg 4: b-star drag term (dimensionless) (double)
///     - Arg 5: mean motion (orbits per day) (double)
///     - Arg 6: mean anomaly (degrees) (double)
///     - Arg 7: epoch (integer component is year, decimal component is day) (double)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_sgp4_orbit_params(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    // parse arguments into doubles
    const uint8_t num_args = 8;
    double arguments[num_args]; 
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_double_arg(args_str, strlen(args_str), i, &arguments[i]);
    }

    uint8_t status = ADCS_set_sgp4_orbit_params(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7]);
    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_sgp4_orbit_params(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_orbit_params_struct_t packed_struct;
    uint8_t status = ADCS_get_sgp4_orbit_params(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_orbit_params_struct_TO_json(
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
    uint8_t status = ADCS_save_orbit_params();
    return status;
}                            

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_rate_sensor_rates(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                        char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_rated_sensor_rates_struct_t packed_struct;
    uint8_t status = ADCS_get_rate_sensor_rates(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_rated_sensor_rates_struct_TO_json(
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
    ADCS_wheel_speed_struct_t packed_struct;
    uint8_t status = ADCS_get_wheel_speed(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_wheel_speed_struct_TO_json(
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
    ADCS_magnetorquer_command_struct_t packed_struct;
    uint8_t status = ADCS_get_magnetorquer_command(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_magnetorquer_command_struct_TO_json(
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
    ADCS_raw_magnetometer_values_struct_t packed_struct;
    uint8_t status = ADCS_get_raw_magnetometer_values(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_raw_magnetometer_values_struct_TO_json(
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
    ADCS_fine_angular_rates_struct_t packed_struct;
    uint8_t status = ADCS_get_estimate_fine_angular_rates(&packed_struct); 
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_fine_angular_rates_struct_TO_json(
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
    ADCS_magnetometer_config_struct_t packed_struct;
    uint8_t status = ADCS_get_magnetometer_config(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_magnetometer_config_struct_TO_json(
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
    ADCS_commanded_angles_struct_t packed_struct;
    uint8_t status = ADCS_get_commanded_attitude_angles(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_commanded_angles_struct_TO_json(
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
///     - Arg 0: x attitude angle (double)
///     - Arg 1: y attitude angle (double)
///     - Arg 2: z attitude angle (double)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_commanded_attitude_angles(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                    char *response_output_buf, uint16_t response_output_buf_len) {
    // parse arguments into doubles
    const uint8_t num_args = 3;
    double arguments[num_args]; 
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_double_arg(args_str, strlen(args_str), i, &arguments[i]);
    }

    uint8_t status = ADCS_set_commanded_attitude_angles(arguments[0], arguments[1], arguments[2]); 
    return status;
}          

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - Arg 0: magnetometer_rate_filter_system_noise (float; magnetometer rate filter system noise covariance parameter)
///     - Arg 1: extended_kalman_filter_system_noise (float; extended kalman filter system noise covariance parameter)
///     - Arg 2: coarse_sun_sensor_measurement_noise (float; CSS measurement noise covariance parameter)
///     - Arg 3: sun_sensor_measurement_noise (float; sun sensor measurement noise covariance parameter)
///     - Arg 4: nadir_sensor_measurement_noise (float; nadir sensor measurement noise covariance parameter)
///     - Arg 5: magnetometer_measurement_noise (float; magnetometer measurement noise covariance parameter)
///     - Arg 6: star_tracker_measurement_noise (float; star tracker measurement noise covariance parameter)
///     - Arg 7: use_sun_sensor (bool; whether or not to use the sun sensor measurement in extended_kalman_filter)
///     - Arg 8: use_nadir_sensor (bool; whether or not to use the nadir sensor measurement in extended_kalman_filter)
///     - Arg 9: use_css (bool; whether or not to use the CSS measurement in extended_kalman_filter)
///     - Arg 10: use_star_tracker (bool; whether or not to use the star tracker measurement in extended_kalman_filter)
///     - Arg 11: nadir_sensor_terminator_test (bool; select to ignore nadir sensor measurements when terminator is in FOV)
///     - Arg 12: automatic_magnetometer_recovery (bool; select whether automatic switch to redundant magnetometer should occur in case of  {failure)
///     - Arg 13: magnetometer_mode (enum; select magnetometer mode for estimation and control)
///     - Arg 14: magnetometer_selection_for_raw_magnetometer_telemetry (enum; select magnetometer mode for the second raw telemetry frame)
///     - Arg 15: automatic_estimation_transition_due_to_rate_sensor_errors (bool; enable/disable automatic transition from MEMS rate estimation mode to RKF in case of  {rate sensor error)
///     - Arg 16: wheel_30s_power_up_delay (bool; present in CubeSupport but not in the manual -- need to test)
///     - Arg 17: cam1_and_cam2_sampling_period (uint8; the manual calls it this, but CubeSupport calls it "error counter reset period" -- need to test)
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
    ADCS_magnetometer_mode_enum_t enum_args[2];
    uint8_t uint8_arg;

    for (uint8_t i = 0; i < new_num_args; i++) {
        TCMD_extract_uint64_arg(args_str, strlen(args_str), i + 7, &uint_type_arguments[i]);
        if (i < 6 || i == 8 || i == 9) {
            bool_args[i] = (bool) uint_type_arguments[i];
        }
    }
    enum_args[0] = (ADCS_magnetometer_mode_enum_t) uint_type_arguments[6];
    enum_args[1] = (ADCS_magnetometer_mode_enum_t) uint_type_arguments[7];
    uint8_arg = (uint8_t) uint_type_arguments[10];
    
    uint8_t status = ADCS_set_estimation_params(float_args[0], float_args[1], float_args[2], float_args[3], float_args[4], float_args[5], float_args[6],
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
    ADCS_estimation_params_struct_t packed_struct;
    uint8_t status = ADCS_get_estimation_params(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_estimation_params_struct_TO_json(
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
///     - Arg 0: incl_coefficient (set inclination filter coefficient) (double)
///     - Arg 1: raan_coefficient (set RAAN filter coefficient) (double)
///     - Arg 2: ecc_coefficient (set eccentricity filter coefficient) (double)
///     - Arg 3: aop_coefficient (set argument of perigee filter coefficient) (double)
///     - Arg 4: time_coefficient (set time filter coefficient) (double)
///     - Arg 5: pos_coefficient (set position filter coefficient) (double)
///     - Arg 6: maximum_position_error (maximum position error for Augmented_SGP4 to continue working) (double)
///     - Arg 7: augmented_sgp4_filter (The type of filter being used (enum))
///     - Arg 8: xp_coefficient (polar coefficient xdouble; p) (double)
///     - Arg 9: yp_coefficient (polar coefficient ydouble; p) (double)
///     - Arg 10: gps_roll_over (GPS roll over number)
///     - Arg 11: position_sd (maximum position standard deviation for Augmented_SGP4 to continue working) (double)
///     - Arg 12: velocity_sd (maximum velocity standard deviation for Augmented_SGP4 to continue working) (double)
///     - Arg 13: min_satellites (Minimum satellites required for Augmented_SGP4 to continue working)
///     - Arg 14: time_gain (time offset compensation gain) (double)
///     - Arg 15: max_lag (maximum lagged timestamp measurements to incorporate) (double)
///     - Arg 16: min_samples (Minimum samples to use to get Augmented_SGP4)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_augmented_sgp4_params(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
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

    uint8_t status = ADCS_set_augmented_sgp4_params(doubles_params[0], doubles_params[1], doubles_params[2], doubles_params[3], doubles_params[4], doubles_params[5], doubles_params[6],
                                        (ADCS_augmented_sgp4_filter_enum_t) uint_params[0],
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
uint8_t TCMDEXEC_adcs_get_augmented_sgp4_params(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                       char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_augmented_sgp4_params_struct_t packed_struct;
    uint8_t status = ADCS_get_augmented_sgp4_params(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_augmented_sgp4_params_struct_TO_json(
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
///     - Arg 0: longitude (double)
///     - Arg 1: latitude (double)
///     - Arg 2: altitude (double)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_tracking_controller_target_reference(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                                char *response_output_buf, uint16_t response_output_buf_len) {
    // parse arguments into doubles
    const uint8_t num_args = 3;
    double arguments[num_args]; 
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_double_arg(args_str, strlen(args_str), i, &arguments[i]);
    }

    uint8_t status = ADCS_set_tracking_controller_target_reference((float) arguments[0], (float) arguments[1], (float) arguments[2]); 
    
    return status;
}                                                    

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_tracking_controller_target_reference(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                                char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_tracking_controller_target_struct_t packed_struct;
    uint8_t status = ADCS_get_tracking_controller_target_reference(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_tracking_controller_target_struct_TO_json(
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
///     - Arg 3: x_rate_offset (x-rate sensor offset) (double)
///     - Arg 4: y_rate_offset (y-rate sensor offset) (double)
///     - Arg 5: z_rate_offset (z-rate sensor offset) (double)
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
    
    uint8_t status = ADCS_set_rate_gyro_config((ADCS_axis_select_enum_t) axis_arguments[0], (ADCS_axis_select_enum_t) axis_arguments[1], (ADCS_axis_select_enum_t) axis_arguments[2],  offset_arguments[0], offset_arguments[1], offset_arguments[2], (uint8_t) rate_sensor_mult); 
    return status;
}                                

/// @brief Telecommand: Request the given telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_rate_gyro_config(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                           char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_rate_gyro_config_struct_t packed_struct;
    uint8_t status = ADCS_get_rate_gyro_config(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_rate_gyro_config_struct_TO_json(
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
    ADCS_estimated_attitude_angles_struct_t packed_struct;
    uint8_t status = ADCS_get_estimated_attitude_angles(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_estimated_attitude_angles_struct_TO_json(
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
    ADCS_magnetic_field_vector_struct_t packed_struct;
    uint8_t status = ADCS_get_magnetic_field_vector(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_magnetic_field_vector_struct_TO_json(
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
    ADCS_fine_sun_vector_struct_t packed_struct;
    uint8_t status = ADCS_get_fine_sun_vector(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_fine_sun_vector_struct_TO_json(
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
    ADCS_nadir_vector_struct_t packed_struct;
    uint8_t status = ADCS_get_nadir_vector(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_nadir_vector_struct_TO_json(
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
    ADCS_wheel_speed_struct_t packed_struct;
    uint8_t status = ADCS_get_commanded_wheel_speed(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_wheel_speed_struct_TO_json(
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
    ADCS_magnetic_field_vector_struct_t packed_struct;
    uint8_t status = ADCS_get_igrf_magnetic_field_vector(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_magnetic_field_vector_struct_TO_json(
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
    ADCS_quaternion_error_vector_struct_t packed_struct;
    uint8_t status = ADCS_get_quaternion_error_vector(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_quaternion_error_vector_struct_TO_json(
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
    ADCS_estimated_gyro_bias_struct_t packed_struct;
    uint8_t status = ADCS_get_estimated_gyro_bias(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_estimated_gyro_bias_struct_TO_json(
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
    ADCS_estimation_innovation_vector_struct_t packed_struct;
    uint8_t status = ADCS_get_estimation_innovation_vector(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_estimation_innovation_vector_struct_TO_json(
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
    ADCS_raw_cam_sensor_struct_t packed_struct;
    uint8_t status = ADCS_get_raw_cam1_sensor(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_raw_cam_sensor_struct_TO_json(
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
    ADCS_raw_cam_sensor_struct_t packed_struct;
    uint8_t status = ADCS_get_raw_cam2_sensor(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_raw_cam_sensor_struct_TO_json(
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
uint8_t TCMDEXEC_adcs_raw_coarse_sun_sensor_1_to_6(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_raw_coarse_sun_sensor_1_to_6_struct_t packed_struct;
    uint8_t status = ADCS_get_raw_coarse_sun_sensor_1_to_6(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_raw_coarse_sun_sensor_1_to_6_struct_TO_json(
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
uint8_t TCMDEXEC_adcs_raw_coarse_sun_sensor_7_to_10(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_raw_coarse_sun_sensor_7_to_10_struct_t packed_struct;
    uint8_t status = ADCS_get_raw_coarse_sun_sensor_7_to_10(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_raw_coarse_sun_sensor_7_to_10_struct_TO_json(
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
    ADCS_cubecontrol_current_struct_t packed_struct;
    uint8_t status = ADCS_get_cubecontrol_current(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_cubecontrol_current_struct_TO_json(
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
    ADCS_raw_gps_status_struct_t packed_struct;
    uint8_t status = ADCS_get_raw_gps_status(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_raw_gps_status_struct_TO_json(
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
    ADCS_raw_gps_time_struct_t packed_struct;
    uint8_t status = ADCS_get_raw_gps_time(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_raw_gps_time_struct_TO_json(
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
    ADCS_raw_gps_struct_t packed_struct;
    uint8_t status = ADCS_get_raw_gps_x(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_raw_gps_struct_TO_json(
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
    ADCS_raw_gps_struct_t packed_struct;
    uint8_t status = ADCS_get_raw_gps_y(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_raw_gps_struct_TO_json(
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
    ADCS_raw_gps_struct_t packed_struct;
    uint8_t status = ADCS_get_raw_gps_z(&packed_struct);
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_raw_gps_struct_TO_json(
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
    ADCS_measurements_struct_t packed_struct;
    uint8_t status = ADCS_get_measurements(&packed_struct); 
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_measurements_struct_TO_json(
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
uint8_t TCMDEXEC_adcs_acp_execution_state(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_acp_execution_state_struct_t packed_struct;
    uint8_t status = ADCS_get_acp_execution_state(&packed_struct); 
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_acp_execution_struct_TO_json(
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
uint8_t TCMDEXEC_adcs_get_current_state_1(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_current_state_1_struct_t packed_struct;
    uint8_t status = ADCS_get_current_state_1(&packed_struct); 
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_current_state_1_struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS telemetry request JSON failed (err %d)", result_json);
        return 2;
    }

    return status;
}

/// @brief Telecommand: Request raw star tracker telemetry data from the ADCS
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_raw_star_tracker_data(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    ADCS_raw_star_tracker_struct_t packed_struct;
    uint8_t status = ADCS_get_raw_star_tracker_data(&packed_struct); 
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS raw star tracker telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_raw_star_tracker_struct_TO_json(
        &packed_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS raw star tracker telemetry JSON conversion failed (err %d)", result_json);
        return 2;
    }

    return status;
}

/// @brief Telecommand: Save an image to the ADCS onboard SD card
/// @param args_str 
///     - Arg 0: Which camera to save the image from; can be Camera 1 (0), Camera 2 (1), or Star (2)
///     - Arg 1: Resolution of the image to save; can be 1024x1024 (0), 512x512, (1) 256x256, (2) 128x128, (3) or 64x64 (4)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_save_image_to_sd(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    // parse arguments: first into int64_t, then convert to correct form for input
    const uint8_t num_args = 2;
    uint64_t arguments[num_args]; 
    uint8_t args_8[num_args];
    for (uint8_t i = 0; i < num_args; i++) {
        TCMD_extract_uint64_arg(args_str, strlen(args_str), i, &arguments[i]);
        args_8[i] = (uint8_t) arguments[i];
    }
    
    uint8_t status = ADCS_save_image_to_sd(args_8[0], args_8[1]); 
    return status;
}

// TODO: agenda modification for repeating
/// @brief Telecommand: Request commissioning telemetry from the ADCS and save it to the memory module
/// @param args_str 
///     - Arg 0: Which commissioning step to request telemetry for (1-18)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_request_commissioning_telemetry(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    // parse arguments: first into int64_t, then convert to correct form for input
    uint64_t argument;
    TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &argument);
    ADCS_commissioning_step_enum_t commissioning_step = (ADCS_commissioning_step_enum_t) argument;

    ADCS_acp_execution_state_struct_t current_state;
    do { // wait until 500ms has passed since last update
        uint8_t state_status = ADCS_get_acp_execution_state(&current_state);
        if (state_status) {
            snprintf(response_output_buf, response_output_buf_len,
                "ACP execution state telemetry request failed (err %d)", state_status);
            return 1;
        }
    } while (current_state.time_since_iteration_start_ms <= 500);

    uint8_t status = 0;

    #define MEMORY_MODULE_FUNCTION(x) ; // TODO: delete this once memory module is implemented

    switch(commissioning_step) {
        case ADCS_COMMISISONING_STEP_DETERMINE_INITIAL_ANGULAR_RATES: {
            ADCS_commissioning_determine_initial_angular_rates_struct_t commissioning_data;
            commissioning_data.current_unix_time = TIM_get_current_unix_epoch_time_ms();
            status += ADCS_get_estimate_angular_rates(&commissioning_data.estimated_angular_rates);
            status += ADCS_get_rate_sensor_rates(&commissioning_data.rated_sensor_rates);
            status += ADCS_get_raw_magnetometer_values(&commissioning_data.raw_magnetometer_measurements);
            MEMORY_MODULE_FUNCTION(commissioning_data); // save to memory module
            break;
        }
        case ADCS_COMMISISONING_STEP_INITIAL_DETUMBLING: {
            ADCS_commissioning_initial_detumbling_struct_t commissioning_data;
            commissioning_data.current_unix_time = TIM_get_current_unix_epoch_time_ms();
            status += ADCS_get_estimate_angular_rates(&commissioning_data.estimated_angular_rates);
            status += ADCS_get_rate_sensor_rates(&commissioning_data.rated_sensor_rates);
            status += ADCS_get_raw_magnetometer_values(&commissioning_data.raw_magnetometer_measurements);
            status += ADCS_get_magnetorquer_command(&commissioning_data.magnetorquer_commands);
            MEMORY_MODULE_FUNCTION(commissioning_data); // save to memory module
            break;
        }
        case ADCS_COMMISISONING_STEP_CONTINUED_DETUMBLING_TO_Y_THOMSON: {
            ADCS_commissioning_initial_detumbling_struct_t commissioning_data;
            commissioning_data.current_unix_time = TIM_get_current_unix_epoch_time_ms();
            status += ADCS_get_estimate_angular_rates(&commissioning_data.estimated_angular_rates);
            status += ADCS_get_rate_sensor_rates(&commissioning_data.rated_sensor_rates);
            status += ADCS_get_raw_magnetometer_values(&commissioning_data.raw_magnetometer_measurements);
            status += ADCS_get_magnetorquer_command(&commissioning_data.magnetorquer_commands);
            MEMORY_MODULE_FUNCTION(commissioning_data); // save to memory module
            break;
        }
        case ADCS_COMMISISONING_STEP_MAGNETOMETER_DEPLOYMENT: {
            ADCS_commissioning_magnetometer_deployment_struct_t commissioning_data;
            commissioning_data.current_unix_time = TIM_get_current_unix_epoch_time_ms();
            status += ADCS_get_estimate_fine_angular_rates(&commissioning_data.fine_estimated_angular_rates);
            status += ADCS_get_rate_sensor_rates(&commissioning_data.rated_sensor_rates);
            status += ADCS_get_raw_magnetometer_values(&commissioning_data.raw_magnetometer_measurements);
            status += ADCS_get_cubecontrol_current(&commissioning_data.cubecontrol_currents);
            MEMORY_MODULE_FUNCTION(commissioning_data); // save to memory module
            break;
        }
        case ADCS_COMMISISONING_STEP_MAGNETOMETER_CALIBRATION: {
            ADCS_commissioning_magnetometer_calibration_struct_t commissioning_data;
            commissioning_data.current_unix_time = TIM_get_current_unix_epoch_time_ms();
            status += ADCS_get_estimate_fine_angular_rates(&commissioning_data.fine_estimated_angular_rates);
            status += ADCS_get_rate_sensor_rates(&commissioning_data.rated_sensor_rates);
            status += ADCS_get_measurements(&commissioning_data.adcs_measurements);
            MEMORY_MODULE_FUNCTION(commissioning_data); // save to memory module
            break;
        }
        case ADCS_COMMISISONING_STEP_ANGULAR_RATE_AND_PITCH_ANGLE_ESTIMATION: {
            ADCS_commissioning_angular_rate_and_pitch_angle_estimation_struct_t commissioning_data;
            commissioning_data.current_unix_time = TIM_get_current_unix_epoch_time_ms();
            status += ADCS_get_estimate_fine_angular_rates(&commissioning_data.fine_estimated_angular_rates);
            status += ADCS_get_estimated_attitude_angles(&commissioning_data.estimated_attitude_angles);
            status += ADCS_get_rate_sensor_rates(&commissioning_data.rated_sensor_rates);
            status += ADCS_get_measurements(&commissioning_data.adcs_measurements);
            MEMORY_MODULE_FUNCTION(commissioning_data); // save to memory module
            break;
        }
        case ADCS_COMMISISONING_STEP_Y_WHEEL_RAMP_UP_TEST: {
            ADCS_commissioning_y_wheel_ramp_up_test_struct_t commissioning_data;
            commissioning_data.current_unix_time = TIM_get_current_unix_epoch_time_ms();
            status += ADCS_get_estimate_fine_angular_rates(&commissioning_data.fine_estimated_angular_rates);
            status += ADCS_get_estimated_attitude_angles(&commissioning_data.estimated_attitude_angles);
            status += ADCS_get_rate_sensor_rates(&commissioning_data.rated_sensor_rates);
            status += ADCS_get_wheel_speed(&commissioning_data.measured_wheel_speeds);
            status += ADCS_get_measurements(&commissioning_data.adcs_measurements);            
            MEMORY_MODULE_FUNCTION(commissioning_data); // save to memory module
            break;
        }
        case ADCS_COMMISISONING_STEP_INITIAL_Y_MOMENTUM_ACTIVATION: {
            ADCS_commissioning_y_momentum_activation_struct_t commissioning_data;
            commissioning_data.current_unix_time = TIM_get_current_unix_epoch_time_ms();
            status += ADCS_get_estimate_fine_angular_rates(&commissioning_data.fine_estimated_angular_rates);
            status += ADCS_get_estimated_attitude_angles(&commissioning_data.estimated_attitude_angles);
            status += ADCS_get_rate_sensor_rates(&commissioning_data.rated_sensor_rates);
            status += ADCS_get_wheel_speed(&commissioning_data.measured_wheel_speeds);
            status += ADCS_get_measurements(&commissioning_data.adcs_measurements);       
            status += ADCS_get_llh_position(&commissioning_data.LLH_positions);
            MEMORY_MODULE_FUNCTION(commissioning_data); // save to memory module
            break;
        }
        case ADCS_COMMISISONING_STEP_CONTINUED_Y_MOMENTUM_ACTIVATION_AND_MAGNETOMETER_EKF: {
            ADCS_commissioning_y_momentum_activation_struct_t commissioning_data;
            commissioning_data.current_unix_time = TIM_get_current_unix_epoch_time_ms();
            status += ADCS_get_estimate_fine_angular_rates(&commissioning_data.fine_estimated_angular_rates);
            status += ADCS_get_estimated_attitude_angles(&commissioning_data.estimated_attitude_angles);
            status += ADCS_get_rate_sensor_rates(&commissioning_data.rated_sensor_rates);
            status += ADCS_get_wheel_speed(&commissioning_data.measured_wheel_speeds);
            status += ADCS_get_measurements(&commissioning_data.adcs_measurements);       
            status += ADCS_get_llh_position(&commissioning_data.LLH_positions);
            MEMORY_MODULE_FUNCTION(commissioning_data); // save to memory module
            break;
        }
        case ADCS_COMMISISONING_STEP_CUBESENSE_SUN_NADIR: {
            ADCS_commissioning_cubesense_sun_nadir_commissioning_struct_t commissioning_data;
            commissioning_data.current_unix_time = TIM_get_current_unix_epoch_time_ms();
            status += ADCS_get_estimate_fine_angular_rates(&commissioning_data.fine_estimated_angular_rates);
            status += ADCS_get_estimated_attitude_angles(&commissioning_data.estimated_attitude_angles);
            status += ADCS_get_rate_sensor_rates(&commissioning_data.rated_sensor_rates);
            status += ADCS_get_raw_coarse_sun_sensor_1_to_6(&commissioning_data.raw_css_1_to_6_measurements); 
            status += ADCS_get_raw_coarse_sun_sensor_7_to_10(&commissioning_data.raw_css_7_to_10_measurements); 
            status += ADCS_get_raw_cam1_sensor(&commissioning_data.raw_cam1_measurements);            
            status += ADCS_get_raw_cam2_sensor(&commissioning_data.raw_cam2_measurements);
            status += ADCS_get_fine_sun_vector(&commissioning_data.fine_sun_vector);
            status += ADCS_get_nadir_vector(&commissioning_data.nadir_vector);
            MEMORY_MODULE_FUNCTION(commissioning_data); // save to memory module
            break;
        }
        case ADCS_COMMISISONING_STEP_EKF_ACTIVATION_SUN_AND_NADIR: {
            ADCS_commissioning_cubesense_sun_nadir_commissioning_struct_t commissioning_data;
            commissioning_data.current_unix_time = TIM_get_current_unix_epoch_time_ms();
            status += ADCS_get_estimate_fine_angular_rates(&commissioning_data.fine_estimated_angular_rates);
            status += ADCS_get_estimated_attitude_angles(&commissioning_data.estimated_attitude_angles);
            status += ADCS_get_rate_sensor_rates(&commissioning_data.rated_sensor_rates);
            status += ADCS_get_raw_coarse_sun_sensor_1_to_6(&commissioning_data.raw_css_1_to_6_measurements); 
            status += ADCS_get_raw_coarse_sun_sensor_7_to_10(&commissioning_data.raw_css_7_to_10_measurements); 
            status += ADCS_get_raw_cam1_sensor(&commissioning_data.raw_cam1_measurements);            
            status += ADCS_get_raw_cam2_sensor(&commissioning_data.raw_cam2_measurements);
            status += ADCS_get_fine_sun_vector(&commissioning_data.fine_sun_vector);
            status += ADCS_get_nadir_vector(&commissioning_data.nadir_vector);
            MEMORY_MODULE_FUNCTION(commissioning_data); // save to memory module
            break;
        }
        case ADCS_COMMISISONING_STEP_CUBESTAR_STAR_TRACKER: {
            ADCS_commissioning_cubestar_star_tracker_commissioning_struct_t commissioning_data;
            commissioning_data.current_unix_time = TIM_get_current_unix_epoch_time_ms();
            status += ADCS_get_estimate_fine_angular_rates(&commissioning_data.fine_estimated_angular_rates);
            status += ADCS_get_estimated_attitude_angles(&commissioning_data.estimated_attitude_angles);
            status += ADCS_get_rate_sensor_rates(&commissioning_data.rated_sensor_rates);
            status += ADCS_get_raw_star_tracker_data(&commissioning_data.raw_star_tracker);
            MEMORY_MODULE_FUNCTION(commissioning_data); // save to memory module
            break;
        }
        case ADCS_COMMISISONING_STEP_EKF_ACTIVATION_WITH_STAR_VECTOR_MEASUREMENTS: {
            ADCS_commissioning_cubestar_star_tracker_commissioning_struct_t commissioning_data;
            commissioning_data.current_unix_time = TIM_get_current_unix_epoch_time_ms();
            status += ADCS_get_estimate_fine_angular_rates(&commissioning_data.fine_estimated_angular_rates);
            status += ADCS_get_estimated_attitude_angles(&commissioning_data.estimated_attitude_angles);
            status += ADCS_get_rate_sensor_rates(&commissioning_data.rated_sensor_rates);
            status += ADCS_get_raw_star_tracker_data(&commissioning_data.raw_star_tracker);
            MEMORY_MODULE_FUNCTION(commissioning_data); // save to memory module
            break;
        }
        case ADCS_COMMISISONING_STEP_ZERO_BIAS_3_AXIS_REACTION_WHEEL_CONTROL: {
            ADCS_commissioning_zero_bias_3_axis_reaction_wheel_control_struct_t commissioning_data;
            commissioning_data.current_unix_time = TIM_get_current_unix_epoch_time_ms();
            status += ADCS_get_estimate_fine_angular_rates(&commissioning_data.fine_estimated_angular_rates);
            status += ADCS_get_estimated_attitude_angles(&commissioning_data.estimated_attitude_angles);
            status += ADCS_get_rate_sensor_rates(&commissioning_data.rated_sensor_rates);
            status += ADCS_get_wheel_speed(&commissioning_data.measured_wheel_speeds);
            MEMORY_MODULE_FUNCTION(commissioning_data); // save to memory module
            break;
        }
        case ADCS_COMMISISONING_STEP_EKF_WITH_RATE_GYRO_STAR_TRACKER_MEASUREMENTS: {
            ADCS_commissioning_sun_tracking_3_axis_control_struct_t commissioning_data;
            commissioning_data.current_unix_time = TIM_get_current_unix_epoch_time_ms();
            status += ADCS_get_estimate_fine_angular_rates(&commissioning_data.fine_estimated_angular_rates);
            status += ADCS_get_estimated_attitude_angles(&commissioning_data.estimated_attitude_angles);
            status += ADCS_get_estimated_gyro_bias(&commissioning_data.estimated_gyro_bias);
            status += ADCS_get_estimation_innovation_vector(&commissioning_data.estimation_innovation_vector);            
            status += ADCS_get_magnetic_field_vector(&commissioning_data.magnetic_field_vector);
            status += ADCS_get_rate_sensor_rates(&commissioning_data.rated_sensor_rates);
            status += ADCS_get_fine_sun_vector(&commissioning_data.fine_sun_vector);
            status += ADCS_get_nadir_vector(&commissioning_data.nadir_vector);
            status += ADCS_get_wheel_speed(&commissioning_data.measured_wheel_speeds);
            status += ADCS_get_magnetorquer_command(&commissioning_data.magnetorquer_commands);
            status += ADCS_get_igrf_magnetic_field_vector(&commissioning_data.igrf_magnetic_field_vector);
            status += ADCS_get_quaternion_error_vector(&commissioning_data.quaternion_error_vector);
            MEMORY_MODULE_FUNCTION(commissioning_data); // save to memory module
            break;
        }
        case ADCS_COMMISISONING_STEP_SUN_TRACKING_3_AXIS_CONTROL: {
            ADCS_commissioning_sun_tracking_3_axis_control_struct_t commissioning_data;
            commissioning_data.current_unix_time = TIM_get_current_unix_epoch_time_ms();
            status += ADCS_get_estimate_fine_angular_rates(&commissioning_data.fine_estimated_angular_rates);
            status += ADCS_get_estimated_attitude_angles(&commissioning_data.estimated_attitude_angles);
            status += ADCS_get_estimated_gyro_bias(&commissioning_data.estimated_gyro_bias);
            status += ADCS_get_estimation_innovation_vector(&commissioning_data.estimation_innovation_vector);            
            status += ADCS_get_magnetic_field_vector(&commissioning_data.magnetic_field_vector);
            status += ADCS_get_rate_sensor_rates(&commissioning_data.rated_sensor_rates);
            status += ADCS_get_fine_sun_vector(&commissioning_data.fine_sun_vector);
            status += ADCS_get_nadir_vector(&commissioning_data.nadir_vector);
            status += ADCS_get_wheel_speed(&commissioning_data.measured_wheel_speeds);
            status += ADCS_get_magnetorquer_command(&commissioning_data.magnetorquer_commands);
            status += ADCS_get_igrf_magnetic_field_vector(&commissioning_data.igrf_magnetic_field_vector);
            status += ADCS_get_quaternion_error_vector(&commissioning_data.quaternion_error_vector);
            MEMORY_MODULE_FUNCTION(commissioning_data); // save to memory module
            break;
        }
        case ADCS_COMMISISONING_STEP_GROUND_TARGET_TRACKING_CONTROLLER: {
            ADCS_commissioning_ground_target_tracking_controller_struct_t commissioning_data;
            commissioning_data.current_unix_time = TIM_get_current_unix_epoch_time_ms();
            status += ADCS_get_estimate_fine_angular_rates(&commissioning_data.fine_estimated_angular_rates);
            status += ADCS_get_estimated_attitude_angles(&commissioning_data.estimated_attitude_angles);
            status += ADCS_get_estimated_gyro_bias(&commissioning_data.estimated_gyro_bias);
            status += ADCS_get_estimation_innovation_vector(&commissioning_data.estimation_innovation_vector);            
            status += ADCS_get_magnetic_field_vector(&commissioning_data.magnetic_field_vector);
            status += ADCS_get_rate_sensor_rates(&commissioning_data.rated_sensor_rates);
            status += ADCS_get_fine_sun_vector(&commissioning_data.fine_sun_vector);
            status += ADCS_get_nadir_vector(&commissioning_data.nadir_vector);
            status += ADCS_get_wheel_speed(&commissioning_data.measured_wheel_speeds);
            status += ADCS_get_magnetorquer_command(&commissioning_data.magnetorquer_commands);
            status += ADCS_get_igrf_magnetic_field_vector(&commissioning_data.igrf_magnetic_field_vector);
            status += ADCS_get_quaternion_error_vector(&commissioning_data.quaternion_error_vector);
            status += ADCS_get_llh_position(&commissioning_data.LLH_position);
            status += ADCS_get_commanded_attitude_angles(&commissioning_data.commanded_attitude_angles);
            MEMORY_MODULE_FUNCTION(commissioning_data); // save to memory module
            break;
        }
        case ADCS_COMMISISONING_STEP_GPS_RECEIVER: {
            ADCS_commissioning_gps_receiver_commissioning_struct_t commissioning_data;
            commissioning_data.current_unix_time = TIM_get_current_unix_epoch_time_ms();
            status += ADCS_get_llh_position(&commissioning_data.LLH_position);          
            status += ADCS_get_raw_gps_status(&commissioning_data.raw_GPS_status);
            status += ADCS_get_raw_gps_time(&commissioning_data.raw_GPS_time);
            status += ADCS_get_raw_gps_x(&commissioning_data.raw_GPS_x);
            status += ADCS_get_raw_gps_y(&commissioning_data.raw_GPS_y);
            status += ADCS_get_raw_gps_z(&commissioning_data.raw_GPS_z);            
            MEMORY_MODULE_FUNCTION(commissioning_data); // save to memory module
            break;
        }           
        default: {
            snprintf(response_output_buf, response_output_buf_len,
                "Commissioning step case out of range (err %d)", 1);
            return 1;
        }
    }

    return status;
}