#include "littlefs/flash_driver.h"
#include "littlefs/littlefs_driver.h"
#include "main.h"

SPI_HandleTypeDef *hspi_lfs_ptr = &hspi1;

// -----------------------------LITTLEFS CONFIG FUNCTIONS-----------------------------

uint8_t LFS_get_chip_number(lfs_block_t block_num) {
	// FIXME: add a conversion here
	return 0;
}

/// @brief LittleFS read function
/// @param LittleFS Configurations, Block to write, offset, buffer, buffer size
/// @retval int - any error codes that happened in littlefs

int LFS_block_device_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
	return FLASH_read_data(
		hspi_lfs_ptr,
		LFS_get_chip_number(block),
		((block * c->block_size) + off)/FLASH_MAX_BYTES_PER_PAGE,
		(uint8_t *)buffer,
		size
	);
}

/// @brief LittleFS write function
/// @param LittleFS Configurations, Block to read, offset, buffer, buffer size
/// @retval int - any error codes that happened in littlefs

int LFS_block_device_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
	return FLASH_write_data(
		hspi_lfs_ptr,
		LFS_get_chip_number(block),
		((block * c->block_size) + off)/FLASH_MAX_BYTES_PER_PAGE,
		(uint8_t *)buffer,
		size
	);
}

/// @brief LittleFS erase function
/// @param LittleFS Configurations, Block to erase
/// @retval int - any error codes that happened in littlefs

int LFS_block_device_erase(const struct lfs_config *c, lfs_block_t block)
{
	return FLASH_erase(
		hspi_lfs_ptr,
		LFS_get_chip_number(block),
		block * FLASH_CHIP_PAGES_PER_BLOCK
	);
}

/// @brief LittleFS sync function
/// @param c - LittleFS Configuration
/// @retval int - 0 since we are not caching reads or writes

int LFS_block_device_sync(const struct lfs_config *c)
{
	// Per the README:
	// If the write function does not perform caching, and therefore each read or write call hits the memory, the sync function can simply return 0.
	return 0;
}
