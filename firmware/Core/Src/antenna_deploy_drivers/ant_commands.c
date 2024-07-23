#include "main.h"

#include "antenna_deploy_drivers/ant_commands.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"
#include "stm32l4xx_hal_i2c.h"
#include "debug_tools/debug_i2c.h"

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

/// @brief  Arm the antenna deploy system
/// @return 0 if successful, >0 if error occurred
uint8_t ANT_CMD_arm_antenna_system() {
    uint8_t CC = 0xAD;

    uint8_t cmd_len = 1;
    uint8_t cmd_buf[cmd_len];

    cmd_buf[0] = CC;

    uint8_t rx_len = 0;
    uint8_t rx_buf[] = {0};

    const uint8_t comms_err = ANT_send_cmd_get_response(cmd_buf, cmd_len, rx_buf, rx_len);
    return comms_err;
}

/// @brief Deploys antenna 1
/// @param activation_time_seconds Activation time in seconds
/// @return 0 if successful, >0 if error occurred
uint8_t ANT_CMD_deploy_antenna1(uint8_t activation_time_seconds) {
    uint8_t CC = 0xA1;
    uint8_t CC_param = activation_time_seconds;

    uint8_t cmd_len = 2;
    uint8_t cmd_buf[cmd_len];

    cmd_buf[0] = CC;
    cmd_buf[1] = CC_param;

    uint8_t rx_len = 0;
    uint8_t rx_buf[] = {0};

    const uint8_t comms_err = ANT_send_cmd_get_response(cmd_buf, cmd_len, rx_buf, rx_len);
    return comms_err;
}

/// @brief Measure the temperature of the antenna controller system
/// @return 0 if successful, >0 if error occurred
uint8_t ANT_CMD_measure_temp() {
    uint8_t CC = 0xC0;

    uint8_t cmd_len = 1;
    uint8_t cmd_buf[cmd_len];

    cmd_buf[0] = CC;

    uint8_t rx_len = 2;
    uint8_t rx_buf[rx_len];

    const uint8_t comms_err = ANT_send_cmd_get_response(cmd_buf, cmd_len, rx_buf, rx_len);
    return comms_err;
}