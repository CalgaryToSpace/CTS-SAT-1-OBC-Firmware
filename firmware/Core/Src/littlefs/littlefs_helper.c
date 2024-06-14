

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
uint8_t LFS_lookahead_buf[16];

// TODO: look into the `LFS_F_INLINE` macro to increase the maximum number of files we can have

// Variables LittleFS uses for various functions
lfs_t lfs;
struct lfs_config cfg = {
	.read = LFS_block_device_read,
	.prog = LFS_block_device_prog,
	.erase = LFS_block_device_erase,
	.sync = LFS_block_device_sync,

	// block device configuration
	.read_size = 512,
	.prog_size = 512,
	// On the "S25FL512S", the minimum eraseable block size is 256 KiB = 262144 bytes.
	// The block size is the smallest erasable unit of the flash memory, and is the minimum size
	// an individual file occupies.
	.block_size = 262144,
	.block_count = 256, // 512 Mib / 256 KiB = 256 (i.e., max 256 files)
	.block_cycles = 100, // TODO: ASK ABOUT THIS (HOW FREQUENT ARE WE USING THE MODULE)
	.cache_size = 512,
	.lookahead_size = 16,
	.compact_thresh = -1, // Defaults to ~88% block_size when zero (lfs.h, line 232)

	.read_buffer = LFS_read_buffer,
	.prog_buffer = LFS_prog_buffer,
	.lookahead_buffer = LFS_lookahead_buf
};

// -----------------------------LITTLEFS FUNCTIONS-----------------------------


/**
 * @brief Formats Memory Module so it can successfully mount LittleFS
 * @param None
 * @retval Returns negative values if format failed, else return 0
 */
int8_t LFS_format()
{
	int8_t result = lfs_format(&lfs, &cfg);
	if (result < 0)
	{
		DEBUG_uart_print_str("Error formatting!\n");
		return result;
	}
	
	DEBUG_uart_print_str("Formatting successful!\n");
	return 0;
}

/**
 * @brief Mounts LittleFS to the Memory Module
 * @param None
 * @retval Returns 0 on success. Negative values if mount failed. 1 if already mounted.
 */
int8_t LFS_mount()
{
	if (LFS_is_lfs_mounted) {
		DEBUG_uart_print_str("LittleFS already mounted!\n");
		return 1;
	}

	// Variable to store status of LittleFS mounting
	int8_t result = lfs_mount(&lfs, &cfg);
	if (result < 0)
	{
		DEBUG_uart_print_str("Mounting unsuccessful\n");
		return result;
	}
	
	DEBUG_uart_print_str("Mounting successful\n");
	LFS_is_lfs_mounted = 1;
	return 0;
}

/**
 * @brief Unmounts LittleFS to the Memory Module
 * @param None
 * @retval Returns negative values if unmount failed, else return 0
 */
int8_t LFS_unmount()
{
	if (! LFS_is_lfs_mounted) {
		DEBUG_uart_print_str("LittleFS not mounted.\n");
		return -1;
	}

	// Unmount LittleFS to release any resources used by LittleFS
	const int8_t unmount_result = lfs_unmount(&lfs);
	if (unmount_result < 0)
	{
		DEBUG_uart_print_str("Error un-mounting.\n");
		return unmount_result;
	}
	
	DEBUG_uart_print_str("Successfully un-mounted LittleFS.\n");
	LFS_is_lfs_mounted = 0;
	return 0;
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
 * @param file_name - Pointer to cstring holding the file name to create or open
 * @param write_buffer - Pointer to buffer holding the data to write
 * @param write_buffer_len - Size of the data to write
 * @retval Returns negative values if write or file create / open failed, else return 0
 */
int8_t LFS_write_file(char *file_name, uint8_t *write_buffer, uint32_t write_buffer_len)
{
	// Create or Open a file with Write only flag
	lfs_file_t file;
	const int8_t open_result = lfs_file_open(&lfs, &file, file_name, LFS_O_WRONLY | LFS_O_CREAT);

	if (open_result < 0)
	{
		DEBUG_uart_print_str("Error opening/creating file.\n");
		return open_result;
	}
	
	DEBUG_uart_print_str("Opened/created a file named: '");
	DEBUG_uart_print_str(file_name);
	DEBUG_uart_print_str("'\n");

	// Write data to file
	const int8_t write_result = lfs_file_write(&lfs, &file, write_buffer, write_buffer_len);
	if (write_result < 0)
	{
		DEBUG_uart_print_str("Error writing to file!\n");
		return write_result;
	}
	
	DEBUG_uart_print_str("Successfully wrote data to file!\n");

	// Close the File, the storage is not updated until the file is closed successfully
	const int8_t close_result = lfs_file_close(&lfs, &file);
	if (close_result < 0)
	{
		DEBUG_uart_print_str("Error closing the file!\n");
		return close_result;
	}
	
	DEBUG_uart_print_str("Successfully closed the file!\n");
	return 0;
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
	lfs_file_t file;
	const int8_t open_result = lfs_file_open(&lfs, &file, file_name, LFS_O_RDONLY);
	if (open_result < 0)
	{
		DEBUG_uart_print_str("Error opening file to read\n");
		return open_result;
	}
	
	DEBUG_uart_print_str("Opened file to read: ");
	DEBUG_uart_print_str(file_name);
	DEBUG_uart_print_str("\n");

	const int8_t read_result = lfs_file_read(&lfs, &file, read_buffer, read_buffer_len);
	if (read_result < 0)
	{
		DEBUG_uart_print_str("Error Reading File!\n");
		return read_result;
	}
	
	DEBUG_uart_print_str("Successfully read file!\n");

	// Close the File, the storage is not updated until the file is closed successfully
	const int8_t close_result = lfs_file_close(&lfs, &file);
	if (close_result < 0)
	{
		DEBUG_uart_print_str("Error closing the file!\n");
		return close_result;
	}
	
	DEBUG_uart_print_str("Successfully closed the file!\n");
	return 0;	
}
