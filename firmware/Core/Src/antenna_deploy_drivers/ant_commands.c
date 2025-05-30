#include "main.h"

#include "antenna_deploy_drivers/ant_internal_drivers.h"
#include "antenna_deploy_drivers/ant_commands.h"
#include "stm32l4xx_hal_i2c.h"
#include "log/log.h"

#include <stdint.h>
#include <stdio.h>

/*-----------------------------COMMAND VARIABLES-----------------------------*/
// All commands in this section come from the "ISIS.ANTS.UM.001" datasheet by ISISpace
static const uint8_t ANT_CMD_RESET = 0xAA;
static const uint8_t ANT_CMD_ARM_ANTENNA_SYSTEM = 0xAD; 
static const uint8_t ANT_CMD_DISARM_ANTENNA_SYSTEM = 0xAC;
static const uint8_t ANT_CMD_DEPLOY_ANTENNA1 = 0xA1; 
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


/// @brief Performs a reset of the antenna deployment systems microcontroller which is specified 
/// @param i2c_bus_mcu specifies which mcu on the antenna deployment system to reset, and which i2c bus to use
/// @return 0 when the antenna deployment system has received the command, >0 otherwise
uint8_t ANT_CMD_reset(enum ANT_i2c_bus_mcu i2c_bus_mcu) {
    const uint8_t CMD_BUF_LEN = 1;
    uint8_t cmd_buf[CMD_BUF_LEN];

    cmd_buf[0] = ANT_CMD_RESET;

    const uint8_t send_status = ANT_send_cmd(i2c_bus_mcu, cmd_buf, CMD_BUF_LEN); 
    return send_status;
}


/// @brief  Arm the antenna deploy system
/// @param i2c_bus_mcu specifies which mcu on the antenna deployment system to arm, and which i2c bus to use
/// @return 0 when the antenna deployment system has received the command, >0 otherwise
/// @note arming one mcu does not arm the other. Each mcu must be armed before it can be used to deploy
uint8_t ANT_CMD_arm_antenna_system(enum ANT_i2c_bus_mcu i2c_bus_mcu) {
    uint8_t cmd_len = 1;
    uint8_t cmd_buf[cmd_len];

    cmd_buf[0] = ANT_CMD_ARM_ANTENNA_SYSTEM;

    const uint8_t comms_err = ANT_send_cmd(i2c_bus_mcu, cmd_buf, cmd_len);
    return comms_err;
}

/// @brief Disarms the antenna deploy system
/// @param i2c_bus_mcu specifies which mcu on the antenna deployment system to disarm, and which i2c bus to use
/// @return 0 when the antenna deployment system has received the command, >0 otherwise
uint8_t ANT_CMD_disarm_antenna_system(enum ANT_i2c_bus_mcu i2c_bus_mcu) {
    const uint8_t CMD_BUF_LEN = 1;
    uint8_t cmd_buf[CMD_BUF_LEN];

    cmd_buf[0] = ANT_CMD_DISARM_ANTENNA_SYSTEM;

    const uint8_t send_status = ANT_send_cmd(i2c_bus_mcu, cmd_buf, CMD_BUF_LEN); 
    return send_status;
}

/// @brief activates the deployment system for the selected antenna for the specified amount of time
/// @param i2c_bus_mcu specifies which mcu on the antenna deployment system to use, and which i2c bus to use
/// @param antenna The antenna number of the antenna to deploy, this is a number between 1-4.
/// @param activation_time_seconds the amount of time the deployment system should be active for in seconds.
/// @return 0 when the antenna deployment system has received the command, >0 otherwise.
uint8_t ANT_CMD_deploy_antenna(enum ANT_i2c_bus_mcu i2c_bus_mcu ,uint8_t antenna, uint8_t activation_time_seconds) {
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
            LOG_message(
                LOG_SYSTEM_ANTENNA_DEPLOY,
                LOG_SEVERITY_ERROR,
                LOG_SINK_ALL, 
                "Invalid choice for antenna: antenna must be between 1-4 inclusive."
            );
            return 1;
    }
    cmd_buf[1] = activation_time_seconds;

    const uint8_t comms_err = ANT_send_cmd(i2c_bus_mcu, cmd_buf, cmd_len);
    return comms_err;
}

/// @brief deploys all antennas one by one sequentially.
/// @param i2c_bus_mcu specifies which mcu on the antenna deployment system to use, and which i2c bus to use
/// @param activation_time_seconds the amount of time the deployment system for each antenna should be active for in seconds
/// @return 0 when the antenna deployment system has received the command, >0 otherwise
uint8_t ANT_CMD_start_automated_sequential_deployment(enum ANT_i2c_bus_mcu i2c_bus_mcu,uint8_t activation_time_seconds) {
    const uint8_t CMD_BUF_LEN = 2;
    uint8_t cmd_buf[CMD_BUF_LEN];

    cmd_buf[0] = ANT_CMD_DEPLOY_ALL_ANTENNAS_SEQ;
    cmd_buf[1] = activation_time_seconds;

    const uint8_t send_status = ANT_send_cmd(i2c_bus_mcu, cmd_buf, CMD_BUF_LEN); 
    return send_status;
}

/// @brief initiates deployment of the selected antenna, ignoring whether the current status of that antenna is deployed
/// @param i2c_bus_mcu specifies which mcu on the antenna deployment system to use, and which i2c bus to use
/// @param antenna the antenna to deploy 
/// @param activation_time_seconds the amount of time the deployment system should be active for in seconds.
/// @return 0 when the antenna deployment system has received the command, >0 otherwise.
uint8_t ANT_CMD_deploy_antenna_with_override(enum ANT_i2c_bus_mcu i2c_bus_mcu,uint8_t antenna, uint8_t activation_time_seconds) {
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
            LOG_message(
                LOG_SYSTEM_ANTENNA_DEPLOY,
                LOG_SEVERITY_ERROR,
                LOG_SINK_ALL, 
                "Invalid choice for antenna: antenna must be between 1-4 inclusive."
            );
            return 1;
    }
    cmd_buf[1] = activation_time_seconds;

    const uint8_t send_status = ANT_send_cmd(i2c_bus_mcu, cmd_buf, CMD_BUF_LEN); 
    return send_status;
}

/// @brief cancels any active attempts to deploy an antenna
/// @param i2c_bus_mcu specifies which mcu on the antenna deployment system to transmit to, and which i2c bus to use
/// @return 0 when the antenna deployment system has received the command, >0 otherwise
uint8_t ANT_CMD_cancel_deployment_system_activation(enum ANT_i2c_bus_mcu i2c_bus_mcu) {
    const uint8_t CMD_BUF_LEN = 1;
    uint8_t cmd_buf[CMD_BUF_LEN];

    cmd_buf[0] = ANT_CMD_CANCEL_DEPLOYMENT;

    const uint8_t send_status = ANT_send_cmd(i2c_bus_mcu, cmd_buf, CMD_BUF_LEN); 
    return send_status;
}

/// @brief Measures the temperature at the antenna controller system.
/// @param i2c_bus_mcu specifies which mcu on the antenna deployment system to transmit to, and which i2c bus to use
/// @param result a pointer to a 16 bit unsigned integer where the temperature measurement is written. Refer to the 
/// "ISIS.ANTS.UM.001" datasheet by ISISpace for information on interpreting this measurement
/// @return 0 when the antenna deployment system has received the command, > 0 otherwise.
uint8_t ANT_CMD_measure_temp(enum ANT_i2c_bus_mcu i2c_bus_mcu, uint16_t *result) {
    const uint8_t cmd_len = 1;
    uint8_t cmd_buf[cmd_len];

    cmd_buf[0] = ANT_CMD_MEASURE_TEMP;

    uint8_t rx_len = 2;
    uint8_t rx_buf[rx_len];

    uint8_t comms_err = ANT_send_cmd(i2c_bus_mcu, cmd_buf, cmd_len);
    if (comms_err == 0) {
        comms_err = ANT_get_response(i2c_bus_mcu, rx_buf, rx_len);
        *result = (rx_buf[1] << 8) | rx_buf[0];
    }
    return comms_err;
}
/// @brief  Converts temperature measurement to centi-degrees celsius
/// @param measurement measurement result from ANT_CMD_measure_temp()
/// @return returns the temperature in centi-degrees celsius
/// @note The formula used is based on the formula found in the data sheet for the LM94022. Derivation can be found on github issue#146.
int16_t ANT_convert_raw_temp_to_cCelsius(uint16_t measurement) {
    return (int16_t)(((float) measurement * -0.29648 + 193.375) * 100);
}


static uint8_t extract_bit(uint8_t byte, uint8_t position) {
    return (byte >> position) & 1u;
}

/// @brief Writes 2 bytes of information representing the deployment status of the antennas to the passed buffer,
///         information on interpreting the response may be found in the ISIS Antenna System user manual. Doc ID: ISIS.ANTS.UM.001 pg. 42
/// @param i2c_bus_mcu specifies which mcu on the antenna deployment system to transmit to, and which i2c bus to use
/// @param response a two byte buffer where the status information is written to.
/// @return 0 when the antenna deployment system has received the command, >0 otherwise
/// @note - Data written to the response struct is only valid if 0 was returned. One should check this before using the response.
/// @note - This command probes the micro controller specified. data from each mcu may differ.
uint8_t ANT_CMD_report_deployment_status(enum ANT_i2c_bus_mcu i2c_bus_mcu, ANT_deployment_status_struct_t *response) {
    const uint8_t CMD_LEN = 1;
    uint8_t cmd_buf[CMD_LEN];

    cmd_buf[0] = ANT_CMD_REPORT_DEPLOYMENT_STATUS;

    uint8_t status = ANT_send_cmd(i2c_bus_mcu, cmd_buf, CMD_LEN); 
    
    if (status == 0) {
        const uint8_t response_size = 2;
        uint8_t raw_bytes[response_size];
        status = ANT_get_response(i2c_bus_mcu, raw_bytes, response_size);

        response->antenna_1_deployed = !extract_bit(raw_bytes[1], 7);       
        response->antenna_1_deployment_time_limit_reached = extract_bit(raw_bytes[1], 6);       
        response->antenna_1_deployment_system_active= extract_bit(raw_bytes[1], 5);       

        response->antenna_2_deployed = !extract_bit(raw_bytes[1], 3);       
        response->antenna_2_deployment_time_limit_reached = extract_bit(raw_bytes[1], 2);       
        response->antenna_2_deployment_system_active= extract_bit(raw_bytes[1], 1);

        response->antenna_3_deployed = !extract_bit(raw_bytes[0], 7);       
        response->antenna_3_deployment_time_limit_reached = extract_bit(raw_bytes[0], 6);       
        response->antenna_3_deployment_system_active= extract_bit(raw_bytes[0], 5);       

        response->antenna_4_deployed = !extract_bit(raw_bytes[0], 3);       
        response->antenna_4_deployment_time_limit_reached = extract_bit(raw_bytes[0], 2);       
        response->antenna_4_deployment_system_active= extract_bit(raw_bytes[0], 1);

        response->ignoring_deployment_switches = extract_bit(raw_bytes[1], 0);
        response->independent_burn = extract_bit(raw_bytes[0], 4);
        response->antenna_system_armed = extract_bit(raw_bytes[0], 0);
    }
    return status;
}
/// @brief writes the number of times deployment has been attempted (for a specified antenna and mcu) in a response buffer.
/// @param i2c_bus_mcu specifies which mcu on the antenna deployment system to check, and which i2c bus to use
/// @param antenna the antenna to check
/// @param response a 1 byte buffer where the count of attempted deployments will be written
/// @return 0 when the antenna deployment system has received the command, >0 otherwise
/// @note data written to the response buffer is only valid if 0 was returned. One should check this before using the response.
uint8_t ANT_CMD_report_antenna_deployment_activation_count(enum ANT_i2c_bus_mcu i2c_bus_mcu, uint8_t antenna, uint8_t *response) {
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
            LOG_message(
                LOG_SYSTEM_ANTENNA_DEPLOY,
                LOG_SEVERITY_ERROR,
                LOG_SINK_ALL, 
                "Invalid choice for antenna: antenna must be between 1-4 inclusive."
            );
            return 1;
    }

    uint8_t status = ANT_send_cmd(i2c_bus_mcu, cmd_buf, CMD_BUFF_SIZE); 
    if(status == 0) {
        const uint8_t response_len = 1;
        status = ANT_get_response(i2c_bus_mcu, response, response_len);
    }
    return status;
}

/// @brief writes the cumulative time (in ms increments) that the deployment system has been active (for a specified antenna and mcu) in a response buffer.
/// @param i2c_bus_mcu specifies which mcu on the antenna deployment system to transmit to, and which i2c bus to use
/// @param antenna the antenna to check. A number between 1-4
/// @param result_ms a 2 byte buffer where the cumulative deployment time (in ms increments) will be written. divide the response by 20 to get seconds.
/// @return 0 when the antenna deployment system has received the command, >0 otherwise
/// @note data written to the result buffer is only valid if 0 was returned. One should check this before using the result.
uint8_t ANT_CMD_get_antenna_deployment_activation_time(enum ANT_i2c_bus_mcu i2c_bus_mcu, uint8_t antenna, uint32_t *result_ms) {
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
            LOG_message(
                LOG_SYSTEM_ANTENNA_DEPLOY,
                LOG_SEVERITY_ERROR,
                LOG_SINK_ALL, 
                "Invalid choice for antenna: antenna must be between 1-4 inclusive."
            );
            return 1;
    }
    uint8_t status = ANT_send_cmd(i2c_bus_mcu, cmd_buf, CMD_BUFF_SIZE); 
    if(status == 0) {
        const uint8_t response_len = 2; 
        uint8_t response_ms[2];
        status = ANT_get_response(i2c_bus_mcu, response_ms, response_len);
        *result_ms = (response_ms[1] << 8) | response_ms[0];
        *result_ms *= 50;
    }
    return status;
}
