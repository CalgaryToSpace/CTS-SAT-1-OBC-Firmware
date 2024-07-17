#include "main.h"

#include "stm32l4xx_hal_i2c.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"
#include "debug_tools/debug_i2c.h"

#include <stdint.h>
#include <string.h>

static const uint8_t ANT_ADDR = 0x31 << 1;

extern I2C_HandleTypeDef hi2c2;

// Might not need to include cmd buf array and length depending on the parameters of the command
// If no parameters, you should only need to send the command byte (8 bits)

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
    const HAL_StatusTypeDef tx_status = HAL_I2C_Master_Transmit(&hi2c2, ANT_ADDR, cmd_buf, cmd_len, 1000);
    if (tx_status != HAL_OK) {
        // TODO: Add print statement for debugging
        return 1;
    }

    if (rx_len != 0) {
        const HAL_StatusTypeDef rx_status = HAL_I2C_Master_Receive(&hi2c2, ANT_ADDR, rx_buf, rx_len, 1000);
        if(rx_status != HAL_OK) {
            // TODO: Add print statement for debugging
            return 2;
        }


    }
    return 0;
}