#include "littlefs/flash_driver.h"
#include "littlefs/littlefs_driver.h"
#include "main.h"

SPI_HandleTypeDef *hspi_lfs_ptr = &hspi1;

// -----------------------------LITTLEFS CONFIG FUNCTIONS-----------------------------

inline static FLASH_Physical_Address_t _block_plus_offset_to_address(lfs_block_t block, lfs_off_t offset) {
	FLASH_Physical_Address_t address = {
		.block_address = (block * FLASH_CHIP_BLOCK_SIZE_BYTES)/ FLASH_CHIP_PAGE_SIZE_BYTES,
		.row_address = ((block * FLASH_CHIP_BLOCK_SIZE_BYTES) + (offset - (offset % FLASH_CHIP_PAGE_SIZE_BYTES))) / FLASH_CHIP_PAGE_SIZE_BYTES, 
		.col_address = (block* FLASH_CHIP_BLOCK_SIZE_BYTES + offset) % FLASH_CHIP_PAGE_SIZE_BYTES // address to the a specific byte in the page.
	};
	return address;
}

uint8_t LFS_get_chip_number(lfs_block_t block_num) {
	// FIXME: add a conversion here
	return 0;
}

/// @brief LittleFS read function, memory is mapped to a physical address here.
/// @param LittleFS Configurations, Block to write, offset, buffer, buffer size
/// @return int - any error codes that happened in littlefs

int LFS_block_device_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{

	return FLASH_read_page(
		LFS_get_chip_number(block),
		_block_plus_offset_to_address(block, off),
		(uint8_t *)buffer,
		size
	);
}

/// @brief LittleFS write function, memory is mapped to a physical address here.
/// @param LittleFS Configurations, Block to read, offset, buffer, buffer size
/// @return int - any error codes that happened in littlefs

int LFS_block_device_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
	return FLASH_program_page(
		LFS_get_chip_number(block),
		_block_plus_offset_to_address(block, off),
		(uint8_t *)buffer,
		size
	);
}

/// @brief LittleFS erase function, memory is mapped to a physical address here.
/// @param LittleFS Configurations, Block to erase
/// @return int - any error codes that happened in littlefs

int LFS_block_device_erase(const struct lfs_config *c, lfs_block_t block)
{
	return FLASH_erase_block(
		LFS_get_chip_number(block),
		_block_plus_offset_to_address(block, 0)
	);
}

/// @brief LittleFS sync function
/// @param c - LittleFS Configuration
/// @return int - 0 since we are not caching reads or writes

int LFS_block_device_sync(const struct lfs_config *c)
{
	// Per the README:
	// If the write function does not perform caching, and therefore each read or write call hits the memory, the sync function can simply return 0.
	return 0;
}
