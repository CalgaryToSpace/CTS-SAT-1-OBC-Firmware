/*-----------------------------INCLUDES-----------------------------*/
#include "main.h"

#include "littlefs/flash_driver.h"
#include "debug_tools/debug_uart.h"

#include "config/static_config.h"

/// Timeout duration for HAL_SPI_READ/WRITE operations.
// Note: FLASH_read_data has sporadic timeouts at 5ms; 10ms is a safe bet.
// 512 bytes should take 2ms at 2Mbps.
// TODO: ^ investigate the HAL_SPI_Receive overhead (2ms expected, >5ms observed)
#define FLASH_HAL_TIMEOUT_MS 10 

// The following timeout values are sourced from Section 11.3.1, Table 56: "CFI system interface string"
// and are interpreted using: https://www.infineon.com/dgdl/Infineon-AN98488_Quick_Guide_to_Common_Flash_Interface-ApplicationNotes-v05_00-EN.pdf

/// Duration to wait for the status register to show that Write Enable Latch changed status.
// Assume this value is the same as the typical timeout for a register change.
#define FLASH_LOOP_REGISTER_CHANGE_TIMEOUT_MS 10

/// Duration to wait for the status register to show that Write In Progress bit changed status, for writes.
// "Typical timeout for page program" = 2^(0x09) us = 0.5 ms
// "Max timeout for page program" = 2^(0x02) * typical = 2 ms
// With a safety factor of 5, we get 10 ms.
#define FLASH_LOOP_WRITE_TIMEOUT_MS 10

/// Duration to wait for the status register to show that Write In Progress bit changed status, for erases.
// "Typical timeout for sector erase" = 2^(0x09) ms = 512 ms
// "Max timeout for sector erase" = 2^(0x03) * typical = 4096 ms
// 4 seconds is a very long time, so no safety factor is applied here (to avoid lockups).
#define FLASH_LOOP_SECTOR_ERASE_TIMEOUT_MS 4096

// -----------------------------FLASH DRIVER FUNCTIONS-----------------------------

/// @brief Activates the chip select for the given flash module number.
/// @param chip_number - Index of the chip select line to activate
/// @details Activates the chip select for the given chip number by setting the corresponding GPIO
///     pins LOW. Deactivates all other chip selects by setting their corresponding GPIO pins HIGH.
/// @return None - GPIO writes can't fail
void FLASH_activate_chip_select(uint8_t chip_number)
{
    FLASH_deactivate_chip_select();
    // NOTE: the "reset low" activate action must be AFTER all other pins are "set high"

    if (chip_number == 0) {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_0_GPIO_Port, PIN_MEM_NCS_FLASH_0_Pin, GPIO_PIN_RESET);
    } else if (chip_number == 1) {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_1_GPIO_Port, PIN_MEM_NCS_FLASH_1_Pin, GPIO_PIN_RESET);
    } else if (chip_number == 2) {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_2_GPIO_Port, PIN_MEM_NCS_FLASH_2_Pin, GPIO_PIN_RESET);
    } else if (chip_number == 3) {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_3_GPIO_Port, PIN_MEM_NCS_FLASH_3_Pin, GPIO_PIN_RESET);
    } else if (chip_number == 4) {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_4_GPIO_Port, PIN_MEM_NCS_FLASH_4_Pin, GPIO_PIN_RESET);
    } else if (chip_number == 5) {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_5_GPIO_Port, PIN_MEM_NCS_FLASH_5_Pin, GPIO_PIN_RESET);
    } else if (chip_number == 6) {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_6_GPIO_Port, PIN_MEM_NCS_FLASH_6_Pin, GPIO_PIN_RESET);
    } else if (chip_number == 7) {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_7_GPIO_Port, PIN_MEM_NCS_FLASH_7_Pin, GPIO_PIN_RESET);
    }
    
    // TODO: check if this is how we want to access the FRAM chips
    else if (chip_number == 8) {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FRAM_0_GPIO_Port, PIN_MEM_NCS_FRAM_0_Pin, GPIO_PIN_RESET);
    } else if (chip_number == 9) {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FRAM_1_GPIO_Port, PIN_MEM_NCS_FRAM_1_Pin, GPIO_PIN_RESET);
    }
}

/// @brief Deactivates the chip select for all lines.
/// @return None - GPIO writes can't fail
void FLASH_deactivate_chip_select()
{
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_0_GPIO_Port, PIN_MEM_NCS_FLASH_0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_1_GPIO_Port, PIN_MEM_NCS_FLASH_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_2_GPIO_Port, PIN_MEM_NCS_FLASH_2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_3_GPIO_Port, PIN_MEM_NCS_FLASH_3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_4_GPIO_Port, PIN_MEM_NCS_FLASH_4_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_5_GPIO_Port, PIN_MEM_NCS_FLASH_5_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_6_GPIO_Port, PIN_MEM_NCS_FLASH_6_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_7_GPIO_Port, PIN_MEM_NCS_FLASH_7_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FRAM_0_GPIO_Port, PIN_MEM_NCS_FRAM_0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FRAM_1_GPIO_Port, PIN_MEM_NCS_FRAM_1_Pin, GPIO_PIN_SET);
}

/**
 * @brief Read Status Register and store the values in given buffer
 * @param hspi - Pointer to the SPI HAL handle
 * @param chip_number - the chip select number to activate
 * @param buf - Pointer to a buffer to store SR1 value. Length: 1 byte.
 * @retval FLASH_ERR_OK on success, < 0 on failure
 */
FLASH_error_enum_t FLASH_read_status_register(SPI_HandleTypeDef *hspi, uint8_t chip_number, uint8_t *buf)
{
    // Send GET FEATURES command
    FLASH_activate_chip_select(chip_number);
    const HAL_StatusTypeDef tx_result_1 = HAL_SPI_Transmit(hspi, (uint8_t *)&FLASH_CMD_GET_FEATURES, 1, FLASH_HAL_TIMEOUT_MS);

    // If couldn't send the command, check why
    if (tx_result_1 != HAL_OK) {
        FLASH_deactivate_chip_select();

        if (tx_result_1 == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_TRANSMIT_TIMEOUT;
        }
        
        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }

    // Send the byte address of status register
    const HAL_StatusTypeDef tx_result_2 = HAL_SPI_Transmit(hspi, (uint8_t *)&FLASH_FEAT_STATUS, 1, FLASH_HAL_TIMEOUT_MS);

    // If couldn't send the byte, check why
    if (tx_result_2 != HAL_OK) {
        FLASH_deactivate_chip_select();

        if (tx_result_2 == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_TRANSMIT_TIMEOUT;
        }
        
        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }

    // Receive the Status Register bits
    const HAL_StatusTypeDef rx_result = HAL_SPI_Receive(hspi, (uint8_t *)buf, 1, FLASH_HAL_TIMEOUT_MS);

    // If couldn't receive the byte, check why
    if (rx_result != HAL_OK) {
        FLASH_deactivate_chip_select();

        if (rx_result == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_RECEIVE_TIMEOUT;
        }

        return FLASH_ERR_SPI_RECEIVE_FAILED;
    }

    FLASH_deactivate_chip_select();
    return FLASH_ERR_OK;
}

/**
 * @brief Sends Write Enable Command
 * @param hspi - Pointer to the SPI HAL handle
 * @param chip_number - the chip select number to activate
 * @retval FLASH_ERR_OK on success, < 0 on failure
 */
FLASH_error_enum_t FLASH_write_enable(SPI_HandleTypeDef *hspi, uint8_t chip_number)
{
    // Buffer to store status register value
    uint8_t status_reg_buffer[1] = {0};

    // Send the WRITE ENABLE Command
    FLASH_activate_chip_select(chip_number);
    const HAL_StatusTypeDef tx_result_1 = HAL_SPI_Transmit(hspi, (uint8_t *)&FLASH_CMD_WRITE_ENABLE, 1, FLASH_HAL_TIMEOUT_MS);
    FLASH_deactivate_chip_select();

    // If couldn't send the command, check why
    if (tx_result_1 != HAL_OK) {

        if (tx_result_1 == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_TRANSMIT_TIMEOUT;
        }

        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }

    // Keep looping as long as device is busy (until the Write Enable Latch is active [1])
    const uint32_t start_loop_time_ms = HAL_GetTick();
    while (1)
    {
        // Get status register bits
        const FLASH_error_enum_t read_status_result = FLASH_read_status_register(hspi, chip_number, status_reg_buffer);
        if (read_status_result != FLASH_ERR_OK) {
            FLASH_deactivate_chip_select();
            return read_status_result;
        }

        if ((status_reg_buffer[0] & FLASH_SR1_WRITE_ENABLE_LATCH_MASK) > 0) {
            // Success condition: write enabled.
            return FLASH_ERR_OK;
        }

        // Do this comparison AFTER checking the success condition (for speed, and to avoid timing out on a success).
        if (HAL_GetTick() - start_loop_time_ms > FLASH_LOOP_REGISTER_CHANGE_TIMEOUT_MS) {
            DEBUG_uart_print_str("Flash write enable timeout\n");
            return FLASH_ERR_DEVICE_BUSY_TIMEOUT;
        }

        if (FLASH_enable_hot_path_debug_logs) {
            DEBUG_uart_print_str("DEBUG: status_reg = 0x");
            DEBUG_uart_print_array_hex(status_reg_buffer, 1);
            DEBUG_uart_print_str("\n");
        }
    }

    // Should never be reached:
    return FLASH_ERR_UNKNOWN;
}

/**
 * @brief Sends Write Disable Command
 * @param hspi - Pointer to the SPI HAL handle
 * @param chip_number - the chip select number to activate
 * @retval FLASH_ERR_OK on success, < 0 on failure
 */
FLASH_error_enum_t FLASH_write_disable(SPI_HandleTypeDef *hspi, uint8_t chip_number)
{
    // Buffer to store status register value
    uint8_t status_reg_buffer[1] = {0};

    // Send WRITE DISABLE Command
    FLASH_activate_chip_select(chip_number);
    const HAL_StatusTypeDef tx_status = HAL_SPI_Transmit(hspi, (uint8_t *)&FLASH_CMD_WRITE_DISABLE, 1, FLASH_HAL_TIMEOUT_MS);
    FLASH_deactivate_chip_select();

    // If couldn't send the command, check why
    if (tx_status != HAL_OK) {

        if (tx_status == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_TRANSMIT_TIMEOUT;
        }
        
        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }

    // Keep looping as long as device is busy (until the Write Enable Latch is inactive [0])
    const uint32_t start_loop_time_ms = HAL_GetTick();
    while (1)
    {
        // Get status register bits
        const FLASH_error_enum_t read_status_result = FLASH_read_status_register(hspi, chip_number, status_reg_buffer);
        if (read_status_result != FLASH_ERR_OK) {
            FLASH_deactivate_chip_select();
            return read_status_result;
        }

        if ((status_reg_buffer[0] & FLASH_SR1_WRITE_ENABLE_LATCH_MASK) == 0) {
            // Success condition: write disabled.
            return FLASH_ERR_OK;
        }

        // Do this comparison AFTER checking the success condition (for speed, and to avoid timing out on a success).
        if (HAL_GetTick() - start_loop_time_ms > FLASH_LOOP_REGISTER_CHANGE_TIMEOUT_MS) {
            DEBUG_uart_print_str("Flash write disable timeout\n");
            return FLASH_ERR_DEVICE_BUSY_TIMEOUT;
        }

        if (FLASH_enable_hot_path_debug_logs) {
            DEBUG_uart_print_str("DEBUG: status_reg = 0x");
            DEBUG_uart_print_array_hex(status_reg_buffer, 1);
            DEBUG_uart_print_str("\n");
        }
    }

    // Should never be reached:
    return FLASH_ERR_UNKNOWN;
}

/**
 * @brief Sends Block Erase Command
 * @param hspi - Pointer to the SPI HAL handle
 * @param chip_number - the chip select number to activate
 * @param addr - block address that is to be erased
 * @retval FLASH_ERR_OK on success, < 0 on failure
 */
FLASH_error_enum_t FLASH_erase(SPI_HandleTypeDef *hspi, uint8_t chip_number, lfs_block_t addr)
{
    // Split address into its 4 bytes
    uint8_t addr_bytes[4] = {(addr >> 24) & 0xFF, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF};

    // Send WRITE ENABLE Command
    const FLASH_error_enum_t wren_result = FLASH_write_enable(hspi, chip_number);
    if (wren_result != FLASH_ERR_OK) {
        FLASH_deactivate_chip_select();
        return wren_result;
    }

    // Send BLOCK ERASE Command
    FLASH_activate_chip_select(chip_number);
    const HAL_StatusTypeDef tx_result_1 = HAL_SPI_Transmit(hspi, (uint8_t *)&FLASH_CMD_BLOCK_ERASE, 1, FLASH_HAL_TIMEOUT_MS);
    
    // If couldn't send the command, check why
    if (tx_result_1 != HAL_OK) {
        FLASH_deactivate_chip_select();

        if (tx_result_1 == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_TRANSMIT_TIMEOUT;
        }
        
        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }

    // Send the address bytes
    const HAL_StatusTypeDef tx_result_2 = HAL_SPI_Transmit(hspi, (uint8_t *)addr_bytes, 4, FLASH_HAL_TIMEOUT_MS);

    // If couldn't send the bytes, check why
    if (tx_result_2 != HAL_OK) {
        FLASH_deactivate_chip_select();

        if (tx_result_2 == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_TRANSMIT_TIMEOUT;
        }

        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }
    FLASH_deactivate_chip_select();

    // Buffer to store status register value
    uint8_t status_reg_buffer[1] = {0};

    // Keep looping as long as device is busy (until the Operation In Progress bit is active [1])
    const uint32_t start_loop_time_ms = HAL_GetTick();
    while (1)
    {
        // Get status register bits
        const FLASH_error_enum_t read_status_result = FLASH_read_status_register(hspi, chip_number, status_reg_buffer);
        if (read_status_result != FLASH_ERR_OK) {
            FLASH_deactivate_chip_select();
            return read_status_result;
        }

        if ((status_reg_buffer[0] & FLASH_SR1_ERASE_ERROR_MASK) > 0) {
            // Flash module returned "erase error" via the status register.
            DEBUG_uart_print_str("Flash erase error\n");
            return FLASH_ERR_STATUS_REG_ERROR;
        }

        if ((status_reg_buffer[0] & FLASH_SR1_WRITE_IN_PROGRESS_MASK) == 0) {
            // Success condition: write in progress has completed.
            return FLASH_ERR_OK;
        }

        // Do this comparison AFTER checking the success condition (for speed, and to avoid timing out on a success).
        if (HAL_GetTick() - start_loop_time_ms > FLASH_LOOP_SECTOR_ERASE_TIMEOUT_MS) {
            DEBUG_uart_print_str("Flash erase timeout\n");
            return FLASH_ERR_DEVICE_BUSY_TIMEOUT;
        }

        if (FLASH_enable_hot_path_debug_logs) {
            DEBUG_uart_print_str("DEBUG: status_reg = 0x");
            DEBUG_uart_print_array_hex(status_reg_buffer, 1);
            DEBUG_uart_print_str("\n");
        }
    }

    // Should never be reached:
    return FLASH_ERR_UNKNOWN;
}

/**
 * @brief Sends Page Program Command
 * @param hspi - Pointer to the SPI HAL handle
 * @param chip_number - the chip select number to activate
 * @param addr - block address that is to be written
 * @param packet_buffer - Pointer to buffer containing data to write
 * @param packet_buffer_len - integer that indicates the size of the data to write
 * @retval FLASH_ERR_OK on success, < 0 on failure
 */
FLASH_error_enum_t FLASH_write_data(SPI_HandleTypeDef *hspi, uint8_t chip_number, lfs_block_t addr, uint8_t *packet_buffer, lfs_size_t packet_buffer_len)
{
    // Split main address into its 4 bytes
    uint8_t addr_bytes[4] = {(addr >> 24) & 0xFF, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF};
    
    // Define the address where data will be stored in cache register (First 4 bits are dummy bits)
    // TODO: Is it fine to always have this at 0?
    uint16_t cache_addr = 0x0000;
    uint8_t cache_addr_bytes[2] = {(cache_addr >> 8) & 0xFF, cache_addr & 0xFF};

    // Send WRITE ENABLE Command
    const FLASH_error_enum_t wren_result = FLASH_write_enable(hspi, chip_number);
    if (wren_result != FLASH_ERR_OK) {
        FLASH_deactivate_chip_select();
        return wren_result;
    }

    // Send PROGAM LOAD Command
    FLASH_activate_chip_select(chip_number);
    const uint8_t tx_result_1 = HAL_SPI_Transmit(hspi, (uint8_t *)&FLASH_CMD_PROGRAM_LOAD, 1, FLASH_HAL_TIMEOUT_MS);

    // If couldn't send the command, check why
    if (tx_result_1 != HAL_OK) {
        FLASH_deactivate_chip_select();

        if (tx_result_1 == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_TRANSMIT_TIMEOUT;
        }

        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }

    // Send Cache Register address
    const uint8_t tx_result_2 = HAL_SPI_Transmit(hspi, (uint8_t *)cache_addr_bytes, 2, FLASH_HAL_TIMEOUT_MS);

    // If couldn't send the bytes, check why
    if (tx_result_2 != HAL_OK) {
        FLASH_deactivate_chip_select();

        if (tx_result_2 == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_TRANSMIT_TIMEOUT;
        }

        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }

    // Send the data bytes
    const uint8_t tx_result_3 = HAL_SPI_Transmit(hspi, (uint8_t *)packet_buffer, packet_buffer_len, FLASH_HAL_TIMEOUT_MS);

    // If couldn't send the bytes, check why
    if (tx_result_3 != HAL_OK) {
        FLASH_deactivate_chip_select();

        if (tx_result_3 == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_TRANSMIT_TIMEOUT;
        }

        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }

    // TODO: If Write doesn't work as intended or not all data bits are stored
    // do the status register check loop here before deactivating chip select
    FLASH_deactivate_chip_select();

    // Send PROGAM EXECUTE Command
    FLASH_activate_chip_select(chip_number);
    const uint8_t tx_result_4 = HAL_SPI_Transmit(hspi, (uint8_t *)&FLASH_CMD_PROGRAM_EXEC, 1, FLASH_HAL_TIMEOUT_MS);

    // If couldn't send the command, check why
    if (tx_result_4 != HAL_OK) {
        FLASH_deactivate_chip_select();

        if (tx_result_4 == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_TRANSMIT_TIMEOUT;
        }

        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }

    // Send address bytes
    const uint8_t tx_result_5 = HAL_SPI_Transmit(hspi, (uint8_t *)addr_bytes, 4, FLASH_HAL_TIMEOUT_MS);

    // If couldn't send the bytes, check why
    if (tx_result_5 != HAL_OK) {
        FLASH_deactivate_chip_select();

        if (tx_result_5 == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_TRANSMIT_TIMEOUT;
        }

        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }
    FLASH_deactivate_chip_select();

    // Buffer to store status register value
    uint8_t status_reg_buffer[1] = {0};

    // Keep looping as long as device is busy (until the Operation In Progress bit is active [1])
    const uint32_t start_loop_time_ms = HAL_GetTick();
    while (1)
    {
        const FLASH_error_enum_t read_status_result = FLASH_read_status_register(hspi, chip_number, status_reg_buffer);
        if (read_status_result != FLASH_ERR_OK) {
            FLASH_deactivate_chip_select();
            return read_status_result;
        }

        if ((status_reg_buffer[0] & FLASH_SR1_PROGRAMMING_ERROR_MASK) > 0) {
            // Flash module returned "programming error" via the status register.
            DEBUG_uart_print_str("Flash programming error\n");
            return FLASH_ERR_STATUS_REG_ERROR;
        }

        if ((status_reg_buffer[0] & FLASH_SR1_WRITE_IN_PROGRESS_MASK) == 0) {
            // Success condition: write in progress has completed.
            return FLASH_ERR_OK;
        }

        // Do this comparison AFTER checking the success condition (for speed, and to avoid timing out on a success).
        if (HAL_GetTick() - start_loop_time_ms > FLASH_LOOP_WRITE_TIMEOUT_MS) {
            DEBUG_uart_print_str("Flash write timeout\n");
            return FLASH_ERR_DEVICE_BUSY_TIMEOUT;
        }

        if (FLASH_enable_hot_path_debug_logs) {
            DEBUG_uart_print_str("DEBUG: status_reg = 0x");
            DEBUG_uart_print_array_hex(status_reg_buffer, 1);
            DEBUG_uart_print_str("\n");
        }
    }

    // Should never be reached:
    return FLASH_ERR_UNKNOWN;
}

/**
 * @brief Sends Page Read Command
 * @param hspi - Pointer to the SPI HAL handle
 * @param chip_number - The chip select number to activate
 * @param addr - Address to be read
 * @param rx_buffer - A buffer where the read data will be stored
 * @param rx_buffer_len - Integer that indicates the capacity of `rx_buffer`
 * @retval FLASH_ERR_OK on success, < 0 on failure
 */
FLASH_error_enum_t FLASH_read_data(SPI_HandleTypeDef *hspi, uint8_t chip_number, lfs_block_t addr, uint8_t *rx_buffer, lfs_size_t rx_buffer_len)
{
    uint8_t read_addr_bytes[4] = {(addr >> 24) & 0xFF, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF};

    // Define the address where data will be read from in cache register (First 4 bits are dummy bits)
    // TODO: Is it fine to always have this at 0?
    uint16_t cache_addr = 0x0000;
    uint8_t cache_addr_bytes[2] = {(cache_addr >> 8) & 0xFF, cache_addr & 0xFF};

    // Send PAGE READ command
    FLASH_activate_chip_select(chip_number);
    const HAL_StatusTypeDef tx_result_1 = HAL_SPI_Transmit(hspi, (uint8_t *) &FLASH_CMD_PAGE_READ, 1, FLASH_HAL_TIMEOUT_MS);

    // If couldn't send the command, check why
    if (tx_result_1 != HAL_OK) {
        FLASH_deactivate_chip_select();

        if (tx_result_1 == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_TRANSMIT_TIMEOUT;
        }

        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }

    // FIXME: Datasheet Page 16 talks about only giving 8-bit dummy values and 16-bit address, not sure how that works
    // Send Address bytes 
    const HAL_StatusTypeDef tx_result_2 = HAL_SPI_Transmit(hspi, (uint8_t *) read_addr_bytes, 4, FLASH_HAL_TIMEOUT_MS);

    // If couldn't send the bytes, check why
    if (tx_result_2 != HAL_OK) {
        FLASH_deactivate_chip_select();

        if (tx_result_2 == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_TRANSMIT_TIMEOUT;
        }

        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }

    // Set Chip Select HIGH
    FLASH_deactivate_chip_select();

    // Buffer to store status register value
    uint8_t status_reg_buffer[1] = {0};

    // Keep looping as long as device is busy (until the Operation In Progress bit is active [1])
    const uint32_t start_loop_time_ms = HAL_GetTick();
    while (1)
    {
        const FLASH_error_enum_t read_status_result = FLASH_read_status_register(hspi, chip_number, status_reg_buffer);
        if (read_status_result != FLASH_ERR_OK) {
            FLASH_deactivate_chip_select();
            return read_status_result;
        }

        if ((status_reg_buffer[0] & FLASH_SR1_WRITE_IN_PROGRESS_MASK) == 0) {
            // Success condition: write in progress has completed.
            return FLASH_ERR_OK;
        }

        // Do this comparison AFTER checking the success condition (for speed, and to avoid timing out on a success).
        if (HAL_GetTick() - start_loop_time_ms > FLASH_LOOP_WRITE_TIMEOUT_MS) {
            DEBUG_uart_print_str("Flash read timeout\n");
            return FLASH_ERR_DEVICE_BUSY_TIMEOUT;
        }

        if (FLASH_enable_hot_path_debug_logs) {
            DEBUG_uart_print_str("DEBUG: status_reg = 0x");
            DEBUG_uart_print_array_hex(status_reg_buffer, 1);
            DEBUG_uart_print_str("\n");
        }
    }
    FLASH_deactivate_chip_select();

    // Send READ FROM CACHE command
    FLASH_activate_chip_select(chip_number);
    const HAL_StatusTypeDef tx_result_3 = HAL_SPI_Transmit(hspi, (uint8_t *) &FLASH_CMD_READ_FROM_CACHE, 1, FLASH_HAL_TIMEOUT_MS);

    // If couldn't send the command, check why
    if (tx_result_3 != HAL_OK) {
        FLASH_deactivate_chip_select();

        if (tx_result_3 == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_TRANSMIT_TIMEOUT;
        }

        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }

    // Send cache address bytes 
    const HAL_StatusTypeDef tx_result_4 = HAL_SPI_Transmit(hspi, (uint8_t *) cache_addr_bytes, 2, FLASH_HAL_TIMEOUT_MS);

    // If couldn't send the bytes, check why
    if (tx_result_4 != HAL_OK) {
        FLASH_deactivate_chip_select();

        if (tx_result_4 == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_TRANSMIT_TIMEOUT;
        }

        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }

    // Send cache address bytes again just as 2 dummy bytes to use 2 8-bit clock cycles
    const HAL_StatusTypeDef tx_result_5 = HAL_SPI_Transmit(hspi, (uint8_t *) cache_addr_bytes, 2, FLASH_HAL_TIMEOUT_MS);

    // If couldn't send the bytes, check why
    if (tx_result_5 != HAL_OK) {
        FLASH_deactivate_chip_select();

        if (tx_result_5 == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_TRANSMIT_TIMEOUT;
        }

        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }

    // Receive the data into the buffer
    const HAL_StatusTypeDef rx_result_1 = HAL_SPI_Receive(hspi, (uint8_t *)rx_buffer, rx_buffer_len, FLASH_HAL_TIMEOUT_MS);

    // If couldn't receive data, check why
    if (rx_result_1 != HAL_OK) {
        FLASH_deactivate_chip_select();

        if (rx_result_1 == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_RECEIVE_TIMEOUT;
        }

        return FLASH_ERR_SPI_RECEIVE_FAILED;
    }

    // Set Chip Select HIGH
    FLASH_deactivate_chip_select();

    // TODO: Are there any other errors that can occur while reading?
    return FLASH_ERR_OK;
}

























/**
 * @brief Checks if the FLASH chip is reachable by checking it's ID
 * @param hspi - Pointer to the SPI HAL handle
 * @param chip_number - The chip select number to activate
 * @retval FLASH_ERR_OK on success, < 0 on failure
 */
FLASH_error_enum_t FLASH_is_reachable(SPI_HandleTypeDef *hspi, uint8_t chip_number)
{
    // TODO: confirm if this works with the CS2 logical chip on each physical FLASH chip;
    // ^ Seems as though it only works for CS1.

    uint8_t tx_buffer[1] = {FLASH_CMD_READ_ID};
    uint8_t rx_buffer[5];
    memset(rx_buffer, 0, 5);

    // Send READ ID Command
    FLASH_activate_chip_select(chip_number);
    const HAL_StatusTypeDef tx_result_1 = HAL_SPI_Transmit(hspi, tx_buffer, 1, FLASH_HAL_TIMEOUT_MS);

    // If couldn't send the command, check why
    if (tx_result_1 != HAL_OK) {
        FLASH_deactivate_chip_select();

        if (tx_result_1 == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_TRANSMIT_TIMEOUT;
        }

        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }

    // Send the command again just as a dummy byte
    const HAL_StatusTypeDef tx_result_2 = HAL_SPI_Transmit(hspi, tx_buffer, 1, FLASH_HAL_TIMEOUT_MS);

    // If couldn't send the command, check why
    if (tx_result_2 != HAL_OK) {
        FLASH_deactivate_chip_select();

        if (tx_result_2 == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_TRANSMIT_TIMEOUT;
        }

        return FLASH_ERR_SPI_TRANSMIT_FAILED;
    }

    // Receive the response
    const HAL_StatusTypeDef rx_result = HAL_SPI_Receive(hspi, rx_buffer, 5, FLASH_HAL_TIMEOUT_MS);

    // If couldn't receive, checky why
    if (rx_result != HAL_OK) {
        FLASH_deactivate_chip_select();

        if (rx_result == HAL_TIMEOUT) {
            return FLASH_ERR_SPI_RECEIVE_TIMEOUT;
        }

        return FLASH_ERR_SPI_RECEIVE_FAILED;
    }

    FLASH_deactivate_chip_select();

    // Check the received ID (modify according to the expected ID of your memory module)
    // rx_buffer[0] is the manufacturer ID, rx_buffer[1] is the memory type,
    // and rx_buffer[2] is the memory capacity (not checked, as we have a few different capacities).
    // rx_buffer[2] is 0x20=512 for 512 Mib (mebibits)
    // TODO: maybe check the capacity as well here, esp. in deployment
    uint8_t are_bytes_correct = 0;
    if (rx_buffer[0] == 0x2C && rx_buffer[1] == 0x14) {
        DEBUG_uart_print_str("SUCCESS: FLASH_is_reachable received IDs: ");
        are_bytes_correct = 1;
    } else {
        DEBUG_uart_print_str("ERROR: FLASH_is_reachable received IDs: ");
        are_bytes_correct = 0;
    }

    DEBUG_uart_print_array_hex(rx_buffer, 5);
    DEBUG_uart_print_str("\n");

    if (!are_bytes_correct) {
        // error: IDs don't match
        return FLASH_ERR_UNKNOWN;
    }
    return FLASH_ERR_OK; // success
}
