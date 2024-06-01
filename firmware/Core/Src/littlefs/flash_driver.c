
/*-----------------------------INCLUDES-----------------------------*/
#include "main.h"

#include "littlefs/flash_driver.h"
#include "debug_tools/debug_uart.h"

// TODO: check on timeout, maybe decrease a lot
#define FLASH_TIMEOUT_MS 100

// -----------------------------FLASH DRIVER FUNCTIONS-----------------------------

/// @brief Activates the chip select for the given flash module number.
/// @param chip_number - Index of the chip select line to activate
/// @details Activates the chip select for the given chip number by setting the corresponding GPIO
///     pins LOW. Deactivates all other chip selects by setting their corresponding GPIO pins HIGH.
/// @return None - GPIO writes can't fail
void FLASH_activate_chip_select(uint8_t chip_number)
{
    // TODO: add the rest of the CS pins, refactor
    if (chip_number == 0) {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_0_GPIO_Port, PIN_MEM_NCS_FLASH_0_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_1_GPIO_Port, PIN_MEM_NCS_FLASH_1_Pin, GPIO_PIN_SET);
    } else if (chip_number == 1) {
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_0_GPIO_Port, PIN_MEM_NCS_FLASH_0_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_1_GPIO_Port, PIN_MEM_NCS_FLASH_1_Pin, GPIO_PIN_RESET);
    }
}

/// @brief Deactivates the chip select for all lines.
/// @return None - GPIO writes can't fail
void FLASH_deactivate_chip_select()
{
    // TODO: add the rest of the CS pins, refactor
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_0_GPIO_Port, PIN_MEM_NCS_FLASH_0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PIN_MEM_NCS_FLASH_1_GPIO_Port, PIN_MEM_NCS_FLASH_1_Pin, GPIO_PIN_SET);
}

// FIXME: needs a length argument if writing to an array pointer
// FIXME: all the following functions must propogate their errors

/**
 * @brief Read Status Register and store the values in given buffer
 * @param hspi - Pointer to the SPI HAL handle
 * @param chip_number - the chip select number to activate
 * @param buf - Pointer to a buffer to store SR1 values
 * @retval None
 */
uint8_t FLASH_read_status_register(SPI_HandleTypeDef *hspi, uint8_t chip_number, uint8_t *buf)
{
    FLASH_activate_chip_select(chip_number);
    const HAL_StatusTypeDef tx_result = HAL_SPI_Transmit(hspi, (uint8_t *)&FLASH_RDSR1, 1, FLASH_TIMEOUT_MS);
    if (tx_result != HAL_OK) {
        FLASH_deactivate_chip_select();
        return 1;
    }

    const HAL_StatusTypeDef rx_result = HAL_SPI_Receive(hspi, (uint8_t *)buf, 1, FLASH_TIMEOUT_MS);
    if (rx_result != HAL_OK) {
        FLASH_deactivate_chip_select();
        return 2;
    }

    FLASH_deactivate_chip_select();

    return 0;
}

/**
 * @brief Sends Write Enable Command
 * @param hspi - Pointer to the SPI HAL handle
 * @param chip_number - the chip select number to activate
 * @retval None
 */
uint8_t FLASH_write_enable(SPI_HandleTypeDef *hspi, uint8_t chip_number)
{
    // Buffer to store status register values in
    uint8_t statusRegBuffer[1] = {0};

    FLASH_activate_chip_select(chip_number);
    const HAL_StatusTypeDef tx_result_1 = HAL_SPI_Transmit(hspi, (uint8_t *)&FLASH_WREN, 1, HAL_MAX_DELAY);
    FLASH_deactivate_chip_select();
    if (tx_result_1 != HAL_OK) {
        return 1;
    }

    // Keep looping as long as device is busy
    uint8_t wip = 1;
    uint8_t read_status_result;
    while (wip)
    {
        // FIXME: this could stall forever
        read_status_result = FLASH_read_status_register(hspi, chip_number, statusRegBuffer);
        if (read_status_result != 0) {
            FLASH_deactivate_chip_select();
            return 1;
        }
        wip = statusRegBuffer[0] & 1;
    }
    return 0;
}

/**
 * @brief Sends Write Disable Command
 * @param hspi - Pointer to the SPI HAL handle
 * @param chip_number - the chip select number to activate
 * @retval None
 */
uint8_t FLASH_write_disable(SPI_HandleTypeDef *hspi, uint8_t chip_number)
{
    // Buffer to store status register values in
    uint8_t statusRegBuffer[1] = {0};

    FLASH_deactivate_chip_select();
    const HAL_StatusTypeDef tx_status = HAL_SPI_Transmit(hspi, (uint8_t *)&FLASH_WRDI, 1, FLASH_TIMEOUT_MS);

    if (tx_status != HAL_OK) {
        FLASH_deactivate_chip_select();
        return 1;
    }

    // Keep looping until Write Enable latch isn't 0
    uint8_t wel = 1;
    while (wel)
    {
        const uint8_t read_status_result = FLASH_read_status_register(hspi, chip_number, statusRegBuffer);
        if (read_status_result != 0) {
            FLASH_deactivate_chip_select();
            return 1;
        }
        wel = statusRegBuffer[0] & 2;
    }
    return 0;
}

/**
 * @brief Sends Sector Erase Command
 * @param hspi - Pointer to the SPI HAL handle
 * @param chip_number - the chip select number to activate
 * @param addr - block number that is to be erased
 * @retval Returns -1 if erase failed, else return 0
 */
uint8_t FLASH_erase(SPI_HandleTypeDef *hspi, uint8_t chip_number, lfs_block_t addr)
{
    // Split block into 3 address spaces that the device accepts
    uint8_t addr_bytes[4] = {(addr >> 24) & 0xFF, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF};

    // Send Write Enable Command
    FLASH_write_enable(hspi, chip_number);

    // Send Sector Erase Command
    // FIXME: read result here
    FLASH_activate_chip_select(chip_number);
    HAL_SPI_Transmit(hspi, (uint8_t *)&FLASH_4SE, 1, FLASH_TIMEOUT_MS);
    HAL_SPI_Transmit(hspi, (uint8_t *)addr_bytes, 4, FLASH_TIMEOUT_MS);
    FLASH_deactivate_chip_select();

    // Buffer to store status register values in
    uint8_t statusRegBuffer[1] = {0};

    // Keep looping as long as device is busy
    uint8_t wip = 1;
    uint8_t err = 0;
    while (wip)
    {
        FLASH_read_status_register(hspi, chip_number, statusRegBuffer);
        wip = statusRegBuffer[0] & 1;
        err = statusRegBuffer[0] & 0b01000000;
        if (err == 1) {
            // FIXME: why don't we return here?
            break;
        }
    }
    if (err)
        return -1;
    
    return 0;
}

/**
 * @brief Sends Page Program Command
 * @param hspi - Pointer to the SPI HAL handle
 * @param chip_number - the chip select number to activate
 * @param addr - block number that is to be written
 * @param packet_buffer - Pointer to buffer containing data to write
 * @param size - integer that idicates the size of the data
 * @retval Returns -1 if write failed, else return 0
 */
uint8_t FLASH_write(SPI_HandleTypeDef *hspi, uint8_t chip_number, lfs_block_t addr, uint8_t *packet_buffer, lfs_size_t size)
{
    // Split block into 3 address spaces that the device accepts
    uint8_t addr_bytes[4] = {(addr >> 24) & 0xFF, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF};

    // Enable WREN Command, so that we can write to the memory module
    FLASH_write_enable(hspi, chip_number);

    // Send WREN Command and the Data required with the command
    // FIXME: read results here and below
    FLASH_activate_chip_select(chip_number);
    HAL_SPI_Transmit(hspi, (uint8_t *)&FLASH_4WRITE, 1, FLASH_TIMEOUT_MS);
    HAL_SPI_Transmit(hspi, (uint8_t *)addr_bytes, 4, FLASH_TIMEOUT_MS);
    HAL_SPI_Transmit(hspi, (uint8_t *)packet_buffer, size, FLASH_TIMEOUT_MS);
    FLASH_deactivate_chip_select();

    // Buffer to store status register values in
    uint8_t statusRegBuffer[1] = {0};

    // Keep looping as long as device is busy
    uint8_t wip = 1;
    uint8_t err = 0;
    while (wip)
    {
        FLASH_read_status_register(hspi, chip_number, statusRegBuffer);
        wip = statusRegBuffer[0] & 1;
        err = statusRegBuffer[0] & 0b01000000;
        if (err == 1)
            break;
    }
    if (err)
        return -1;
    else
        return 0;
}

/**
 * @brief Sends Page Program Command
 * @param hspi - Pointer to the SPI HAL handle
 * @param chip_number - the chip select number to activate
 * @param addr - block number that is to be read
 * @param rx_buffer - a to buffer where the read data will be stored
 * @param rx_buffer_len - integer that idicates the size of the data
 * @retval Returns -1 if read failed, else return 0
 */
uint8_t FLASH_read_data(SPI_HandleTypeDef *hspi, uint8_t chip_number, lfs_block_t addr, uint8_t *rx_buffer, lfs_size_t rx_buffer_len)
{
    // Split block into 3 address spaces that the device accepts
    uint8_t addr_bytes[4] = {(addr >> 24) & 0xFF, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF};

    // Send Read Command and the data required with it
    FLASH_activate_chip_select(chip_number);
    const HAL_StatusTypeDef tx_result_1 = HAL_SPI_Transmit(hspi, (uint8_t *)&FLASH_4READ, 1, FLASH_TIMEOUT_MS);
    if (tx_result_1 != HAL_OK) {
        FLASH_deactivate_chip_select();
        return 1;
    }
    const HAL_StatusTypeDef tx_result_2 = HAL_SPI_Transmit(hspi, (uint8_t *)addr_bytes, 4, FLASH_TIMEOUT_MS);
    if (tx_result_2 != HAL_OK) {
        FLASH_deactivate_chip_select();
        return 2;
    }
    const HAL_StatusTypeDef rx_result_1 = HAL_SPI_Receive(hspi, (uint8_t *)rx_buffer, rx_buffer_len, FLASH_TIMEOUT_MS);
    if (rx_result_1 != HAL_OK) {
        FLASH_deactivate_chip_select();
        return 3;
    }
    FLASH_deactivate_chip_select();

    // Haven't yet implemented a way to check any errors while reading data from memory
    return 0;
}
