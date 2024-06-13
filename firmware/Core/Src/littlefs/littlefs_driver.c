/**
 * littlefs_driver.c
 *
 * Created on: May 2, 2024
 * Author: Saksham Puri
 */

/*-----------------------------INCLUDES-----------------------------*/
#include "littlefs/littlefs_driver.h"
#include "littlefs/flash_driver.h"

// #include "main.h" // FIXME: Is this needed?

/*-----------------------------VARIABLES-----------------------------*/
// SPI pointer for LittleFS
SPI_HandleTypeDef *hspi_lfs_ptr = &hspi1;

// -----------------------------LITTLEFS CONFIG FUNCTIONS-----------------------------
/**
 * @brief LittleFS read function
 * @param block_num - Block number LittleFS is trying to access
 * @retval returns number from 0 - FLASH_NUMBER_OF_FLASH_DEVICES - 1
 */
uint8_t LFS_get_chip_number(lfs_block_t block_num)
{
    uint16_t block_count_per_device = (FLASH_CHIP_SIZE_BYTES / FLASH_CHIP_BLOCK_SIZE_BYTES);

    for (uint8_t i = FLASH_NUMBER_OF_FLASH_DEVICES-1; i > 0; i--)
    {
        if (block_num > (block_count_per_device * i))
        {
            return i;
        }
    }

    return 0;
}

/**
 * @brief LittleFS read function
 * @param c - Pointer to LittleFS Configuration
 * @param block - Block number that will be read
 * @param off - Offset after the starting address of block
 * @param buffer - Pointer to the buffer where the data will be stored
 * @param size - Size of the data to be read
 * @retval 0 on success, != 0 on failure
 */
int LFS_block_device_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
    return FLASH_read(
        hspi_lfs_ptr,
        LFS_get_chip_number(block),
        (block * c->block_size + off),
        (uint8_t *)buffer,
        size);
}

/**
 * @brief LittleFS write function
 * @param c - Pointer to LittleFS Configuration
 * @param block - Block number that will be programmed
 * @param off - Offset after the starting address of block
 * @param buffer - Pointer to the buffer containing the data to be programmed
 * @param size - Size of the data to be programmed
 * @retval 0 on success, != 0 on failure
 */
int LFS_block_device_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    return FLASH_write(
        hspi_lfs_ptr,
        LFS_get_chip_number(block),
        (block * c->block_size + off),
        (uint8_t *)buffer,
        size);
}

/**
 * @brief LittleFS erase function
 * @param c - Pointer to LittleFS Configuration
 * @param block - Block number that will be erased
 * @retval 0 on success, != 0 on failure
 */
int LFS_block_device_erase(const struct lfs_config *c, lfs_block_t block)
{
    return FLASH_erase(
        hspi_lfs_ptr,
        LFS_get_chip_number(block),
        block * c->block_size);
}

/**
 * @brief LittleFS sync function
 * @param c - Pointer to LittleFS Configuration
 * @retval 0 since this function isn't utilized
 */
int LFS_block_device_sync(const struct lfs_config *c)
{
    return 0;
}