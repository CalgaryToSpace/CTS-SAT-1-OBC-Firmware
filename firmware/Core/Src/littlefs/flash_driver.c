/**
 * flash_driver.c
 *
 * Created on: June 1, 2024
 *     Author: Saksham Puri
 */

/*-----------------------------INCLUDES-----------------------------*/
#include "littlefs/flash_driver.h"

// -----------------------------FLASH DRIVER FUNCTIONS-----------------------------
/*
 * Chip Select set to LOW
 */
void FLASH_activate_chip_select()
{
    HAL_GPIO_WritePin(PIN_SCI_NCS_FLASH_0_GPIO_Port, PIN_SCI_NCS_FLASH_0_Pin, GPIO_PIN_RESET);
}

/*
 * Chip Select set to HIGH
 * This is the default state of the CS
 */
void FLASH_deactivate_chip_select()
{
    HAL_GPIO_WritePin(PIN_SCI_NCS_FLASH_0_GPIO_Port, PIN_SCI_NCS_FLASH_0_Pin, GPIO_PIN_SET);
}

/**
 * @brief Read Status Register and store the values in given buffer
 * @param SPI_HandleTypeDef - Pointer to the SPI configurations used
 * @param uint8_t - Pointer to a buffer to store SR1 values
 * @retval None
 */
void FLASH_read_status_register(SPI_HandleTypeDef *hspi1, uint8_t *buf)
{
    FLASH_activate_chip_select();
    HAL_SPI_Transmit(hspi1, (uint8_t *)&FLASH_RDSR1, 1, 100);
    HAL_SPI_Receive(hspi1, (uint8_t *)buf, 1, 100);
    FLASH_deactivate_chip_select();
}

/**
 * @brief Sends Write Enable Command
 * @param SPI_HandleTypeDef - Pointer to the SPI configurations used
 * @retval None
 */
void FLASH_write_enable(SPI_HandleTypeDef *hspi1)
{
    // Buffer to store status register values in
    uint8_t statusRegBuffer[1] = {0};

    FLASH_activate_chip_select();
    if (HAL_SPI_Transmit(hspi1, (uint8_t *)&FLASH_WREN, 1, HAL_MAX_DELAY) != HAL_OK)
        debug_uart_print_str("HAL Timeout when enabling WREN\n");
    FLASH_deactivate_chip_select();

    // Keep looping as long as device is busy
    uint8_t wip = 1;
    while (wip)
    {
        FLASH_read_status_register(hspi1, statusRegBuffer);
        wip = statusRegBuffer[0] & 1;
    }
}

/**
 * @brief Sends Write Disable Command
 * @param SPI_HandleTypeDef - Pointer to the SPI configurations used
 * @retval None
 */
void FLASH_write_disable(SPI_HandleTypeDef *hspi1)
{
    // Buffer to store status register values in
    uint8_t statusRegBuffer[1] = {0};

    FLASH_deactivate_chip_select();
    HAL_SPI_Transmit(hspi1, (uint8_t *)&FLASH_WRDI, 1, 100);

    // Keep looping until Write Enable latch isn't 0
    uint8_t wel = 1;
    while (wel)
    {
        FLASH_read_status_register(hspi1, statusRegBuffer);
        wel = statusRegBuffer[0] & 2;
    }
}

/**
 * @brief Sends Sector Erase Command
 * @param SPI_HandleTypeDef - Pointer to the SPI configurations used
 * @param lfs_block_t - block number that is to be erased
 * @retval Returns -1 if erase failed, else return 0
 */
uint8_t FLASH_erase(SPI_HandleTypeDef *ptr_hspi1, lfs_block_t block)
{
    // Split block into 3 address spaces that the device accepts
    uint8_t addr[4] = {(block >> 24) & 0xFF, (block >> 16) & 0xFF, (block >> 8) & 0xFF, block & 0xFF};

    // Send Write Enable Command
    FLASH_write_enable(ptr_hspi1);

    // Send Sector Erase Command
    FLASH_activate_chip_select();
    HAL_SPI_Transmit(ptr_hspi1, (uint8_t *)&FLASH_4SE, 1, 100);
    HAL_SPI_Transmit(ptr_hspi1, (uint8_t *)addr, 4, 100);
    FLASH_deactivate_chip_select();

    // Buffer to store status register values in
    uint8_t statusRegBuffer[1] = {0};

    // Keep looping as long as device is busy
    uint8_t wip = 1;
    uint8_t err = 0;
    while (wip)
    {
        FLASH_read_status_register(ptr_hspi1, statusRegBuffer);
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
 * @param SPI_HandleTypeDef - Pointer to the SPI configurations used
 * @param uint8_t - Pointer to buffer containing data to write
 * @param lfs_block_t - block number that is to be written
 * @param lfs_size_t - integer that idicates the size of the data
 * @retval Returns -1 if write failed, else return 0
 */
uint8_t FLASH_write(SPI_HandleTypeDef *hspi1, uint8_t *packetBuffer, lfs_block_t block, lfs_size_t size)
{
    // Split block into 3 address spaces that the device accepts
    uint8_t addr[4] = {(block >> 24) & 0xFF, (block >> 16) & 0xFF, (block >> 8) & 0xFF, block & 0xFF};

    // Enable WREN Command, so that we can write to the memory module
    FLASH_write_enable(hspi1);

    // Send WREN Command and the Data required with the command
    FLASH_activate_chip_select();
    HAL_SPI_Transmit(hspi1, (uint8_t *)&FLASH_4WRITE, 1, 100);
    HAL_SPI_Transmit(hspi1, (uint8_t *)addr, 4, 100);
    HAL_SPI_Transmit(hspi1, (uint8_t *)packetBuffer, size, 100);
    FLASH_deactivate_chip_select();

    // Buffer to store status register values in
    uint8_t statusRegBuffer[1] = {0};

    // Keep looping as long as device is busy
    uint8_t wip = 1;
    uint8_t err = 0;
    while (wip)
    {
        FLASH_read_status_register(hspi1, statusRegBuffer);
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
 * @param SPI_HandleTypeDef - Pointer to the SPI configurations used
 * @param uint8_t - Pointer to buffer where the read data will be stored
 * @param lfs_block_t - block number that is to be read
 * @param lfs_size_t - integer that idicates the size of the data
 * @retval Returns -1 if read failed, else return 0
 */
uint8_t FLASH_read(SPI_HandleTypeDef *hspi1, uint8_t *spiRxBuffer, lfs_block_t block, lfs_size_t size)
{
    // Split block into 3 address spaces that the device accepts
    uint8_t addr[4] = {(block >> 24) & 0xFF, (block >> 16) & 0xFF, (block >> 8) & 0xFF, block & 0xFF};

    // Send Read Command and the data required with it
    FLASH_activate_chip_select();
    HAL_SPI_Transmit(hspi1, (uint8_t *)&FLASH_4READ, 1, 100);
    HAL_SPI_Transmit(hspi1, (uint8_t *)addr, 4, 100);
    HAL_SPI_Receive(hspi1, (uint8_t *)spiRxBuffer, size, 100);
    FLASH_deactivate_chip_select();

    // Haven't yet implemented a way to check any errors while reading data from memory
    return 0;
}