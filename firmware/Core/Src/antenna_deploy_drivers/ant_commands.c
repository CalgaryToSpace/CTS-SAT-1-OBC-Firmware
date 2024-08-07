#include "main.h"

#include "antenna_deploy_drivers/ant_commands.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"
#include "stm32l4xx_hal_i2c.h"
#include "debug_tools/debug_uart.h"

#include <stdint.h>
#include <stdio.h>

/// @brief  Arm the antenna deploy system
/// @return 0 if successful, >0 if error occurred
uint8_t ANT_CMD_arm_antenna_system() {
    uint8_t CC = 0xAD;

    uint8_t cmd_len = 1;
    uint8_t cmd_buf[cmd_len];

    cmd_buf[0] = CC;

    const uint8_t comms_err = ANT_send_cmd(cmd_buf, cmd_len);
    return comms_err;
}

/// @brief Deploys antenna 1
/// @param[in] activation_time_seconds Activation time in seconds
/// @return 0 if successful, >0 if error occurred
uint8_t ANT_CMD_deploy_antenna1(uint8_t activation_time_seconds) {
    uint8_t CC = 0xA1;
    uint8_t CC_param = activation_time_seconds;

    uint8_t cmd_len = 2;
    uint8_t cmd_buf[cmd_len];

    cmd_buf[0] = CC;
    cmd_buf[1] = CC_param;

    const uint8_t comms_err = ANT_send_cmd(cmd_buf, cmd_len);
    return comms_err;
}

/// @brief Measure the temperature of the antenna controller system
/// @return 0 if successful, >0 if error occurred
/// @note Upon success, the temperature is printed to the debug UART
uint8_t ANT_CMD_measure_temp() {
    uint8_t CC = 0xC0;

    uint8_t cmd_len = 1;
    uint8_t cmd_buf[cmd_len];

    cmd_buf[0] = CC;

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