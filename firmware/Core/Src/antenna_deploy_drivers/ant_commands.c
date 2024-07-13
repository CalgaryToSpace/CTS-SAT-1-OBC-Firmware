#include "main.h"

#include "antenna_deploy_drivers/ant_commands.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"
#include "stm32l4xx_hal_i2c.h"
#include "debug_tools/debug_i2c.h"

uint8_t ANT_CMD_measure_temp() {
    const uint8_t CC = 0xC0;

    const uint8_t cmd_buf[] = {CC};
    const uint8_t cmd_len = sizeof(cmd_buf);

    const uint8_t rx_len = ANT_DEFAULT_RX_LEN_MIN;
    const rx_buf[rx_len];

    const uint8_t comms_err = ANT_send_cmd_get_response(cmd_buf, cmd_len, rx_buf, rx_len);
    return comms_err;
}