#include "main.h"

#include "antenna_deploy_drivers/ant_commands.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"
#include "stm32l4xx_hal_i2c.h"
#include "debug_tools/debug_uart.h"

#include <stdint.h>
#include <stdio.h>

/// @brief Performs a reset of the antenna deployment systems microcontroller 
/// @return 0 if successful, > 0 if error occurred
uint8_t ANT_CMD_reset() {
    const uint8_t CMD_BUF_LEN = 1;
    uint8_t cmd_buf[CMD_BUF_LEN];

    cmd_buf[0] = ANT_CMD_RESET;

    const uint8_t send_status = ANT_send_cmd(cmd_buf, CMD_BUF_LEN); 
    return send_status;
}

//TODO: Some state should be kept when the antenna is armed so that deploy antenna fails when disarmed 
/// @brief  Arm the antenna deploy system
/// @return 0 if successful, >0 if error occurred
uint8_t ANT_CMD_arm_antenna_system() {
    uint8_t cmd_len = 1;
    uint8_t cmd_buf[cmd_len];

    cmd_buf[0] = ANT_CMD_ARM_ANTENNA_SYSTEM;

    const uint8_t comms_err = ANT_send_cmd(cmd_buf, cmd_len);
    return comms_err;
}
/// @brief Disarms the antenna deploy system
/// @return 0 if successful, > 0 if error occurred
uint8_t ANT_CMD_disarm_antenna_system() {
    const uint8_t CMD_BUF_LEN = 1;
    uint8_t cmd_buf[CMD_BUF_LEN];

    cmd_buf[0] = ANT_CMD_DISARM_ANTENNA_SYSTEM;

    const uint8_t send_status = ANT_send_cmd(cmd_buf, CMD_BUF_LEN); 
    return send_status;
}

/// @brief Deploys antenna 
/// @param antenna The antenna number of the antenna to deploy, this is a number between 1-4.
/// @param[in] activation_time_seconds Activation time in seconds
/// @return 0 if successful, >0 if error occurred
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
/// @brief deploys all antennas one by one automatically
/// @param activation_time_seconds the maximum activation time for each deployment system in seconds.  
/// @return 0 if successful, > 0 otherwise. 
uint8_t ANT_CMD_start_automated_sequential_deployment(uint8_t activation_time_seconds) {
    const uint8_t CMD_BUF_LEN = 2;
    uint8_t cmd_buf[CMD_BUF_LEN];

    cmd_buf[0] = ANT_CMD_DISARM_ANTENNA_SYSTEM;
    cmd_buf[1] = activation_time_seconds;

    const uint8_t send_status = ANT_send_cmd(cmd_buf, CMD_BUF_LEN); 
    return send_status;
}

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

uint8_t ANT_CMD_cancel_deployment_system_activation() {
    const uint8_t CMD_BUF_LEN = 1;
    uint8_t cmd_buf[CMD_BUF_LEN];

    cmd_buf[0] = ANT_CMD_CANCEL_DEPLOYMENT;
    

    const uint8_t send_status = ANT_send_cmd(cmd_buf, CMD_BUF_LEN); 
    return send_status;
}

/// @brief Measure the temperature of the antenna controller system
/// @return 0 if successful, >0 if error occurred
/// @note Upon success, the temperature is printed to the debug UART
uint8_t ANT_CMD_measure_temp() {
    const uint8_t cmd_len = 1;
    uint8_t cmd_buf[cmd_len];

    cmd_buf[0] = ANT_CMD_MEASURE_TEMP;

    uint8_t rx_len = 2;
    uint8_t rx_buf[rx_len];

    uint8_t comms_err = ANT_send_cmd(cmd_buf, cmd_len);
    if (comms_err == 0) {
        comms_err = ANT_get_response(rx_buf, rx_len);
    }

    DEBUG_uart_print_str("Received raw bytes: ");
    DEBUG_uart_print_array_hex(rx_buf, rx_len);
    DEBUG_uart_print_str("\n");
    return comms_err;
}
/// @brief writes 2 bytes of information representing the deployment status of the antennas to the passed buffer,
///         information on interpreting the response may be found in the ISIS Antenna System user manual. Doc ID: ISIS.ANTS.UM.001 pg. 42
/// @param response a two byte buffer where the status information is written to.
/// @return 0 on success, > 0 on failure.
uint8_t ANT_CMD_report_deployment_status(uint8_t response[2]) {
    const uint8_t CMD_LEN  = 1;
    uint8_t cmd_buf[CMD_LEN];

    cmd_buf[0] = ANT_CMD_REPORT_DEPLOYMENT_STATUS;

    uint8_t status = ANT_send_cmd(cmd_buf, CMD_LEN); 
    
    if (status == 0) {
        const uint8_t response_size = 2;
        status = ANT_get_response(response, response_size);
    }
    return status;
}
/// @brief writes the number of times deployment has been attempted (for a specified antenna) in a response buffer.
/// @param antenna the antenna to check
/// @param response a 1 byte buffer where the count of attempted deployments will be written
/// @return 0 on success, > 0 on failure
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
/// @param response a 2 byte buffer where the cumulative deployment time (in 50ms increments) will be written
/// @return 0 on success, > 0 on failure
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

        *result = response[0] + (response[1] << 8);
        // time is reported in increments of 50ms, divide by 20 to get seconds
        *result = *result / 20;
    }
    return status;
}