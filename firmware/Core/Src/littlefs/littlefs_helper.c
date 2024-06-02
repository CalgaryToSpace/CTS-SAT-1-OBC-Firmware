/**
 * littlefs_helper.c
 *
 * Created on: May 2, 2024
 *     Author: Saksham Puri
 */

/*-----------------------------INCLUDES-----------------------------*/
#include "littlefs/littlefs_helper.h"

/*-----------------------------VARIABLES-----------------------------*/
// Variables to track LittleFS on Flash Memory Module
uint8_t LFS_initialized = 0;
uint8_t LFS_mounted = 0;

// Variables LittleFS uses for various functions
lfs_t lfs;
lfs_file_t file;
struct lfs_config cfg;

// LittleFS Buffers for reading and writing
uint8_t LFS_read_buffer[512];
uint8_t LFS_prog_buffer[512];

// Variable to store SPI configuration
SPI_HandleTypeDef *hspi_ptr;

// -----------------------------LITTLEFS CONFIG FUNCTIONS-----------------------------
/**
 * @brief Initialize LittleFS configurations
 * @param None
 * @retval None
 */
void LFS_config()
{
	// block device operations
	cfg.read = LFS_block_device_read;
	cfg.prog = LFS_block_device_prog;
	cfg.erase = LFS_block_device_erase;
	cfg.sync = LFS_block_device_sync;

	// block device configuration
	cfg.read_size = 512;
	cfg.prog_size = 512;
	cfg.block_size = 262144;
	cfg.block_count = 256;
	cfg.block_cycles = 100; // ASK ABOUT THIS (HOW FREQUENT ARE WE USING THE MODULE)
	cfg.cache_size = 512;
	cfg.lookahead_size = 16;
	cfg.compact_thresh = -1; // Defaults to ~88% block_size when zero (lfs.h, line 232)

	cfg.read_buffer = LFS_read_buffer;
	cfg.prog_buffer = LFS_prog_buffer;
}

/**
 * @brief LittleFS read function
 * @param LittleFS Configurations, Block to write, offset, buffer, buffer size
 * @retval int - any error codes that happened in littlefs
 */
int LFS_block_device_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
	return FLASH_read(hspi_ptr, (uint8_t *)buffer, (block * c->block_size + off), size);
}

/**
 * @brief LittleFS write function
 * @param LittleFS Configurations, Block to read, offset, buffer, buffer size
 * @retval int - any error codes that happened in littlefs
 */
int LFS_block_device_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
	return FLASH_write(hspi_ptr, (uint8_t *)buffer, (block * c->block_size + off), size);
}

/**
 * @brief LittleFS erase function
 * @param LittleFS Configurations, Block to erase
 * @retval int - any error codes that happened in littlefs
 */
int LFS_block_device_erase(const struct lfs_config *c, lfs_block_t block)
{
	return FLASH_erase(hspi_ptr, block * c->block_size);
}

/**
 * @brief LittleFS sync function
 * @param LittleFS Configurations
 * @retval int - 0 since this function isn't used
 */
int LFS_block_device_sync(const struct lfs_config *c)
{
	return 0;
}

// -----------------------------LITTLEFS FUNCTIONS-----------------------------
/**
 * @brief Initializes all LittleFS values as well as
 * 		  recieves SPI pointer to use for all functions
 * @param SPI_HandleTypeDef - Pointer to the SPI configurations used
 * @retval None
 */
void LFS_INITIALIZE(SPI_HandleTypeDef *ptr)
{
	LFS_config();
	LFS_initialized = 1;
	hspi_ptr = ptr;
}

/**
 * @brief Fromates Memory Module so it can successfully mount LittleFS
 * @param None
 * @retval Returns negative values if format failed, else return 0
 */
int8_t LFS_FORMAT()
{
	if (LFS_initialized)
	{
		int8_t result = lfs_format(&lfs, &cfg);
		if (result < 0)
		{
			debug_uart_print_str("Error Formatting!\n");
			return result;
		}
		else
		{
			debug_uart_print_str("Formatting Successful!\n");
			return result;
		}
	}
	debug_uart_print_str("LittleFS not Initialized\n");
	return -1;
}

/**
 * @brief Mounts LittleFS to the Memory Module
 * @param None
 * @retval Returns negative values if mount failed, else return 0
 */
int8_t LFS_MOUNT()
{
	// If LittleFS is initialized and not mounted
	if (LFS_initialized && !LFS_mounted)
	{
		// Variable to store status of LittleFS mounting
		int8_t result = lfs_mount(&lfs, &cfg);
		if (result < 0)
		{
			debug_uart_print_str("Mounting Unsuccessful\n");
			return result;
		}
		else
		{
			debug_uart_print_str("Mounting Successful\n");
			LFS_mounted = 1;
			return 0;
		}
	}
	if (!LFS_initialized)
		debug_uart_print_str("LittleFS not initialized!\n");
	else
		debug_uart_print_str("LittleFS already mounted!\n");
	return -1;
}

/**
 * @brief Unmounts LittleFS to the Memory Module
 * @param None
 * @retval Returns negative values if unmount failed, else return 0
 */
int8_t LFS_UNMOUNT()
{
	if (LFS_initialized && LFS_mounted)
	{
		// Unmount LittleFS to release any resources used by LittleFS
		int8_t result = lfs_unmount(&lfs);
		if (result < 0)
		{
			debug_uart_print_str("Error Un-mounting!\n");
			return result;
		}
		else
		{
			debug_uart_print_str("Successfully Un-mounted LittleFS!\n");
			LFS_mounted = 0;
			return result;
		}
	}
	if (!LFS_initialized)
		debug_uart_print_str("LittleFS not initialized!\n");
	else
		debug_uart_print_str("LittleFS not mounted!\n");
	return -1;
}

void LFS_LIST_DIRECTORY()
{
}

void LFS_REMOVE_FILE()
{
}

void LFS_MAKE_DIRECTORY()
{
}

void LFS_MOVE_DIRECTORY()
{
}

/**
 * @brief Creates / Opens LittleFS File to write to the Memory Module
 * @param char - Pointer to buffer holding the file name to create or open
 * @param void - Pointer to buffer holding the data to write
 * @param uint32_t - Size of the data to write
 * @retval Returns negative values if write or file create / open failed, else return 0
 */
int8_t LFS_WRITE_FILE(char *file_name, void *write_buffer, uint32_t size)
{
	// Create or Open a file with Write only flag
	int8_t result = lfs_file_open(&lfs, &file, file_name, LFS_O_WRONLY | LFS_O_CREAT);

	if (result < 0)
	{
		debug_uart_print_str("Error Opening / Creating a File!\n");
		return result;
	}
	else
	{
		debug_uart_print_str("Opened / Created a file named: \n");
		debug_uart_print_str(file_name);

		// Write data to file
		result = lfs_file_write(&lfs, &file, write_buffer, size);
		if (result < 0)
		{
			debug_uart_print_str("Error Writing to File!\n");
			return result;
		}
		else
		{
			debug_uart_print_str("Successfully wrote data to file!\n");

			// Close the File, the storage is not updated until the file is closed successfully
			result = lfs_file_close(&lfs, &file);
			if (result < 0)
			{
				debug_uart_print_str("Error Closing the File!\n");
				return result;
			}
			else
			{
				debug_uart_print_str("Successfully closed the File!\n");
				return result;
			}
		}
	}
}

/**
 * @brief Opens LittleFS File to read from the Memory Module
 * @param char - Pointer to buffer holding the file name to open
 * @param void - Pointer to buffer where the read data will be stored
 * @param uint32_t - Size of the data to read
 * @retval Returns negative values if read or file open failed, else return 0
 */
int8_t LFS_READ_FILE(char *file_name, void *read_buffer, uint32_t size)
{
	int8_t result = lfs_file_open(&lfs, &file, file_name, LFS_O_RDONLY | LFS_O_CREAT);
	if (result < 0)
	{
		debug_uart_print_str("Error Opening / Creating a File!\n");
		return result;
	}
	else
	{
		debug_uart_print_str("Opened / Created a file named: \n");
		debug_uart_print_str(file_name);

		// Write defined data to file
		result = lfs_file_read(&lfs, &file, read_buffer, size);
		if (result < 0)
		{
			debug_uart_print_str("Error Reading File!\n");
			return result;
		}
		else
		{
			debug_uart_print_str("Successfully read file!\n");

			// Close the File, the storage is not updated until the file is closed successfully
			result = lfs_file_close(&lfs, &file);
			if (result < 0)
			{
				debug_uart_print_str("Error Closing the File!\n");
				return result;
			}
			else
			{
				debug_uart_print_str("Successfully closed the File!\n");
				return result;
			}
		}
	}
}
