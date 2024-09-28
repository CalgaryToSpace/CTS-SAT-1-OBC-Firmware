#include "main.h"

#include "stm32l4xx_hal_i2c.h"
#include "stm32l4xx_hal_def.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"
#include "debug_tools/debug_i2c.h"
#include "debug_tools/debug_uart.h"
#include "log/log.h"
#include <stdint.h>
#include <string.h>

/**
 * @brief This file contains commands for communicating with the antenna deploy system(ADS). The ADS has two microcontrollers which control
 * the deployment of the antennas. It also has two seperate i2c buses which can be used to communicate with the ADS. The combination of 
 * microcontroller (mcu) to communicate to and i2c line to use is specified using a value from the Ant_i2c_bus_mcu enum. 
 * 
 */
 

const uint16_t ANT_ADDR_A = 0x31 << 1; // i2c address of mcu A on ant deploy system
const uint16_t ANT_ADDR_B = 0x32 << 1; // i2c address of mcu B on ant deploy system

const uint8_t timeout = 50;

extern I2C_HandleTypeDef hi2c2; //i2c bus A
extern I2C_HandleTypeDef hi2c3; //i2c bus B

/**
 * @brief Sends a command to the antenna controller.
 * @param i2c_bus_mcu  the i2c_bus and mcu combination to use/transmit to. Either ANT_I2C_BUS_A_MCU_A or ANT_I2C_BUS_B_MCU_B
 * @param cmd_buf Array of bytes to send to the antenna controller
 * @param cmd_len Length of the command buffer
 * @return 0 upon success, 1 if tx_status received HAL_ERROR, 2 if tx_status received HAL_BUSY, 3 if tx_status received HAL_TIMEOUT, 4 if invalid i2c bus/mcu is passed
 */
uint8_t ANT_send_cmd(enum Ant_i2c_bus_mcu i2c_bus_mcu, uint8_t cmd_buf[], uint8_t cmd_len) {
    HAL_StatusTypeDef transmit_status;
    switch(i2c_bus_mcu) {
        case ANT_I2C_BUS_A_MCU_A:
            transmit_status = HAL_I2C_Master_Transmit(&hi2c2, ANT_ADDR_A, cmd_buf, cmd_len, timeout);
            break;
        case ANT_I2C_BUS_B_MCU_B:
            transmit_status = HAL_I2C_Master_Transmit(&hi2c3, ANT_ADDR_B, cmd_buf, cmd_len, timeout);
            break;
        default:
            DEBUG_uart_print_str("invalid i2c bus passed");
            LOG_message(LOG_SYSTEM_ANTENNA_DEPLOY, LOG_SEVERITY_WARNING, LOG_SINK_ALL, "Invalid choice for i2c bus/mcu");
            return 4;
    }

    if (transmit_status == HAL_ERROR) {
        LOG_message(LOG_SYSTEM_ANTENNA_DEPLOY, LOG_SEVERITY_WARNING, LOG_SINK_ALL, "I2C transmit failed: HAL_ERROR");
        return 1;
    } else if(transmit_status == HAL_BUSY) {
        LOG_message(LOG_SYSTEM_ANTENNA_DEPLOY, LOG_SEVERITY_WARNING, LOG_SINK_ALL, "I2C transmit failed: HAL_BUSY");
        return 2;
    } else if(transmit_status == HAL_TIMEOUT) {
        LOG_message(LOG_SYSTEM_ANTENNA_DEPLOY, LOG_SEVERITY_WARNING, LOG_SINK_ALL, "I2C transmit failed: HAL_TIMEOUT");
        return 3;
    }
    return 0;
}

/**
 * @brief Receives a response from the antenna controller.
 * @param i2c_bus_mcu the i2c_bus_mcu to read from. Either ANT_I2C_BUS_A_MCU_A or ANT_I2C_BUS_B_MCU_B  
 * @param rx_buf Array to store the response from the antenna controller
 * @param rx_len Length of the response buffer
 * @return 0 upon success, 1 if read_status received HAL_ERROR
 */
uint8_t ANT_get_response(enum Ant_i2c_bus_mcu i2c_bus_mcu, uint8_t rx_buf[], uint16_t rx_len) {
    HAL_StatusTypeDef read_status = HAL_ERROR;
    
    switch (i2c_bus_mcu) {
    case ANT_I2C_BUS_A_MCU_A:
        read_status = HAL_I2C_Master_Receive(&hi2c2, ANT_ADDR_A, rx_buf, rx_len, timeout);
        break;
    
    case ANT_I2C_BUS_B_MCU_B:
        read_status = HAL_I2C_Master_Receive(&hi2c3, ANT_ADDR_B, rx_buf, rx_len, timeout);
        break;

    default:
        LOG_message(LOG_SYSTEM_ANTENNA_DEPLOY, LOG_SEVERITY_WARNING, LOG_SINK_ALL, "Invalid choice of i2c bus/mcu");
        break;
    }

    if(read_status != HAL_OK) {
        LOG_message(LOG_SYSTEM_ANTENNA_DEPLOY, LOG_SEVERITY_WARNING, LOG_SINK_ALL, "I2C read failed: HAL_ERROR");
        return 1;
    }
    return 0;
}