#include "main.h"

#include "stm32l4xx_hal_i2c.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"
#include "debug_tools/debug_i2c.h"

#include <stdint.h>
#include <string.h>

const uint16_t ANT_ADDR = 0x31 << 1;
const uint8_t timeout = 50;

extern I2C_HandleTypeDef hi2c2;

/**
 * @brief Sends a command to the antenna controller.
 * 
 * @param cmd_buf Array of bytes to send to the antenna controller
 * @param cmd_len Length of the command buffer
 * @return 0 upon success, 1 if tx_status received HAL_ERROR, 2 if tx_status received HAL_BUSY, 3 if tx_status received HAL_TIMEOUT
 */
uint8_t ANT_send_cmd(uint8_t cmd_buf[], uint8_t cmd_len) {
    const HAL_StatusTypeDef tx_status = HAL_I2C_Master_Transmit(&hi2c2, ANT_ADDR, cmd_buf, cmd_len, timeout);
    if (tx_status == HAL_ERROR) {
        // TODO: Add print statement for debugging
        return 1;
    } else if(tx_status == HAL_BUSY) {
        return 2;
    } else if(tx_status == HAL_TIMEOUT) {
        return 3;
    }

    return 0;
}

/**
 * @brief Receives a response from the antenna controller.
 * 
 * @param rx_buf Array to store the response from the antenna controller
 * @param rx_len Length of the response buffer
 * @return 0 upon success, 4 if rx_status received HAL_ERROR
 */
uint8_t ANT_get_response(uint8_t rx_buf[], uint16_t rx_len) {
    const HAL_StatusTypeDef rx_status = HAL_I2C_Master_Receive(&hi2c2, ANT_ADDR, rx_buf, rx_len, timeout);
        if(rx_status != HAL_OK) {
            // TODO: Add print statement for debugging
            return 4;
        }
    
    return 0;
}