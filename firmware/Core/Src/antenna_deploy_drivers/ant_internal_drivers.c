#include "main.h"

#include "stm32l4xx_hal_i2c.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"
#include "debug_tools/debug_i2c.h"
#include "debug_uart.h"
#include <stdint.h>
#include <string.h>

const uint16_t ANT_ADDR = 0x31 << 1;
const uint8_t timeout = 50;

extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c3;

/**
 * @brief Sends a command to the antenna controller.
 * @param i2c_bus  
 * @param cmd_buf Array of bytes to send to the antenna controller
 * @param cmd_len Length of the command buffer
 * @return 0 upon success, 1 if tx_status received HAL_ERROR, 2 if tx_status received HAL_BUSY, 3 if tx_status received HAL_TIMEOUT, 4 if invalid i2c bus is passed
 */
uint8_t ANT_send_cmd(enum Ant_i2c_bus i2c_bus, uint8_t cmd_buf[], uint8_t cmd_len) {
    HAL_StatusTypeDef transmit_status;
    switch(i2c_bus) {
        case ANT_I2C_BUS_A:
            transmit_status = HAL_I2C_Master_Transmit(&hi2c2, ANT_ADDR, cmd_buf, cmd_len, timeout);
            break;
        case ANT_I2C_BUS_B:
            transmit_status = HAL_I2C_Master_Transmit(&hi2c3, ANT_ADDR, cmd_buf, cmd_len, timeout);
            break;
        default:
            DEBUG_uart_print_str("invalid i2c bus passed");
            return 4;
    }

    if (transmit_status == HAL_ERROR) {
        // TODO: Add print statement for debugging
        DEBUG_uart_print_str("i2c transmit failed: HAL_ERROR");
        return 1;
    } else if(transmit_status == HAL_BUSY) {
        DEBUG_uart_print_str("i2c transmit failed: HAL_BUSY");
        return 2;
    } else if(transmit_status == HAL_TIMEOUT) {
        DEBUG_uart_print_str("i2c transmit failed: HAL_TIMEOUT");
        return 3;
    }
    return 0;
}

/**
 * @brief Receives a response from the antenna controller.
 * @param i2c_bus the i2c_bus to read from. Either ANT_I2C_BUS_A or ANT_I2C_BUS_B  
 * @param rx_buf Array to store the response from the antenna controller
 * @param rx_len Length of the response buffer
 * @return 0 upon success, 4 if read_status received HAL_ERROR
 */
uint8_t ANT_get_response(enum Ant_i2c_bus i2c_bus, uint8_t rx_buf[], uint16_t rx_len) {
    HAL_StatusTypeDef read_status = HAL_ERROR;
    
    switch (i2c_bus) {
    case ANT_I2C_BUS_A:
        read_status = HAL_I2C_Master_Receive(&hi2c2, ANT_ADDR, rx_buf, rx_len, timeout);
        break;
    
    case ANT_I2C_BUS_B:
        read_status = HAL_I2C_Master_Receive(&hi2c3, ANT_ADDR, rx_buf, rx_len, timeout);
        break;

    default:
        DEBUG_uart_print_str("invalid choice of i2c bus");
        break;
    }

    if(read_status != HAL_OK) {
        DEBUG_uart_print_str("i2c read failed: HAL_ERROR");
        return 4;
    }
    return 0;
}