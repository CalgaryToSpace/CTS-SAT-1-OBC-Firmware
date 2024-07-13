#include "main.h"

#include "stm32l4xx_hal_i2c.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"
#include "debug_tools/debug_i2c.h"

#include <stdint.h>
#include <string.h>

extern I2C_HandleTypeDef *I2C_ant_port_handle;

/**
 * @brief Sends a command to the antenna controller and waits for a response.
 * 
 * @param cmd_buf Array of bytes to send to the antenna controller
 * @param cmd_len Length of the command buffer
 * @param rx_buf Array to store the response from the antenna controller
 * @param rx_len Length of the response buffer
 * @retval ANT status
 */
uint8_t ANT_send_cmd_get_response(
    const uint8_t cmd_buf[], uint8_t cmd_len,
    uint8_t rx_buf[], uint16_t rx_len
    ) {
        // ASSERT: 

}