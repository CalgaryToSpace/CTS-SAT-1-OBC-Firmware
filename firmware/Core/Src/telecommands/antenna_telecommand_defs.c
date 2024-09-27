#include "main.h"

#include "telecommands/antenna_telecommand_defs.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"
#include "antenna_deploy_drivers/ant_commands.h"
#include "telecommands/telecommand_args_helpers.h"
#include "debug_uart.h"
#include "log/log.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "inttypes.h"

/// @brief Resets the antenna deployment system's microcontroller
/// @param args_str
/// - Arg 0: specifies which mcu on the antenna deployment system to trasmit to, and which i2c bus to use 
/// @return 0 on success, > 0 otherwise
uint8_t TCMDEXEC_ant_reset(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) 
{
    char i2c_bus_str [2];
    if(TCMD_extract_string_arg(args_str, 0, i2c_bus_str, 2) != 0 ) {
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Failed reading i2c bus");
        return 1;
    }

    enum Ant_i2c_bus_mcu i2c_bus_mcu;
    switch (i2c_bus_str[0])
    {
    case 'A':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_A;
        break;
    case 'B':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_B;
        break; 
    default:
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Invalid choice for i2c bus");
        return 1;
        break;
    }

    const int status = ANT_CMD_reset(i2c_bus_mcu);
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: %d", status);
        return status;
    }
    snprintf(response_output_buf, response_output_buf_len, "Success reseting antenna deployment system");
    return 0;
}

/// @brief Telecommand: Arm the antenna deploy system
/// @param args_str 
/// - Arg 0: specifies which mcu on the antenna deployment system to trasmit to, and which i2c bus to use 
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ant_arm_antenna_system(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    char i2c_bus_str [2];
    if(TCMD_extract_string_arg(args_str, 0, i2c_bus_str, 2) != 0 ) {
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Failed reading i2c bus");
        return 1;
    }

    enum Ant_i2c_bus_mcu i2c_bus_mcu;
    switch (i2c_bus_str[0])
    {
    case 'A':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_A;
        break;
    case 'B':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_B;
        break; 
    default:
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Invalid choice for i2c bus");
        return 1;
        break;
    }
    
    const uint8_t comms_err = ANT_CMD_arm_antenna_system(i2c_bus_mcu);
    if (comms_err != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: %d", comms_err);
        return comms_err;
    }

    snprintf(response_output_buf, response_output_buf_len, "Success");
    return 0;
}

/// @brief Disarms the antenna system
/// @param args_str 
/// - Arg 0: specifies which mcu on the antenna deployment system to trasmit to, and which i2c bus to use 
/// @return 0 on success, 0 > otherwise
uint8_t TCMDEXEC_ant_disarm_antenna_system(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    char i2c_bus_str [2];
    if(TCMD_extract_string_arg(args_str, 0, i2c_bus_str, 2) != 0 ) {
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Failed reading i2c bus");
        return 1;
    }

    enum Ant_i2c_bus_mcu i2c_bus_mcu;
    switch (i2c_bus_str[0])
    {
    case 'A':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_A;
        break;
    case 'B':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_B;
        break; 
    default:
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Invalid choice for i2c bus");
        return 1;
        break;
    }

    const uint8_t status = ANT_CMD_disarm_antenna_system(i2c_bus_mcu);
    if (status != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error disarming antenna system: %d", status);
        return status;
    }

    snprintf(response_output_buf, response_output_buf_len, "Success, antenna system disarmed");
    return 0;
}

/// @brief  Telecommand: Initiates deployment of the selected antenna
/// @param args_str 
/// - Arg 0: specifies which mcu on the antenna deployment system to trasmit to, and which i2c bus to use 
/// - Arg 1: antenna number. between 1-4
/// - Arg 2: Activation time in seconds
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ant_deploy_antenna(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    //TODO: error checking for bad i2c bus arguments
    char i2c_bus_str [2];
    if(TCMD_extract_string_arg(args_str, 0, i2c_bus_str, 2) != 0 ) {
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Failed reading i2c bus");
        return 1;
    }

    enum Ant_i2c_bus_mcu i2c_bus_mcu;
    switch (i2c_bus_str[0])
    {
    case 'A':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_A;
        break;
    case 'B':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_B;
        break; 
    default:
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Invalid choice for i2c bus");
        return 1;
    }

    uint64_t antenna;
    const uint8_t parse_antenna_result = TCMD_extract_uint64_arg(args_str, strlen(args_str),  1, &antenna);
    if (parse_antenna_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing antenna arg: %d", parse_antenna_result);
        return 3;
    }
    if (antenna < 1 || antenna > 4) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error: Antenna number provided is not between 1-4 inclusive."
        );
        return 4;
    }

    uint64_t arg_activation_time;
    const uint8_t parse_activation_time_result = TCMD_extract_uint64_arg(args_str, strlen(args_str),  2, &arg_activation_time);
    if (parse_activation_time_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing activation time arg: %d", parse_activation_time_result);
        return 3;
    }
    if(arg_activation_time > 255) {
        // error: number is too large
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error: Activation time must be less than 255");
        return 4;
    }

    const uint8_t comms_err = ANT_CMD_deploy_antenna(i2c_bus_mcu, (uint8_t)antenna, (uint8_t)arg_activation_time);
    if (comms_err != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: %d", comms_err);
        return comms_err;
    }
    
    snprintf(response_output_buf, response_output_buf_len, "Success: antenna %d deployment in progress.",(uint8_t) antenna);
    return 0;
}

/// @brief begins deployment of all antennas, one by one.
/// @param args_str 
/// - Arg 0: specifies which mcu on the antenna deployment system to trasmit to, and which i2c bus to use 
/// - Arg 1: Activation time in seconds
/// @return returns 0 on success, > 0 otherwise
uint8_t TCMDEXEC_ant_start_automated_antenna_deployment(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) 
{
    char i2c_bus_str [2];
    if(TCMD_extract_string_arg(args_str, 0, i2c_bus_str, 2) != 0 ) {
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Failed reading i2c bus");
        return 1;
    }

    enum Ant_i2c_bus_mcu i2c_bus_mcu;
    switch (i2c_bus_str[0])
    {
    case 'A':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_A;
        break;
    case 'B':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_B;
        break; 
    default:
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Invalid choice for i2c bus");
        return 1;
        break;
    }

    uint64_t activation_time;
    const uint8_t parse_activation_time_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &activation_time);
    if (parse_activation_time_result != 0) {
        snprintf( response_output_buf, response_output_buf_len, "Error parsing argument: %d", parse_activation_time_result);
        return 3;
    }
    if (activation_time > 255) {
        snprintf( response_output_buf, response_output_buf_len, "Error: activation time must be less than 256");
        return 4;
    }

    const uint8_t status = ANT_CMD_start_automated_sequential_deployment(i2c_bus_mcu,(uint8_t)activation_time);
    if (status != 0) {
        snprintf( response_output_buf, response_output_buf_len, "Error: %d", status);
        return status;
    }
    snprintf( response_output_buf, response_output_buf_len, "Success: automated sequential antenna deployment initiated.");
    return 0;
}

/// @brief  Telecommand: Initiates deployment of the selected antenna, ignoring whether the antennas current status is deployed. 
/// @param args_str 
/// - Arg 0: specifies which mcu on the antenna deployment system to trasmit to, and which i2c bus to use 
/// - Arg 1: antenna number. between 1-4
/// - Arg 2: Activation time in seconds
/// @return 0 on successful communication, >0 on communications error 
uint8_t TCMDEXEC_ant_deploy_antenna_with_override(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    char i2c_bus_str [2];
    if(TCMD_extract_string_arg(args_str, 0, i2c_bus_str, 2) != 0 ) {
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Failed reading i2c bus");
        return 1;
    }

    enum Ant_i2c_bus_mcu i2c_bus_mcu;
    switch (i2c_bus_str[0])
    {
    case 'A':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_A;
        break;
    case 'B':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_B;
        break; 
    default:
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Invalid choice for i2c bus");
        return 1;
        break;
    }

    uint64_t antenna;
    const uint8_t parse_antenna_result = TCMD_extract_uint64_arg(args_str, strlen(args_str),  1, &antenna);
    if (parse_antenna_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing antenna arg: %d", parse_antenna_result);
        return 3;
    }
    if (antenna < 1 || antenna > 4) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error: Antenna number provided is not between 1-4 inclusive."
        );
        return 4;
    }

    uint64_t arg_activation_time;
    const uint8_t parse_activation_time_result = TCMD_extract_uint64_arg(args_str, strlen(args_str),  2, &arg_activation_time);
    if (parse_activation_time_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing activation time arg: %d", parse_activation_time_result);
        return 3;
    }
    if(arg_activation_time > 255) {
        // error: number is too large
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error: Activation time must be less than 255");
        return 4;
    }

    const uint8_t comms_err = ANT_CMD_deploy_antenna_with_override(i2c_bus_mcu,(uint8_t)antenna, (uint8_t)arg_activation_time);
    if (comms_err != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: %d", comms_err);
        return comms_err;
    }
    
    snprintf(response_output_buf, response_output_buf_len, "Success: antenna %d deployment in progress.",(uint8_t)antenna);
    return 0;
}

/// @brief Cancels any active attempts to deploy an antenna
/// @param args_str 
/// - Arg 0: specifies which mcu on the antenna deployment system to trasmit to, and which i2c bus to use 
/// @return 0 on successful communication, > 0 on communications error
uint8_t TCMDEXEC_ant_cancel_deployment_system_activation(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    char i2c_bus_str [2];
    if(TCMD_extract_string_arg(args_str, 0, i2c_bus_str, 2) != 0 ) {
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Failed reading i2c bus");
        return 1;
    }

    enum Ant_i2c_bus_mcu i2c_bus_mcu;
    switch (i2c_bus_str[0])
    {
    case 'A':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_A;
        break;
    case 'B':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_B;
        break; 
    default:
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Invalid choice for i2c bus");
        return 1;
        break;
    }
    
    const uint8_t comms_err = ANT_CMD_cancel_deployment_system_activation(i2c_bus_mcu);
    if (comms_err != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: failed to cancel deployment");
        return comms_err;
    }

    snprintf(response_output_buf, response_output_buf_len, "Success: antenna deployment canceled.");
    return 0;
}


/// @brief Prints the deployment status of all antennas
/// @param args_str 
/// - Arg 0: specifies which mcu on the antenna deployment system to trasmit to, and which i2c bus to use 
/// @return 0 on successful communication, > 0 on communications error
uint8_t TCMDEXEC_ant_report_deployment_status(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    char i2c_bus_str [2];
    if(TCMD_extract_string_arg(args_str, 0, i2c_bus_str, 2) != 0 ) {
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Failed reading i2c bus");
        return 1;
    }

    enum Ant_i2c_bus_mcu i2c_bus_mcu;
    switch (i2c_bus_str[0])
    {
    case 'A':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_A;
        break;
    case 'B':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_B;
        break; 
    default:
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Invalid choice for i2c bus");
        return 1;
        break;
    }
    
    struct Antenna_deployment_status response;
    const uint8_t comms_err = ANT_CMD_report_deployment_status(i2c_bus_mcu, &response);
    if (comms_err != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: failed to report status.");
        return comms_err;
    }

    snprintf(
        response_output_buf, 
        response_output_buf_len, 
        "Success, deployment status ----- \n Antenna 1: %u \n Antenna 2: %u \n Antenna 3: %u \n Antenna 4: %u \n",
         response.antenna_1_deployed,
         response.antenna_2_deployed,
         response.antenna_3_deployed,
         response.antenna_4_deployed
        );
        //remove the newline characters at the end of the strings to get a proper JSON string
    snprintf(response_output_buf, response_output_buf_len,
        "{"
        "\"antenna_1_deployed\": %d,"   "\n"
        "\"antenna_1_deployment_time_limit_reached\": %d," "\n"
        "\"antenna_1_deployment_system_active\": %d," "\n"
        "\"antenna_2_deployed\": %d," "\n"
        "\"antenna_2_deployment_time_limit_reached\": %d," "\n"
        "\"antenna_2_deployment_system_active\": %d," "\n"
        "\"antenna_3_deployed\": %d," "\n"
        "\"antenna_3_deployment_time_limit_reached\": %d," "\n"
        "\"antenna_3_deployment_system_active\": %d," "\n"
        "\"antenna_4_deployed\": %d," "\n"
        "\"antenna_4_deployment_time_limit_reached\": %d," "\n"
        "\"antenna_4_deployment_system_active\": %d," "\n"
        "\"independent_burn\": %d," "\n"
        "\"ignoring_deployment_switches\": %d," "\n"
        "\"antenna_system_armed\": %d" "\n"
        "}",
        response.antenna_1_deployed,
        response.antenna_1_deployment_time_limit_reached,
        response.antenna_1_deployment_system_active,
        response.antenna_2_deployed,
        response.antenna_2_deployment_time_limit_reached,
        response.antenna_2_deployment_system_active,
        response.antenna_3_deployed,
        response.antenna_3_deployment_time_limit_reached,
        response.antenna_3_deployment_system_active,
        response.antenna_4_deployed,
        response.antenna_4_deployment_time_limit_reached,
        response.antenna_4_deployment_system_active,
        response.independent_burn,
        response.ignoring_deployment_switches,
        response.antenna_system_armed
    );
    return 0;
}

/// @brief Prints the number of times deployment was attempted on the selected antenna
/// @param args_str 
/// - Arg 0: specifies which mcu on the antenna deployment system to trasmit to, and which i2c bus to use 
/// - Arg 1: the antenna to check, between 1-4 
/// @return 0 on successful communication, > 0 on communications error
uint8_t TCMDEXEC_ant_report_antenna_deployment_activation_count(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    char i2c_bus_str [2];
    if(TCMD_extract_string_arg(args_str, 0, i2c_bus_str, 2) != 0 ) {
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Failed reading i2c bus");
        return 1;
    }

    enum Ant_i2c_bus_mcu i2c_bus_mcu;
    switch (i2c_bus_str[0])
    {
    case 'A':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_A;
        break;
    case 'B':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_B;
        break; 
    default:
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Invalid choice for i2c bus");
        return 1;
        break;
    }

    uint64_t antenna;
    const uint8_t parse_antenna_result = TCMD_extract_uint64_arg(args_str, strlen(args_str),  1, &antenna);
    if (parse_antenna_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing antenna arg: %d", parse_antenna_result);
        return 3;
    }
    if (antenna < 1 || antenna > 4) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error: Antenna number provided is not between 1-4 inclusive."
        );
        return 4;
    }

    uint8_t response[1];
    const uint8_t comms_status = ANT_CMD_report_antenna_deployment_activation_count(i2c_bus_mcu, (uint8_t)antenna, response);
    if (comms_status != 0) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error reporting antenna deployment count: %d", comms_status);
        return comms_status;
    }

    snprintf(
        response_output_buf,
        response_output_buf_len,
        "Success, antenna %u deployment count: %u", (uint8_t)antenna, response[0]);
    return 0;
}

/// @brief Prints amount of time the deployment system has been active for for the selected antenna
/// @param args_str 
/// - Arg 0: specifies which mcu on the antenna deployment system to trasmit to, and which i2c bus to use 
/// - Arg 1: the antenna to check, between 1-4 
/// @return 0 on successful communication, > 0 on communications error
uint8_t TCMDEXEC_ant_report_antenna_deployment_activation_time(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    char i2c_bus_str [2];
    if(TCMD_extract_string_arg(args_str, 0, i2c_bus_str, 2) != 0 ) {
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Failed reading i2c bus");
        return 1;
    }

    enum Ant_i2c_bus_mcu i2c_bus_mcu;
    switch (i2c_bus_str[0])
    {
    case 'A':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_A;
        break;
    case 'B':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_B;
        break; 
    default:
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Invalid choice for i2c bus");
        return 1;
        break;
    }

    uint64_t antenna;
    const uint8_t parse_antenna_result = TCMD_extract_uint64_arg(args_str, strlen(args_str),  1, &antenna);
    if (parse_antenna_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing antenna arg: %d", parse_antenna_result);
        return 3;
    }
    if (antenna < 1 || antenna > 4) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error: Antenna number provided is not between 1-4 inclusive."
        );
        return 4;
    }

    uint16_t response;
    const uint8_t comms_status = ANT_CMD_report_antenna_deployment_activation_time(i2c_bus_mcu,(uint8_t)antenna, &response);
    if (comms_status != 0) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error reporting antenna deployment time: %d", comms_status);
        return comms_status;
    }
    snprintf(
        response_output_buf,
        response_output_buf_len,
        "Success, antenna %u deployment time: %d", (uint8_t)antenna, response);
    return 0;
}

/// @brief Telecommand: Measures the temperature of the antenna controller in centi-degrees celsius
/// @param args_str
/// - Arg 0: specifies which mcu on the antenna deployment system to trasmit to, and which i2c bus to use 
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_ant_measure_temp(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    char i2c_bus_str [2];
    if(TCMD_extract_string_arg(args_str, 0, i2c_bus_str, 2) != 0 ) {
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Failed reading i2c bus");
        return 1;
    }

    enum Ant_i2c_bus_mcu i2c_bus_mcu;
    switch (i2c_bus_str[0])
    {
    case 'A':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_A;
        break;
    case 'B':
        i2c_bus_mcu = ANT_I2C_BUS_A_MCU_B;
        break; 
    default:
        LOG_message(LOG_SYSTEM_TELECOMMAND,LOG_SEVERITY_WARNING,LOG_SINK_ALL, "Invalid choice for i2c bus");
        return 1;
        break;
    }

    uint16_t measurement;
    const uint8_t comms_err = ANT_CMD_measure_temp(i2c_bus_mcu,&measurement);
    if (comms_err != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: %d", comms_err);
        return comms_err;
    }
    //TODO: remove the newline in between the strings to get a JSON string
    snprintf(
        response_output_buf, 
        response_output_buf_len, 
        "{\"raw_temp\":%d," "\n" "\"temp_cC\":%d}", 
        measurement,
        ANT_convert_raw_temp_to_cCelsius(measurement)
    );
    return 0;
}
