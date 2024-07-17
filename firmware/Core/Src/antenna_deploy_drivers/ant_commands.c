#include "main.h"

#include "antenna_deploy_drivers/ant_commands.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"
#include "antenna_deploy_drivers/ant_helper.h"
#include "stm32l4xx_hal_i2c.h"
#include "debug_tools/debug_i2c.h"

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

uint8_t ANT_CMD_arm_antenna_system() {
    const uint8_t CC = 0xAD;

    const uint8_t cmd_buf[] = {CC};
    const uint8_t cmd_len = sizeof(cmd_buf);

    const uint8_t rx_len = 0;
    uint8_t rx_buf[] = NULL;

    const uint8_t comms_err = ANT_send_cmd_get_response(cmd_buf, cmd_len, rx_buf, rx_len);
    return comms_err;
}

uint8_t ANT_CMD_deploy_antenna1(uint8_t activation_time_seconds, uint8_t arg_size) {
    bool ret;
    char buf[arg_size * 2];
    
    const uint8_t CC = 0xA1;
    // FIXME: Find a way to pass activation time as a hex value
    ret = to_hex(buf, sizeof(buf), activation_time_seconds, sizeof(activation_time_seconds));
    const uint8_t CC_param = activation_time_seconds;

    const uint8_t cmd_len = 2;
    uint8_t cmd_buf[cmd_len];

    cmd_buf[0] = CC;
    cmd_buf[1] = CC_param;

    const uint8_t rx_len = 0;
    uint8_t rx_buf[] = NULL;

    const uint8_t comms_err = ANT_send_cmd_get_response(cmd_buf, cmd_len, rx_buf, rx_len);
    return comms_err;
}

uint8_t ANT_CMD_measure_temp() {
    const uint8_t CC = 0xC0;

    const uint8_t cmd_len = 1;
    const uint8_t cmd_buf[cmd_len] = {CC};

    const uint8_t rx_len = 2;
    const rx_buf[rx_len];

    const uint8_t comms_err = ANT_send_cmd_get_response(cmd_buf, cmd_len, rx_buf, rx_len);
    return comms_err;
}