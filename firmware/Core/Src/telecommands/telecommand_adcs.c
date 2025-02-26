#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_args_helpers.h"
#include "transforms/arrays.h"
#include "unit_tests/unit_test_executor.h"
#include "timekeeping/timekeeping.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

#include "adcs_drivers/adcs_types.h"
#include "adcs_drivers/adcs_commands.h"
#include "adcs_drivers/adcs_command_ids.h"
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
    uint8_t extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &command_id);
    if (extract_status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "Telecommand argument extraction failed (err %d)", extract_status);
        return 1;
    }

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
    extract_status = TCMD_extract_hex_array_arg(args_str, 1, &hex_data_array[0], data_length, &data_length);
    if (extract_status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "Telecommand argument extraction failed (err %d)", extract_status);
        return 1;
    }
    
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
    uint8_t extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &telemetry_request_id);
    if (extract_status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "Telecommand argument extraction failed in position 0 (err %d)", extract_status);
        return 1;
    }

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
    extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &data_length);
    if (extract_status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "Telecommand argument extraction failed in position 1 (err %d)", extract_status);
        return 1;
    }

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
        uint8_t extract_status = TCMD_extract_int64_arg(args_str, strlen(args_str), i, &arguments[i]);
        if (extract_status != 0) {
            snprintf(response_output_buf, response_output_buf_len,
                "Telecommand argument extraction failed in position %d (err %d)", i, extract_status);
            return 1;
        }
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
    uint8_t extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &timeout);
    if (extract_status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "Telecommand argument extraction failed (err %d)", extract_status);
        return 1;
    }
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
    uint8_t extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &run_mode);
    if (extract_status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "Telecommand argument extraction failed (err %d)", extract_status);
        return 1;
    }
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
        uint8_t extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), i, &arguments[i]);
        if (extract_status != 0) {
            snprintf(response_output_buf, response_output_buf_len,
                "Telecommand argument extraction failed in position %d (err %d)", i, extract_status);
            return 1;
        }
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
    uint8_t extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &estimation_mode);
    if (extract_status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "Telecommand argument extraction failed (err %d)", extract_status);
        return 1;
    }
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
    uint8_t extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &mode);
    if (extract_status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "Telecommand argument extraction failed (err %d)", extract_status);
        return 1;
    }
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
        uint8_t extract_status = TCMD_extract_double_arg(args_str, strlen(args_str), i, &arguments[i]);
        if (extract_status != 0) {
            snprintf(response_output_buf, response_output_buf_len,
                "Telecommand argument extraction failed in position %d (err %d)", i, extract_status);
            return 1;
        }
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
        uint8_t extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), i, &arguments[i]);
        if (extract_status != 0) {
            snprintf(response_output_buf, response_output_buf_len,
                "Telecommand argument extraction failed in position %d (err %d)", i, extract_status);
            return 1;
        }
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
        uint8_t extract_status = TCMD_extract_double_arg(args_str, strlen(args_str), i, &arguments[i]);
        if (extract_status != 0) {
            snprintf(response_output_buf, response_output_buf_len,
                "Telecommand argument extraction failed in position %d (err %d)", i, extract_status);
            return 1;
        }
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
        uint8_t extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), i, &bools[i]);
        if (extract_status != 0) {
            snprintf(response_output_buf, response_output_buf_len,
                "Telecommand argument extraction failed in position %d (err %d)", i, extract_status);
            return 1;
        }
    }
    
    uint8_t extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), 3, &uint_arg);
    if (extract_status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "Telecommand argument extraction failed in position 3 (err %d)", extract_status);
        return 1;
    }

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
        uint8_t extract_status = TCMD_extract_double_arg(args_str, strlen(args_str), i, &arguments[i]);
        if (extract_status != 0) {
            snprintf(response_output_buf, response_output_buf_len,
                "Telecommand argument extraction failed in position %d (err %d)", i, extract_status);
            return 1;
        }
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
        uint8_t extract_status = TCMD_extract_double_arg(args_str, strlen(args_str), i, &arguments[i]);
        if (extract_status != 0) {
            snprintf(response_output_buf, response_output_buf_len,
                "Telecommand argument extraction failed in position %d (err %d)", i, extract_status);
            return 1;
        }
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
///     - Arg 12: automatic_magnetometer_recovery (bool; select whether automatic switch to redundant magnetometer should occur in case of failure)
///     - Arg 13: magnetometer_mode (enum; select magnetometer mode for estimation and control)
///     - Arg 14: magnetometer_selection_for_raw_magnetometer_telemetry (enum; select magnetometer mode for the second raw telemetry frame)
///     - Arg 15: automatic_estimation_transition_due_to_rate_sensor_errors (bool; enable/disable automatic transition from MEMS rate estimation mode to RKF in case of rate sensor error)
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
        uint8_t extract_status = TCMD_extract_double_arg(args_str, strlen(args_str), i, &double_type_arguments[i]);
        if (extract_status != 0) {
            snprintf(response_output_buf, response_output_buf_len,
                "Telecommand argument extraction failed in position %d (err %d)", i, extract_status);
            return 1;
        }
        float_args[i] = double_type_arguments[i];
    }

    uint8_t new_num_args = 11;
    uint64_t uint_type_arguments[new_num_args]; 
    bool bool_args[8];
    ADCS_magnetometer_mode_enum_t enum_args[2];
    uint8_t uint8_arg;

    for (uint8_t i = 0; i < new_num_args; i++) {
        uint8_t extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), i + 7, &uint_type_arguments[i]);
        if (extract_status != 0) {
            snprintf(response_output_buf, response_output_buf_len,
                "Telecommand argument extraction failed in position %d (err %d)", i + 7, extract_status);
            return 1;
        }
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
            uint8_t extract_status = TCMD_extract_double_arg(args_str, strlen(args_str), i, &doubles_params[double_counter]);
            if (extract_status != 0) {
                snprintf(response_output_buf, response_output_buf_len,
            "Telecommand argument extraction failed in position %d (err %d)", i, extract_status);
        return 1;
            }
            double_counter++;
        }
        else if (i == 7 || i == 10 || i == 13 || i == 16) {
            uint8_t extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), i, &uint_params[uint_counter]);
            if (extract_status != 0) {
                snprintf(response_output_buf, response_output_buf_len,
            "Telecommand argument extraction failed in position %d (err %d)", i, extract_status);
        return 1;
            }
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
        uint8_t extract_status = TCMD_extract_double_arg(args_str, strlen(args_str), i, &arguments[i]);
        if (extract_status != 0) {
            snprintf(response_output_buf, response_output_buf_len,
                "Telecommand argument extraction failed in position %d (err %d)", i, extract_status);
            return 1;
        }
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
        uint8_t extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), i, &axis_arguments[i]);
        if (extract_status != 0) {
            snprintf(response_output_buf, response_output_buf_len,
                "Telecommand argument extraction failed in position %d (err %d)", i, extract_status);
            return 1;
        }
    }

    // parse axis select arguments into doubles
    uint8_t num_offset_args = 3;
    double offset_arguments[num_offset_args]; 
    for (uint8_t i = 0; i < num_offset_args; i++) {
        uint8_t extract_status = TCMD_extract_double_arg(args_str, strlen(args_str), i + 3, &offset_arguments[i]);
        if (extract_status != 0) {
            snprintf(response_output_buf, response_output_buf_len,
                "Telecommand argument extraction failed in position %d (err %d)", i + 3, extract_status);
            return 1;
        }
    }

    uint64_t rate_sensor_mult;
    uint8_t extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), 6, &rate_sensor_mult);
    if (extract_status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "Telecommand argument extraction failed in position 6 (err %d)", extract_status);
        return 1;
    }
    
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

/// @brief Telecommand: Get the list of downloadable files from the ADCS SD card (alternate method)
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_download_index_file(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status;

    // TODO: Function in progress... once the main download function is finished this will be easier.
    // I'm using this as a simpler proxy for the main file download function. 
    // This should work so long as there are fewer than 541 files on the SD card. 

    /*
    The file is uniquely identified by the File Type and Counter parameters. The Offset and Block
    Length parameters indicate which part of the file to buffer in memory. The maximum Block
    Length is 20 kB. [20480 bytes (20 bytes * 1024 packets). Some files may require multiple blocks, such as image files]
    */

    // We can perform up to 1024 read transactions at a time. Each read transaction loads 20 payload bytes from memory. 
    ADCS_file_download_buffer_struct_t file_packets[1024];
    uint64_t packet_index = 0;
    
    // Load a download block from the ADCS
    status = ADCS_load_file_download_block(ADCS_FILE_TYPE_INDEX, 0, 0, 20);
    if (status != 0) {snprintf(response_output_buf, response_output_buf_len, "ADCS load download block failed (err %d)", status); return 1;}
        // TODO: I'm not sure what, exactly, the 'offset' parameter in this function is for. I think the counter is the block_counter, but also not sure.

    // Wait until the download block is ready
    ADCS_download_block_ready_struct_t ready_struct;
    do {
        status = ADCS_get_download_block_ready_telemetry(&ready_struct);
        if (status != 0) {snprintf(response_output_buf, response_output_buf_len, "ADCS get download ready status failed (err %d)", status); return 1;}
    } while (ready_struct.ready != true);
    
    // Initiate download burst, ignoring the hole map
    status = ADCS_initiate_download_burst(20, true);
    if (status != 0) {snprintf(response_output_buf, response_output_buf_len, "ADCS initiate download burst failed (err %d)", status); return 1;}
        // TODO: I suspect that the message_length parameter represents the number of bytes to load, but I'm not sure.

    /* 
    If the Initiate File Download Burst command is issued on the I2C communications link, it is
    expected to successively perform up to 1024 read transactions from the remote side, each with
    22 bytes length, again with the same format as [TLM 241, File Download Buffer...] 
    (the number [of read transactions] depends on the Block Length specified with the Load File Download Block command), 
    each with a payload length of 20 bytes in rapid succession. Each download packet will have a 
    header ID that matches that of the Initiate File Download Burst command. The following two bytes 
    will contain the counter of the packet in the burst. The counter makes it possible to keep track of
    which packets have been received on the remote side. 
    */

    for (uint16_t i = 0; i < 1024; i++) {
        status = ADCS_get_file_download_buffer(&(file_packets[i]));
        if (status != 0) {snprintf(response_output_buf, response_output_buf_len, "ADCS get download buffer failed for index %d (err %d)", i, status); return 1;}
    }

    for (uint16_t i = 0; i < 1024; i++) {
        // now that we've loaded the packets, save them to the output file
        file_packets[packet_index + i] = file_packets[i]; 
    }

    /*
    The Hole Map is a bitmap, where each bit represents one 20-byte packet out of the complete
    download block. Since there are at most 1024 packets in one file download block, the Hole
    Map is also 1024 bits long, or 128 bytes.
    To make the CubeComputer resend only the packets that have been missed by the remote
    side, the remote component must upload the Hole Map to the CubeComputer by performing
    up to 8 Hole Map commands (each Hole Map command contains 16 bytes of the Hole Map).
    After uploading the Hole Map, another Initiate Download Burst command can be issued, but
    this time with the Ignore Hole Map parameter set to false. The CubeComputer will then
    transmit only the packets that have a corresponding ‘0’ in the Hole Map.
    */

    bool hole_map[1024];
    for (uint16_t i = 0; i< 1024; i++) {
        hole_map[i] = false;
        for (uint16_t j = 0; j < 1024; j++) {
            // iterate through the buffer and determine which packets have been received
            if (file_packets[j].packet_counter == i) {
                hole_map[i] = true;
                break;
            }
        }
    }

    // this block converts the booleans to bytes, forwardly (e.g. 0b1111000000001111 becomes 0xf0, 0x0f)
    for (uint8_t i = 1; i <= 8; i++) {
        uint8_t hole_bytes[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        // convert an appropriate slice of the array of bools into a 16-byte array
        for (uint16_t j = ((i-1) * 128); j < (i * 128); j++) {
            // 8 bits * 16 bytes = 128 bits per Hole Map, starting at index (i - 1) * 128
            hole_bytes[(j / 8) - ((i-1) * 16)] |= (hole_map[j] << (7 - (j % 8))); 
        }

        // now set the hole map
        status = ADCS_set_hole_map(hole_bytes, i);
        if (status != 0) {snprintf(response_output_buf, response_output_buf_len, "ADCS set hole map failed for index %d (err %d)", i, status); return 1;}
    }

    snprintf(response_output_buf, response_output_buf_len, "ADCS file download probably worked");

    return 33;

}


/// @brief Telecommand: Get the list of downloadable files from the ADCS SD card
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_sd_download_list(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint8_t status;

    // Reset the file list read pointer
    status = ADCS_reset_file_list_read_pointer();
    if (status != 0 && status != 165) {
        // For some reason, the engineering model ADCS always returns status 165 for this command, regardless of what the status *should* be
        // Effectively, this means we cannot effectively check the status of this command, so we must use a workaround
        // TODO: test this with CubeSupport to see if it gets anything reasonable out of it
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS reset file list read pointer failed (err %d)", status);
        return 1;
    } 
    uint16_t filelist_length = 0;
    ADCS_file_info_struct_t file_info[64]; // TODO: it did not like allocating 1024 of these; check how much needed

    for (uint16_t i = 0; i < 63; i++) {

        // Get the information for each file, polling until data is ready
        do {
            status = ADCS_get_file_info_telemetry(&(file_info[i])); 
            if (status != 0) {
                snprintf(response_output_buf, response_output_buf_len,
                    "ADCS get file info telemetry failed for index %d (err %d)", i, status);
                return 1;
            }
            
        } while (file_info[i].busy_updating != false);

        // if everything is zero, we have reached the end of the file list
        if (file_info[i].file_crc16 == 0 && file_info[i].file_size == 0) {
            filelist_length = i;
            break;
        }

        // otherwise, advance the file list read pointer and do it all again
        status = ADCS_advance_file_list_read_pointer();
        if (status != 0) {
            snprintf(response_output_buf, response_output_buf_len,
                "ADCS advance file list read pointer failed for index %d (err %d)", i, status);
            return 1;
        }
        
    }

    // pack to JSON string and output
    const uint8_t result_json = ADCS_sd_download_list_TO_json(&file_info[0], filelist_length, response_output_buf, response_output_buf_len);

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

/// @brief Telecommand: Download a specific file from the ADCS SD card
/// @param args_str 
///     - Arg 0: The index of the file to download
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_download_sd_file(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {

    uint8_t status;

    // parse file index argument
    uint64_t file_index;
    TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &file_index);

    // get the data about the file to download
    status = ADCS_reset_file_list_read_pointer();
    if (status != 0 && status != 165) {snprintf(response_output_buf, response_output_buf_len, "ADCS reset file pointer failed (err %d)", status); return 1;}
    // For some reason, the engineering model ADCS always returns status 165 for this command, regardless of what the status *should* be
    // Effectively, this means we cannot effectively check the status of this command, so we must use a workaround
    // TODO: write a workaround

    for (uint16_t i = 0; i < file_index; i++) {
        status = ADCS_advance_file_list_read_pointer();
        if (status != 0) {snprintf(response_output_buf, response_output_buf_len, "ADCS advance file pointer failed at index %d (err %d)", i, status); return 1;}
    }
    ADCS_file_info_struct_t file_info;
    status = ADCS_get_file_info_telemetry(&file_info);
    if (status != 0) {snprintf(response_output_buf, response_output_buf_len, "ADCS get file info failed (err %d)", status); return 1;}

    /*
    The file is uniquely identified by the File Type and Counter parameters. The Offset and Block
    Length parameters indicate which part of the file to buffer in memory. The maximum Block
    Length is 20 kB. [20480 bytes (20 bytes * 1024 packets). Some files may require multiple blocks, such as image files]
    */

    uint16_t blocks_required = ceil(file_info.file_size / 20480.0);
    uint64_t remaining_bytes = file_info.file_size;
    
    ADCS_file_download_buffer_struct_t file_packets[(uint64_t) ceil(file_info.file_size/20.0)];
    uint64_t packet_index = 0;

    for (uint16_t block_counter = 0; block_counter < blocks_required; block_counter++) {
        // repeat all this for every block

        uint16_t bytes_to_load = 1024 * 20;
        if (remaining_bytes < bytes_to_load) {
            bytes_to_load = remaining_bytes;
        }

        // We can perform up to 1024 read transactions at a time. Each read transaction loads 20 payload bytes from memory. 
        uint16_t packets_to_load = (uint16_t) ceil(bytes_to_load / 20.0);
        
        // Load a download block from the ADCS
        status = ADCS_load_file_download_block(file_info.file_type, block_counter, 0, bytes_to_load);
        if (status != 0) {snprintf(response_output_buf, response_output_buf_len, "ADCS load download block failed (err %d)", status); return 1;}
            // TODO: I'm not sure what, exactly, the 'offset' parameter in this function is for. I think the counter is the block_counter, but also not sure.

        // Wait until the download block is ready
        ADCS_download_block_ready_struct_t ready_struct;
        do {
            status = ADCS_get_download_block_ready_telemetry(&ready_struct);
            if (status != 0) {snprintf(response_output_buf, response_output_buf_len, "ADCS get download ready status failed (err %d)", status); return 1;}
        } while (ready_struct.ready != true);
        
        // Initiate download burst, ignoring the hole map
        status = ADCS_initiate_download_burst(bytes_to_load, true);
        if (status != 0) {snprintf(response_output_buf, response_output_buf_len, "ADCS initiate download burst failed (err %d)", status); return 1;}
            // TODO: I suspect that the message_length parameter represents the number of bytes to load, but I'm not sure.

        /* 
        If the Initiate File Download Burst command is issued on the I2C communications link, it is
        expected to successively perform up to 1024 read transactions from the remote side, each with
        22 bytes length, again with the same format as [TLM 241, File Download Buffer...] 
        (the number [of read transactions] depends on the Block Length specified with the Load File Download Block command), 
        each with a payload length of 20 bytes in rapid succession. Each download packet will have a 
        header ID that matches that of the Initiate File Download Burst command. The following two bytes 
        will contain the counter of the packet in the burst. The counter makes it possible to keep track of
        which packets have been received on the remote side. 
        */

        ADCS_file_download_buffer_struct_t buffer[packets_to_load];
        for (uint16_t i = 0; i < packets_to_load; i++) {
            status = ADCS_get_file_download_buffer(&(buffer[i]));
            if (status != 0) {snprintf(response_output_buf, response_output_buf_len, "ADCS get download buffer failed for index %d (err %d)", i, status); return 1;}
        }

        for (uint16_t i = 0; i < packets_to_load; i++) {
            // now that we've loaded the packets, save them to the output file
            file_packets[packet_index + i] = buffer[i]; 
        }
        packet_index += packets_to_load;

        /*
        The Hole Map is a bitmap, where each bit represents one 20-byte packet out of the complete
        download block. Since there are at most 1024 packets in one file download block, the Hole
        Map is also 1024 bits long, or 128 bytes.
        To make the CubeComputer resend only the packets that have been missed by the remote
        side, the remote component must upload the Hole Map to the CubeComputer by performing
        up to 8 Hole Map commands (each Hole Map command contains 16 bytes of the Hole Map).
        After uploading the Hole Map, another Initiate Download Burst command can be issued, but
        this time with the Ignore Hole Map parameter set to false. The CubeComputer will then
        transmit only the packets that have a corresponding ‘0’ in the Hole Map.
        */

        bool hole_map[1024];
        for (uint16_t i = 0; i< 1024; i++) {
            hole_map[i] = false;
            for (uint16_t j = 0; j < bytes_to_load; j++) {
                // iterate through the buffer and determine which packets have been received
                if (buffer[j].packet_counter == i) {
                    hole_map[i] = true;
                    break;
                }
            }
        }

        // this block converts the booleans to bytes, forwardly (e.g. 0b1111000000001111 becomes 0xf0, 0x0f)
        for (uint8_t i = 1; i <= 8; i++) {
            uint8_t hole_bytes[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            // convert an appropriate slice of the array of bools into a 16-byte array
            for (uint16_t j = ((i-1) * 128); j < (i * 128); j++) {
                // 8 bits * 16 bytes = 128 bits per Hole Map, starting at index (i - 1) * 128
                hole_bytes[(j / 8) - ((i-1) * 16)] |= (hole_map[j] << (7 - (j % 8))); 
            }

            // now set the hole map
            status = ADCS_set_hole_map(hole_bytes, i);
            if (status != 0) {snprintf(response_output_buf, response_output_buf_len, "ADCS set hole map failed for index %d (err %d)", i, status); return 1;}
        }

        remaining_bytes -= bytes_to_load;
    }

    snprintf(response_output_buf, response_output_buf_len, "ADCS file download probably worked");
    // TODO: reassemble the file and save it to memory (and also don't say "probably")

    qsort(&file_packets[0], sizeof(file_packets), sizeof(ADCS_file_download_buffer_struct_t), ADCS_compare_download_packets); // sort the file packets in ascending order of packet counter
    
    uint8_t file_bytes[file_info.file_size];
    // TODO: check if we have enough RAM for this

    // now that the packets are sorted, we can turn them into a regular array of bytes
    for (uint64_t i = 0; i < ((uint64_t) ceil(file_info.file_size/20.0)); i++) {
        file_bytes[i] = file_packets[i / 1024].file_bytes[i % 1024];
    }

    if (file_bytes[0] == 0) {
        ;;;;;; // this if block is useless and only here to prevent a warning
    }

    WRITE_STRUCT_TO_MEMORY(file_bytes)

    return 0;
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
        uint8_t extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), i, &arguments[i]);
        if (extract_status != 0) {
            snprintf(response_output_buf, response_output_buf_len,
                "Telecommand argument extraction failed in position %d (err %d)", i, extract_status);
            return 1;
        }
        args_8[i] = (uint8_t) arguments[i];
    }
    
    uint8_t status = ADCS_save_image_to_sd(args_8[0], args_8[1]); 
    return status;
}

/// @brief Telecommand: Synchronise the current ADCS Unix epoch time
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_synchronise_unix_time(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_synchronise_unix_time(); 
    return status;
}

/// @brief Telecommand: Retrieve the current ADCS Unix epoch time
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_current_unix_time(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t unix_time_ms;
    uint8_t status = ADCS_get_current_unix_time(&unix_time_ms); 
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS Unix time telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_unix_time_ms_TO_json(
        &unix_time_ms, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS Unix time telemetry JSON conversion failed (err %d)", result_json);
        return 2;
    }

    return status;
}

/// @brief Telecommand: Repeatedly log given data from an SD card
/// @param args_str 
///     - Arg 0: which_log; 1 or 2; which specific log number to log to the SD card
///     - Arg 1: log_array; Hex array of bitmasks for log config (10 hex bytes)
///     - Arg 2: log_period; Period to log data to the SD card; if zero, then disable logging
///     - Arg 3: which_sd; Which SD card to log to, 0 for primary, 1 for secondary 
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_set_sd_log_config(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t which_log;
    uint8_t extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &which_log);
    if (extract_status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "Telecommand argument extraction failed (err %d)", extract_status);
        return 1;
    }

    // parse hex array arguments
    uint8_t hex_data_array[10]; 
    uint16_t data_length;
    extract_status = TCMD_extract_hex_array_arg(args_str, 1, &hex_data_array[0], data_length, &data_length);
    if (extract_status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "Telecommand argument extraction failed (err %d)", extract_status);
        return 1;
    }

    const uint8_t *data_pointer[1] = {hex_data_array};

    uint64_t log_period;
    extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), 2, &log_period);
    if (extract_status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "Telecommand argument extraction failed in position 2 (err %d)", extract_status);
        return 1;
    }

    uint64_t which_sd;
    extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), 3, &which_sd);
    if (extract_status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "Telecommand argument extraction failed in position 3 (err %d)", extract_status);
        return 1;
    }

    uint8_t status = ADCS_set_sd_log_config((uint8_t) which_log, data_pointer, 1, (uint8_t) log_period, (uint8_t) which_sd);

    return status;
}

/// @brief Telecommand: Retrieve the current ADCS SD log configuration
/// @param args_str 
///     - Arg 0: which log to retrieve the configuration for (1 or 2)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_get_sd_log_config(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t which_log;
    uint8_t extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &which_log);
    if (extract_status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "Telecommand argument extraction failed (err %d)", extract_status);
        return 1;
    }

    ADCS_sd_log_config_struct result_struct;
    uint8_t status = ADCS_get_sd_log_config((uint8_t) which_log, &result_struct); 
    
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS SD log config telemetry request failed (err %d)", status);
        return 1;
    }

    const uint8_t result_json = ADCS_sd_log_config_struct_TO_json(
        &result_struct, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "ADCS SD log config telemetry JSON conversion failed (err %d)", result_json);
        return 2;
    }

    return status;
}

/// @brief Telecommand: Request commissioning telemetry from the ADCS and save it to the onboard SD card
/// @param args_str 
///     - Arg 0: Which commissioning step to request telemetry for (1-18)
///     - Arg 1: Log number (1 or 2)
///     - Arg 2: Destination SD card (0 = primary, 1 = secondary)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_request_commissioning_telemetry(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    // parse arguments: first into int64_t, then convert to correct form for input
    uint64_t arguments[3];
    for (uint8_t i = 0; i < 3; i++) {
        uint8_t extract_status = TCMD_extract_uint64_arg(args_str, strlen(args_str), i, &arguments[i]);
        if (extract_status != 0) {
            snprintf(response_output_buf, response_output_buf_len,
                "Telecommand argument extraction failed in position %d (err %d)", i, extract_status);
            return 1;
        }
    }
    ADCS_commissioning_step_enum_t commissioning_step = (ADCS_commissioning_step_enum_t) arguments[0];
    uint8_t log_number = (uint8_t) arguments[1];
    ADCS_sd_log_destination_enum_t sd_destination = (ADCS_sd_log_destination_enum_t) arguments[2];

    if (log_number == 0 || log_number > 2) {
        snprintf(response_output_buf, response_output_buf_len,
            "Commissioning SD log number may be only 1 or 2");
        return 8;
    }

    if (sd_destination > 1) {
        snprintf(response_output_buf, response_output_buf_len,
            "SD destination number may only be 0 (primary) or 1 (secondary)");
        return 9;
    }

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

    switch(commissioning_step) {

        case ADCS_COMMISSIONING_STEP_DETERMINE_INITIAL_ANGULAR_RATES: {
            const uint8_t num_logs = 3;
            const uint8_t period_s = 10; 
            const uint8_t* commissioning_data[3] = {ADCS_SD_LOG_MASK_ESTIMATED_ANGULAR_RATES, ADCS_SD_LOG_MASK_RATE_SENSOR_RATES, ADCS_SD_LOG_MASK_RAW_MAGNETOMETER};
            status = ADCS_set_sd_log_config(log_number, commissioning_data, num_logs, period_s, sd_destination);
            break;
        }
        case ADCS_COMMISSIONING_STEP_INITIAL_DETUMBLING: {
            const uint8_t num_logs = 4; 
            const uint8_t period_s = 10; 
            const uint8_t* commissioning_data[4] = {ADCS_SD_LOG_MASK_ESTIMATED_ANGULAR_RATES, ADCS_SD_LOG_MASK_RATE_SENSOR_RATES, ADCS_SD_LOG_MASK_RAW_MAGNETOMETER, ADCS_SD_LOG_MASK_MAGNETORQUER_COMMAND};
            status = ADCS_set_sd_log_config(log_number, commissioning_data, num_logs, period_s, sd_destination);
            break;
        }
        case ADCS_COMMISSIONING_STEP_CONTINUED_DETUMBLING_TO_Y_THOMSON: {
            const uint8_t num_logs = 4; 
            const uint8_t period_s = 10; 
            const uint8_t* commissioning_data[4] = {ADCS_SD_LOG_MASK_ESTIMATED_ANGULAR_RATES, ADCS_SD_LOG_MASK_RATE_SENSOR_RATES, ADCS_SD_LOG_MASK_RAW_MAGNETOMETER, ADCS_SD_LOG_MASK_MAGNETORQUER_COMMAND};
            status = ADCS_set_sd_log_config(log_number, commissioning_data, num_logs, period_s, sd_destination);
            break;
        }
        case ADCS_COMMISSIONING_STEP_MAGNETOMETER_DEPLOYMENT: {
            const uint8_t num_logs = 4; 
            const uint8_t period_s = 10; 
            const uint8_t* commissioning_data[4] = {ADCS_SD_LOG_MASK_FINE_ESTIMATED_ANGULAR_RATES, ADCS_SD_LOG_MASK_RATE_SENSOR_RATES, ADCS_SD_LOG_MASK_RAW_MAGNETOMETER, ADCS_SD_LOG_MASK_CUBECONTROL_CURRENT_MEASUREMENTS};
            status = ADCS_set_sd_log_config(log_number, commissioning_data, num_logs, period_s, sd_destination);
            break;
        }
        case ADCS_COMMISSIONING_STEP_MAGNETOMETER_CALIBRATION: {
            const uint8_t num_logs = 3; 
            const uint8_t period_s = 10; 
            const uint8_t* commissioning_data[3] = {ADCS_SD_LOG_MASK_FINE_ESTIMATED_ANGULAR_RATES, ADCS_SD_LOG_MASK_RATE_SENSOR_RATES, ADCS_SD_LOG_MASK_MAGNETIC_FIELD_VECTOR};
            status = ADCS_set_sd_log_config(log_number, commissioning_data, num_logs, period_s, sd_destination);                     
            // TODO: Magnetic Field Vector **should** be the calibrated measurements from the magnetometer, but we should see if we can check with CubeSpace about that
            break;
        }
        case ADCS_COMMISSIONING_STEP_ANGULAR_RATE_AND_PITCH_ANGLE_ESTIMATION: {
            const uint8_t num_logs = 4; 
            const uint8_t period_s = 10; 
            const uint8_t* commissioning_data[4] = {ADCS_SD_LOG_MASK_FINE_ESTIMATED_ANGULAR_RATES, ADCS_SD_LOG_MASK_ESTIMATED_ATTITUDE_ANGLES, ADCS_SD_LOG_MASK_RATE_SENSOR_RATES, ADCS_SD_LOG_MASK_MAGNETIC_FIELD_VECTOR};
            status = ADCS_set_sd_log_config(log_number, commissioning_data, num_logs, period_s, sd_destination);                     
            break;
        }
        case ADCS_COMMISSIONING_STEP_Y_WHEEL_RAMP_UP_TEST: {
            const uint8_t num_logs = 5; 
            const uint8_t period_s = 10; 
            const uint8_t* commissioning_data[5] = {ADCS_SD_LOG_MASK_FINE_ESTIMATED_ANGULAR_RATES, ADCS_SD_LOG_MASK_ESTIMATED_ATTITUDE_ANGLES, ADCS_SD_LOG_MASK_RATE_SENSOR_RATES, ADCS_SD_LOG_MASK_WHEEL_SPEED, ADCS_SD_LOG_MASK_MAGNETIC_FIELD_VECTOR};
            status = ADCS_set_sd_log_config(log_number, commissioning_data, num_logs, period_s, sd_destination);   
            break;
        }
        case ADCS_COMMISSIONING_STEP_INITIAL_Y_MOMENTUM_ACTIVATION: {
            const uint8_t num_logs = 6; 
            const uint8_t period_s = 10; 
            const uint8_t* commissioning_data[6] = {ADCS_SD_LOG_MASK_FINE_ESTIMATED_ANGULAR_RATES, ADCS_SD_LOG_MASK_ESTIMATED_ATTITUDE_ANGLES, ADCS_SD_LOG_MASK_RATE_SENSOR_RATES, ADCS_SD_LOG_MASK_WHEEL_SPEED, ADCS_SD_LOG_MASK_MAGNETIC_FIELD_VECTOR, ADCS_SD_LOG_MASK_SATELLITE_POSITION_LLH};
            status = ADCS_set_sd_log_config(log_number, commissioning_data, num_logs, period_s, sd_destination);   
            break;
        }
        case ADCS_COMMISSIONING_STEP_CONTINUED_Y_MOMENTUM_ACTIVATION_AND_MAGNETOMETER_EKF: {
            const uint8_t num_logs = 6; 
            const uint8_t period_s = 10; 
            const uint8_t* commissioning_data[6] = {ADCS_SD_LOG_MASK_FINE_ESTIMATED_ANGULAR_RATES, ADCS_SD_LOG_MASK_ESTIMATED_ATTITUDE_ANGLES, ADCS_SD_LOG_MASK_RATE_SENSOR_RATES, ADCS_SD_LOG_MASK_WHEEL_SPEED, ADCS_SD_LOG_MASK_MAGNETIC_FIELD_VECTOR, ADCS_SD_LOG_MASK_SATELLITE_POSITION_LLH};
            status = ADCS_set_sd_log_config(log_number, commissioning_data, num_logs, period_s, sd_destination);   
            break;
        }
        case ADCS_COMMISSIONING_STEP_CUBESENSE_SUN_NADIR: {
            const uint8_t num_logs = 9; 
            const uint8_t period_s = 10; 
            const uint8_t* commissioning_data[9] = {ADCS_SD_LOG_MASK_FINE_ESTIMATED_ANGULAR_RATES, ADCS_SD_LOG_MASK_ESTIMATED_ATTITUDE_ANGLES, ADCS_SD_LOG_MASK_RATE_SENSOR_RATES, 
                                                            ADCS_SD_LOG_MASK_RAW_CSS_1_TO_6, ADCS_SD_LOG_MASK_RAW_CSS_7_TO_10, ADCS_SD_LOG_MASK_RAW_CAM1_SENSOR, ADCS_SD_LOG_MASK_RAW_CAM2_SENSOR, 
                                                            ADCS_SD_LOG_MASK_FINE_SUN_VECTOR, ADCS_SD_LOG_MASK_NADIR_VECTOR};
            status = ADCS_set_sd_log_config(log_number, commissioning_data, num_logs, period_s, sd_destination);   
            break;
        }
        case ADCS_COMMISSIONING_STEP_EKF_ACTIVATION_SUN_AND_NADIR: {
            const uint8_t num_logs = 9; 
            const uint8_t period_s = 10; 
            const uint8_t* commissioning_data[9] = {ADCS_SD_LOG_MASK_FINE_ESTIMATED_ANGULAR_RATES, ADCS_SD_LOG_MASK_ESTIMATED_ATTITUDE_ANGLES, ADCS_SD_LOG_MASK_RATE_SENSOR_RATES, 
                                                            ADCS_SD_LOG_MASK_RAW_CSS_1_TO_6, ADCS_SD_LOG_MASK_RAW_CSS_7_TO_10, ADCS_SD_LOG_MASK_RAW_CAM1_SENSOR, ADCS_SD_LOG_MASK_RAW_CAM2_SENSOR, 
                                                            ADCS_SD_LOG_MASK_FINE_SUN_VECTOR, ADCS_SD_LOG_MASK_NADIR_VECTOR};
            status = ADCS_set_sd_log_config(log_number, commissioning_data, num_logs, period_s, sd_destination);  
            break;
        }
        case ADCS_COMMISSIONING_STEP_CUBESTAR_STAR_TRACKER: {
            const uint8_t num_logs = 8; 
            const uint8_t period_s = 10; 
            const uint8_t* commissioning_data[8] = {ADCS_SD_LOG_MASK_FINE_ESTIMATED_ANGULAR_RATES, ADCS_SD_LOG_MASK_ESTIMATED_ATTITUDE_ANGLES, ADCS_SD_LOG_MASK_RATE_SENSOR_RATES, 
                                                            ADCS_SD_LOG_MASK_STAR_PERFORMANCE1, ADCS_SD_LOG_MASK_STAR_PERFORMANCE2, 
                                                            ADCS_SD_LOG_MASK_STAR_1_RAW_DATA, ADCS_SD_LOG_MASK_STAR_2_RAW_DATA, ADCS_SD_LOG_MASK_STAR_3_RAW_DATA};
            status = ADCS_set_sd_log_config(log_number, commissioning_data, num_logs, period_s, sd_destination);    
            break;
        }
        case ADCS_COMMISSIONING_STEP_EKF_ACTIVATION_WITH_STAR_VECTOR_MEASUREMENTS: {
            const uint8_t num_logs = 8; 
            const uint8_t period_s = 10; 
            const uint8_t* commissioning_data[8] = {ADCS_SD_LOG_MASK_FINE_ESTIMATED_ANGULAR_RATES, ADCS_SD_LOG_MASK_ESTIMATED_ATTITUDE_ANGLES, ADCS_SD_LOG_MASK_RATE_SENSOR_RATES, 
                                                            ADCS_SD_LOG_MASK_STAR_PERFORMANCE1, ADCS_SD_LOG_MASK_STAR_PERFORMANCE2, 
                                                            ADCS_SD_LOG_MASK_STAR_1_RAW_DATA, ADCS_SD_LOG_MASK_STAR_2_RAW_DATA, ADCS_SD_LOG_MASK_STAR_3_RAW_DATA};
            status = ADCS_set_sd_log_config(log_number, commissioning_data, num_logs, period_s, sd_destination);    
            break;
        }
        case ADCS_COMMISSIONING_STEP_ZERO_BIAS_3_AXIS_REACTION_WHEEL_CONTROL: {
            const uint8_t num_logs = 4; 
            const uint8_t period_s = 10; 
            const uint8_t* commissioning_data[4] = {ADCS_SD_LOG_MASK_FINE_ESTIMATED_ANGULAR_RATES, ADCS_SD_LOG_MASK_ESTIMATED_ATTITUDE_ANGLES, ADCS_SD_LOG_MASK_RATE_SENSOR_RATES, ADCS_SD_LOG_MASK_WHEEL_SPEED};
            status = ADCS_set_sd_log_config(log_number, commissioning_data, num_logs, period_s, sd_destination);   
            break;
        }
        case ADCS_COMMISSIONING_STEP_EKF_WITH_RATE_GYRO_STAR_TRACKER_MEASUREMENTS: {
            const uint8_t num_logs = 12; 
            const uint8_t period_s = 10; 
            const uint8_t* commissioning_data[12] = {ADCS_SD_LOG_MASK_FINE_ESTIMATED_ANGULAR_RATES, ADCS_SD_LOG_MASK_ESTIMATED_ATTITUDE_ANGLES, ADCS_SD_LOG_MASK_ESTIMATED_GYRO_BIAS, ADCS_SD_LOG_MASK_ESTIMATION_INNOVATION_VECTOR, 
                                                            ADCS_SD_LOG_MASK_MAGNETIC_FIELD_VECTOR, ADCS_SD_LOG_MASK_RATE_SENSOR_RATES, ADCS_SD_LOG_MASK_FINE_SUN_VECTOR, ADCS_SD_LOG_MASK_NADIR_VECTOR, ADCS_SD_LOG_MASK_WHEEL_SPEED, ADCS_SD_LOG_MASK_MAGNETORQUER_COMMAND,
                                                            ADCS_SD_LOG_MASK_IGRF_MODELLED_MAGNETIC_FIELD_VECTOR, ADCS_SD_LOG_MASK_QUATERNION_ERROR_VECTOR};
            status = ADCS_set_sd_log_config(log_number, commissioning_data, num_logs, period_s, sd_destination);   
            break;
        }
        case ADCS_COMMISSIONING_STEP_SUN_TRACKING_3_AXIS_CONTROL: {
            const uint8_t num_logs = 12; 
            const uint8_t period_s = 10; 
            const uint8_t* commissioning_data[12] = {ADCS_SD_LOG_MASK_FINE_ESTIMATED_ANGULAR_RATES, ADCS_SD_LOG_MASK_ESTIMATED_ATTITUDE_ANGLES, ADCS_SD_LOG_MASK_ESTIMATED_GYRO_BIAS, ADCS_SD_LOG_MASK_ESTIMATION_INNOVATION_VECTOR, 
                                                            ADCS_SD_LOG_MASK_MAGNETIC_FIELD_VECTOR, ADCS_SD_LOG_MASK_RATE_SENSOR_RATES, ADCS_SD_LOG_MASK_FINE_SUN_VECTOR, ADCS_SD_LOG_MASK_NADIR_VECTOR, ADCS_SD_LOG_MASK_WHEEL_SPEED, ADCS_SD_LOG_MASK_MAGNETORQUER_COMMAND,
                                                            ADCS_SD_LOG_MASK_IGRF_MODELLED_MAGNETIC_FIELD_VECTOR, ADCS_SD_LOG_MASK_QUATERNION_ERROR_VECTOR};
            status = ADCS_set_sd_log_config(log_number, commissioning_data, num_logs, period_s, sd_destination);   
            break;
        }
        case ADCS_COMMISSIONING_STEP_GROUND_TARGET_TRACKING_CONTROLLER: {
            const uint8_t num_logs = 14; 
            const uint8_t period_s = 10; 
            const uint8_t* commissioning_data[14] = {ADCS_SD_LOG_MASK_FINE_ESTIMATED_ANGULAR_RATES, ADCS_SD_LOG_MASK_ESTIMATED_ATTITUDE_ANGLES, ADCS_SD_LOG_MASK_ESTIMATED_GYRO_BIAS, ADCS_SD_LOG_MASK_ESTIMATION_INNOVATION_VECTOR, 
                                                            ADCS_SD_LOG_MASK_MAGNETIC_FIELD_VECTOR, ADCS_SD_LOG_MASK_RATE_SENSOR_RATES, ADCS_SD_LOG_MASK_FINE_SUN_VECTOR, ADCS_SD_LOG_MASK_NADIR_VECTOR, ADCS_SD_LOG_MASK_WHEEL_SPEED, ADCS_SD_LOG_MASK_MAGNETORQUER_COMMAND,
                                                            ADCS_SD_LOG_MASK_IGRF_MODELLED_MAGNETIC_FIELD_VECTOR, ADCS_SD_LOG_MASK_QUATERNION_ERROR_VECTOR, ADCS_SD_LOG_MASK_SATELLITE_POSITION_LLH, ADCS_SD_LOG_MASK_ESTIMATED_ATTITUDE_ANGLES};
            status = ADCS_set_sd_log_config(log_number, commissioning_data, num_logs, period_s, sd_destination);   
            break;
        }
        case ADCS_COMMISSIONING_STEP_GPS_RECEIVER: {
            const uint8_t num_logs = 6; 
            const uint8_t period_s = 10; 
            const uint8_t* commissioning_data[6] = {ADCS_SD_LOG_MASK_SATELLITE_POSITION_LLH, ADCS_SD_LOG_MASK_RAW_GPS_STATUS, ADCS_SD_LOG_MASK_RAW_GPS_TIME, 
                                                            ADCS_SD_LOG_MASK_RAW_GPS_X, ADCS_SD_LOG_MASK_RAW_GPS_Y, ADCS_SD_LOG_MASK_RAW_GPS_Z};
            status = ADCS_set_sd_log_config(log_number, commissioning_data, num_logs, period_s, sd_destination);   
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

/// @brief Telecommand: Instruct the ADCS to format the SD card
/// @param args_str 
///     - No arguments for this command
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_adcs_format_sd(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t status = ADCS_format_sd();
    return status;
}     