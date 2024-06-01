

/*-----------------------------INCLUDES-----------------------------*/
#include <stdint.h>

#include "littlefs/littlefs_helper.h"
#include "littlefs/littlefs_driver.h"
#include "debug_tools/debug_uart.h"

/*-----------------------------VARIABLES-----------------------------*/
// Variables to track LittleFS on Flash Memory Module
uint8_t LFS_is_lfs_mounted = 0;

// LittleFS Buffers for reading and writing
uint8_t LFS_read_buffer[512];
uint8_t LFS_prog_buffer[512];

// Variables LittleFS uses for various functions
lfs_t lfs;
lfs_file_t file; // FIXME: this should be a local variable for each function
struct lfs_config cfg = {
	.read = LFS_block_device_read,
	.prog = LFS_block_device_prog,
	.erase = LFS_block_device_erase,
	.sync = LFS_block_device_sync,

	// block device configuration
	.read_size = 512,
	.prog_size = 512,
	.block_size = 262144, // TODO: this seems way too large
	.block_count = 256,
	.block_cycles = 100, // TODO: ASK ABOUT THIS (HOW FREQUENT ARE WE USING THE MODULE,
	.cache_size = 512,
	.lookahead_size = 16,
	.compact_thresh = -1, // Defaults to ~88% block_size when zero (lfs.h, line 232)

	.read_buffer = LFS_read_buffer,
	.prog_buffer = LFS_prog_buffer
};

// -----------------------------LITTLEFS FUNCTIONS-----------------------------


/**
 * @brief Fromates Memory Module so it can successfully mount LittleFS
 * @param None
 * @retval Returns negative values if format failed, else return 0
 */
int8_t LFS_format()
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

/**
 * @brief Mounts LittleFS to the Memory Module
 * @param None
 * @retval Returns negative values if mount failed, else return 0
 */
int8_t LFS_mount()
{
	// If LittleFS is initialized and not mounted
	if (!LFS_is_lfs_mounted)
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
			LFS_is_lfs_mounted = 1;
			return 0;
		}
	}
	debug_uart_print_str("LittleFS already mounted!\n");
	return -1;
}

/**
 * @brief Unmounts LittleFS to the Memory Module
 * @param None
 * @retval Returns negative values if unmount failed, else return 0
 */
int8_t LFS_unmount()
{
	if (LFS_is_lfs_mounted)
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
			LFS_is_lfs_mounted = 0;
			return result;
		}
	}
	debug_uart_print_str("LittleFS not mounted!\n");
	return -1;
}

void LFS_list_directory()
{
	// FIXME: implement
}

void LFS_delete_file()
{
}

void LFS_make_directory()
{
}


/**
 * @brief Creates / Opens LittleFS File to write to the Memory Module
 * @param file_name - Pointer to cstring buffer holding the file name to create or open
 * @param write_buffer - Pointer to buffer holding the data to write
 * @param write_buffer_len - Size of the data to write
 * @retval Returns negative values if write or file create / open failed, else return 0
 */
int8_t LFS_write_file(char *file_name, uint8_t *write_buffer, uint32_t write_buffer_len)
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
		result = lfs_file_write(&lfs, &file, write_buffer, write_buffer_len);
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
 * @param file_name - Pointer to buffer holding the file name to open
 * @param read_buffer - Pointer to buffer where the read data will be stored
 * @param read_buffer_len - Size of the data to read
 * @retval Returns negative values if read or file open failed, else return 0
 */
int8_t LFS_read_file(char *file_name, uint8_t *read_buffer, uint32_t read_buffer_len)
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
		result = lfs_file_read(&lfs, &file, read_buffer, read_buffer_len);
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
