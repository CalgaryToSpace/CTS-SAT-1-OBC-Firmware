#include "main.h"

#include "antenna_deploy_drivers/ant_commands.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"
#include "stm32l4xx_hal_i2c.h"
#include "debug_tools/debug_uart.h"

#include <stdint.h>
#include <stdio.h>

/*-----------------------------COMMAND VARIABLES-----------------------------*/
// All commands in this section refer to the "ISIS.ANTS.UM.001" datasheet by ISISpace
static const uint8_t ANT_CMD_RESET = 0xAA;
static const uint8_t ANT_CMD_ARM_ANTENNA_SYSTEM = 0xAD; // Arm the antenna deploy system
static const uint8_t ANT_CMD_DISARM_ANTENNA_SYSTEM = 0xAC;
static const uint8_t ANT_CMD_DEPLOY_ANTENNA1 = 0xA1; // Deploy antenna 1
static const uint8_t ANT_CMD_DEPLOY_ANTENNA2 = 0xA2;
static const uint8_t ANT_CMD_DEPLOY_ANTENNA3 = 0xA3;
static const uint8_t ANT_CMD_DEPLOY_ANTENNA4 = 0xA4;
static const uint8_t ANT_CMD_DEPLOY_ALL_ANTENNAS_SEQ = 0xA5;
static const uint8_t ANT_CMD_DEPLOY_ANTENNA1_OVERRIDE = 0xBA;
static const uint8_t ANT_CMD_DEPLOY_ANTENNA2_OVERRIDE = 0xBB;
static const uint8_t ANT_CMD_DEPLOY_ANTENNA3_OVERRIDE = 0xBC;
static const uint8_t ANT_CMD_DEPLOY_ANTENNA4_OVERRIDE = 0xBD;
static const uint8_t ANT_CMD_CANCEL_DEPLOYMENT = 0xA9;
static const uint8_t ANT_CMD_MEASURE_TEMP = 0xC0; // Measure the temperature of the antenna controller system
static const uint8_t ANT_CMD_REPORT_DEPLOYMENT_STATUS = 0xC3;
static const uint8_t ANT_CMD_REPORT_ANT1_DEPLOYMENT_COUNT = 0xB0;
static const uint8_t ANT_CMD_REPORT_ANT2_DEPLOYMENT_COUNT = 0xB1;
static const uint8_t ANT_CMD_REPORT_ANT3_DEPLOYMENT_COUNT = 0xB2;
static const uint8_t ANT_CMD_REPORT_ANT4_DEPLOYMENT_COUNT = 0xB3;
static const uint8_t ANT_CMD_REPORT_ANT1_DEPLOYMENT_SYS_ACTIVATION_TIME= 0xB4;
static const uint8_t ANT_CMD_REPORT_ANT2_DEPLOYMENT_SYS_ACTIVATION_TIME= 0xB5;
static const uint8_t ANT_CMD_REPORT_ANT3_DEPLOYMENT_SYS_ACTIVATION_TIME= 0xB6;
static const uint8_t ANT_CMD_REPORT_ANT4_DEPLOYMENT_SYS_ACTIVATION_TIME= 0xB7;
/*-----------------------------COMMAND VARIABLES-----------------------------*/


/// @brief Performs a reset of the antenna deployment systems microcontroller 
/// @return 0 when the antenna deployment system has received the command, >0 otherwise
uint8_t ANT_CMD_reset() {
    const uint8_t CMD_BUF_LEN = 1;
    uint8_t cmd_buf[CMD_BUF_LEN];

    cmd_buf[0] = ANT_CMD_RESET;

    const uint8_t send_status = ANT_send_cmd(cmd_buf, CMD_BUF_LEN); 
    return send_status;
}


/// @brief  Arm the antenna deploy system
/// @return 0 when the antenna deployment system has received the command, >0 otherwise
uint8_t ANT_CMD_arm_antenna_system() {
    uint8_t cmd_len = 1;
    uint8_t cmd_buf[cmd_len];

    cmd_buf[0] = ANT_CMD_ARM_ANTENNA_SYSTEM;

    const uint8_t comms_err = ANT_send_cmd(cmd_buf, cmd_len);
    return comms_err;
}

/// @brief Disarms the antenna deploy system
/// @return 0 when the antenna deployment system has received the command, >0 otherwise
uint8_t ANT_CMD_disarm_antenna_system() {
    const uint8_t CMD_BUF_LEN = 1;
    uint8_t cmd_buf[CMD_BUF_LEN];

    cmd_buf[0] = ANT_CMD_DISARM_ANTENNA_SYSTEM;

    const uint8_t send_status = ANT_send_cmd(cmd_buf, CMD_BUF_LEN); 
    return send_status;
}

/// @brief activates the deployment system for the selected antenna for the specified amount of time
/// @param antenna The antenna number of the antenna to deploy, this is a number between 1-4.
/// @param[in] activation_time_seconds the amount of time the deployment system should be active for in seconds.
/// @return 0 when the antenna deployment system has received the command, >0 otherwise.
uint8_t ANT_CMD_deploy_antenna(uint8_t antenna, uint8_t activation_time_seconds) {
    const uint8_t cmd_len = 2;
    uint8_t cmd_buf[cmd_len];

    switch (antenna) {
        case 1:
            cmd_buf[0] = ANT_CMD_DEPLOY_ANTENNA1;
            break;
        case 2:
            cmd_buf[0] = ANT_CMD_DEPLOY_ANTENNA2;
            break;
        case 3:
            cmd_buf[0] = ANT_CMD_DEPLOY_ANTENNA3;
            break;
        case 4:
            cmd_buf[0] = ANT_CMD_DEPLOY_ANTENNA4;
            break;
        default:
            DEBUG_uart_print_str("Invalid choice for antenna: antenna must be between 1-4 inclusive.");
            return 1;
    }
    cmd_buf[1] = activation_time_seconds;

    const uint8_t comms_err = ANT_send_cmd(cmd_buf, cmd_len);
    return comms_err;
}

/// @brief deploys all antennas one by one sequentially.
/// @param activation_time_seconds the amount of time the deployment system for each antenna should be active for in seconds.
/// @return 0 when the antenna deployment system has received the command, >0 otherwise
uint8_t ANT_CMD_start_automated_sequential_deployment(uint8_t activation_time_seconds) {
    const uint8_t CMD_BUF_LEN = 2;
    uint8_t cmd_buf[CMD_BUF_LEN];

    cmd_buf[0] = ANT_CMD_DEPLOY_ALL_ANTENNAS_SEQ;
    cmd_buf[1] = activation_time_seconds;

    const uint8_t send_status = ANT_send_cmd(cmd_buf, CMD_BUF_LEN); 
    return send_status;
}

/// @brief initiates deployment of the selected antenna, ignoring whether the current status of that antenna is deployed
/// @param antenna the antenna to deploy 
/// @param activation_time_seconds the amount of time the deployment system should be active for in seconds.
/// @return 0 when the antenna deployment system has received the command, >0 otherwise.
uint8_t ANT_CMD_deploy_antenna_with_override(uint8_t antenna, uint8_t activation_time_seconds) {
    const uint8_t CMD_BUF_LEN = 2;
    uint8_t cmd_buf[CMD_BUF_LEN];

    switch (antenna) {
        case 1:
            cmd_buf[0] = ANT_CMD_DEPLOY_ANTENNA1_OVERRIDE;
            break;
        case 2:
            cmd_buf[0] = ANT_CMD_DEPLOY_ANTENNA2_OVERRIDE;
            break;
        case 3:
            cmd_buf[0] = ANT_CMD_DEPLOY_ANTENNA3_OVERRIDE;
            break;
        case 4:
            cmd_buf[0] = ANT_CMD_DEPLOY_ANTENNA4_OVERRIDE;
            break;
        default:
            DEBUG_uart_print_str("Invalid choice for antenna: antenna must be between 1-4 inclusive.");
            return 1;
    }
    cmd_buf[1] = activation_time_seconds;

    const uint8_t send_status = ANT_send_cmd(cmd_buf, CMD_BUF_LEN); 
    return send_status;
}

/// @brief cancels any active attempts to deploy an antenna
/// @return 0 when the antenna deployment system has received the command, >0 otherwise
uint8_t ANT_CMD_cancel_deployment_system_activation() {
    const uint8_t CMD_BUF_LEN = 1;
    uint8_t cmd_buf[CMD_BUF_LEN];

    cmd_buf[0] = ANT_CMD_CANCEL_DEPLOYMENT;

    const uint8_t send_status = ANT_send_cmd(cmd_buf, CMD_BUF_LEN); 
    return send_status;
}

/// @brief Measures the temperature at the antenna controller system.
/// @param result a pointer to a 16 bit unsigned integer where the temperature measurement is written. Refer to the 
/// "ISIS.ANTS.UM.001" datasheet by ISISpace for information on interpreting this measurement
/// @return 0 when the antenna deployment system has received the command, > 0 otherwise.
uint8_t ANT_CMD_measure_temp(uint16_t *result) {
    const uint8_t cmd_len = 1;
    uint8_t cmd_buf[cmd_len];

    cmd_buf[0] = ANT_CMD_MEASURE_TEMP;

    uint8_t rx_len = 2;
    uint8_t rx_buf[rx_len];

    uint8_t comms_err = ANT_send_cmd(cmd_buf, cmd_len);
    if (comms_err == 0) {
        comms_err = ANT_get_response(rx_buf, rx_len);
        *result = (rx_buf[1] << 8) | rx_buf[0];
    }
    return comms_err;
}
/// @brief  Converts temperature measurement to centi-degrees celsius
/// @param measurement measurement result from ANT_CMD_measure_temp()
/// @return returns the temperature in centi-degrees celsius
/// @note The formula used is based on the formula found in the data sheet for the LM94022. Derivation can be found on github issue#146.
int16_t ANT_CMD_convert_temp_measurement_to_centi_degree_celsius(uint16_t measurement) {
    return (int16_t)(((float) measurement * -0.29648 + 193.375) * 100);
}


static uint8_t extract_bit(uint8_t byte, uint8_t position) {return byte >> position & 1u;}

/// @brief writes 2 bytes of information representing the deployment status of the antennas to the passed buffer,
///         information on interpreting the response may be found in the ISIS Antenna System user manual. Doc ID: ISIS.ANTS.UM.001 pg. 42
/// @param response a two byte buffer where the status information is written to.
/// @return 0 when the antenna deployment system has received the command, >0 otherwise
/// @note data written to the response struct is only valid if 0 was returned. One should check this before using the response.
uint8_t ANT_CMD_report_deployment_status(struct Antenna_deployment_status *response) {
    const uint8_t CMD_LEN  = 1;
    uint8_t cmd_buf[CMD_LEN];

    cmd_buf[0] = ANT_CMD_REPORT_DEPLOYMENT_STATUS;

    uint8_t status = ANT_send_cmd(cmd_buf, CMD_LEN); 
    
    if (status == 0) {
        const uint8_t response_size = 2;
        uint8_t raw_bytes[response_size];
        status = ANT_get_response(raw_bytes, response_size);

        response->antenna_1_deployed= !extract_bit(raw_bytes[1], 7);       
        response->antenna_1_deployment_time_limit_reached= extract_bit(raw_bytes[1], 6);       
        response->antenna_1_deployment_system_active= extract_bit(raw_bytes[1], 5);       

        response->antenna_2_deployed= !extract_bit(raw_bytes[1], 3);       
        response->antenna_2_deployment_time_limit_reached= extract_bit(raw_bytes[1], 2);       
        response->antenna_2_deployment_system_active= extract_bit(raw_bytes[1], 1);

        response->antenna_3_deployed= !extract_bit(raw_bytes[0], 7);       
        response->antenna_3_deployment_time_limit_reached= extract_bit(raw_bytes[0], 6);       
        response->antenna_3_deployment_system_active= extract_bit(raw_bytes[0], 5);       

        response->antenna_4_deployed= !extract_bit(raw_bytes[0], 3);       
        response->antenna_4_deployment_time_limit_reached= extract_bit(raw_bytes[0], 2);       
        response->antenna_4_deployment_system_active= extract_bit(raw_bytes[0], 1);

        response->ignoring_deployment_switches = extract_bit(raw_bytes[1], 0);
        response->independent_burn = extract_bit(raw_bytes[0], 4);
        response->antenna_system_armed = extract_bit(raw_bytes[0], 0);
    }
    return status;
}
/// @brief writes the number of times deployment has been attempted (for a specified antenna) in a response buffer.
/// @param antenna the antenna to check
/// @param response a 1 byte buffer where the count of attempted deployments will be written
/// @return 0 when the antenna deployment system has received the command, >0 otherwise
/// @note data written to the response buffer is only valid if 0 was returned. One should check this before using the response.
uint8_t ANT_CMD_report_antenna_deployment_activation_count(uint8_t antenna, uint8_t *response) {
    const uint8_t CMD_BUFF_SIZE = 1;
    uint8_t cmd_buf[CMD_BUFF_SIZE];

    switch (antenna) {
        case 1:
            cmd_buf[0] = ANT_CMD_REPORT_ANT1_DEPLOYMENT_COUNT;
            break;
        case 2:
            cmd_buf[0] = ANT_CMD_REPORT_ANT2_DEPLOYMENT_COUNT;
            break;
        case 3:
            cmd_buf[0] = ANT_CMD_REPORT_ANT3_DEPLOYMENT_COUNT;
            break;
        case 4:
            cmd_buf[0] = ANT_CMD_REPORT_ANT4_DEPLOYMENT_COUNT;
            break;
        default:
            DEBUG_uart_print_str("Invalid choice for antenna: antenna must be between 1-4 inclusive.");
            return 1;
    }

    uint8_t status = ANT_send_cmd(cmd_buf, CMD_BUFF_SIZE); 
    if(status == 0) {
        const uint8_t response_len = 1;
        status = ANT_get_response(response, response_len);
    }
    return status;
}

/// @brief writes the cumulative time (in 50ms increments) that the deployment system has been active (for a specified antenna) in a response buffer.
/// @param antenna the antenna to check. A number between 1-4
/// @param response a 2 byte buffer where the cumulative deployment time (in 50ms increments) will be written. divide the response by 20 to get seconds.
/// @return 0 when the antenna deployment system has received the command, >0 otherwise
/// @note data written to the result buffer is only valid if 0 was returned. One should check this before using the result.
uint8_t ANT_CMD_report_antenna_deployment_activation_time(uint8_t antenna, uint16_t *result) {
    const uint8_t CMD_BUFF_SIZE = 1;
    uint8_t cmd_buf[CMD_BUFF_SIZE];

    switch (antenna) {
        case 1:
            cmd_buf[0] = ANT_CMD_REPORT_ANT1_DEPLOYMENT_SYS_ACTIVATION_TIME;
            break;
        case 2:
            cmd_buf[0] = ANT_CMD_REPORT_ANT2_DEPLOYMENT_SYS_ACTIVATION_TIME;
            break;
        case 3:
            cmd_buf[0] = ANT_CMD_REPORT_ANT3_DEPLOYMENT_SYS_ACTIVATION_TIME;
            break;
        case 4:
            cmd_buf[0] = ANT_CMD_REPORT_ANT4_DEPLOYMENT_SYS_ACTIVATION_TIME;
            break;
        default:
            DEBUG_uart_print_str("Invalid choice for antenna: antenna must be between 1-4 inclusive.");
            return 1;
    }

    uint8_t status = ANT_send_cmd(cmd_buf, CMD_BUFF_SIZE); 
    if(status == 0) {
        const uint8_t response_len = 2; 
        uint8_t response[2];
        status = ANT_get_response(response, response_len);

        *result = (response[1] << 8) | response[0];
    }
    return status;
}
