#include "main.h"

#include "antenna_deploy_drivers/ant_commands.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"
#include "stm32l4xx_hal_i2c.h"
#include "debug_tools/debug_i2c.h"

uint8_t ANT_CMD_measure_temp() {
    const uint8_t CC = 0xC0;

    const uint8_t cmd_len = 1;
    const uint8_t cmd_buf[cmd_len] = {CC};

    const uint8_t rx_len = 2;
    const rx_buf[rx_len];

    const uint8_t comms_err = ANT_send_cmd_get_response(cmd_buf, cmd_len, rx_buf, rx_len);
    return comms_err;
}

uint8_t ANT_CMD_deploy_antenna1() {
    const uint8_t CC = 0xA1;
    const uint8_t CC_param = 0x05; // Test with 5 second max activation time

    const uint8_t cmd_len = 2;
    uint8_t cmd_buf[cmd_len];

    cmd_buf[0] = CC;
    cmd_buf[1] = CC_param;

    const uint8_t rx_len = 0;
    uint8_t rx_buf[] = NULL;

    const uint8_t comms_err = ANT_send_cmd_get_response(cmd_buf, cmd_len, rx_buf, rx_len);
    return comms_err;
}

uint8_t ANT_CMD_arm_antenna_system() {
    const uint8_t CC = 0xAD;

    const uint8_t cmd_buf[] = {CC};
    const uint8_t cmd_len = sizeof(cmd_buf);

    const uint8_t rx_len = 0;
    uint8_t rx_buf[] = NULL;

    const uint8_t comms_err = ANT_send_cmd_get_response(cmd_buf, cmd_len, rx_buf, rx_len);
    return comms_err;
}