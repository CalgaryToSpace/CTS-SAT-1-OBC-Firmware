

/*-----------------------------INCLUDES-----------------------------*/
#include <stdint.h>

#include "littlefs/littlefs_helper.h"
#include "littlefs/littlefs_driver.h"
#include "debug_tools/debug_uart.h"
#include "config/static_config.h"

/*-----------------------------VARIABLES-----------------------------*/
// Variables to track LittleFS on Flash Memory Module
uint8_t LFS_is_lfs_mounted = 0;

#define FLASH_CHIP_PAGE_SIZE_BYTES 512
#define FLASH_CHIP_BLOCK_SIZE_BYTES 262144
#define FLASH_LOOKAHEAD_SIZE 16

// LittleFS Buffers for reading and writing
uint8_t LFS_read_buffer[FLASH_CHIP_PAGE_SIZE_BYTES];
uint8_t LFS_prog_buffer[FLASH_CHIP_PAGE_SIZE_BYTES];
uint8_t LFS_lookahead_buf[FLASH_LOOKAHEAD_SIZE];
uint8_t LFS_file_buffer[FLASH_CHIP_PAGE_SIZE_BYTES];

// TODO: look into the `LFS_F_INLINE` macro to increase the maximum number of files we can have

// Variables LittleFS uses for various functions (all externed in littlefs_helper.h)
lfs_t LFS_filesystem;
struct lfs_config LFS_cfg = {
    .read = LFS_block_device_read,
    .prog = LFS_block_device_prog,
    .erase = LFS_block_device_erase,
    .sync = LFS_block_device_sync,

    // block device configuration
    .read_size = FLASH_CHIP_PAGE_SIZE_BYTES,
    .prog_size = FLASH_CHIP_PAGE_SIZE_BYTES,
    .block_size = FLASH_CHIP_BLOCK_SIZE_BYTES, // FIXME: Clarify block Size 256KiB or 1KiB
    .block_count = (FLASH_CHIP_SIZE_BYTES / FLASH_CHIP_BLOCK_SIZE_BYTES),
    .block_cycles = 100, // TODO: ASK ABOUT THIS (HOW FREQUENT ARE WE USING THE MODULE),
    .cache_size = FLASH_CHIP_PAGE_SIZE_BYTES,
    .lookahead_size = FLASH_LOOKAHEAD_SIZE,
    .compact_thresh = -1, // Defaults to ~88% block_size when zero (lfs.h, line 232)

    .read_buffer = LFS_read_buffer,
    .prog_buffer = LFS_prog_buffer,
    .lookahead_buffer = LFS_lookahead_buf};

struct lfs_file_config LFS_file_cfg = {
    .buffer = LFS_file_buffer,
    .attr_count = 0,
    .attrs = NULL};

// -----------------------------LITTLEFS FUNCTIONS-----------------------------


/**
 * @brief Formats Memory Module so it can successfully mount LittleFS
 * @param None
 * @retval 0 on success, negative LFS error codes on failure
 */
int8_t LFS_format()
{
	int8_t result = lfs_format(&LFS_filesystem, &LFS_cfg);
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
 * @retval 0 on success, 1 if LFS is already mounted, negative LFS error codes on failure
 */
int8_t LFS_mount()
{
	if (LFS_is_lfs_mounted) {
		DEBUG_uart_print_str("LittleFS already mounted!\n");
		return 1;
	}

    // Variable to store status of LittleFS mounting
    int8_t mount_result = lfs_mount(&LFS_filesystem, &LFS_cfg);
    if (mount_result < 0)
    {
        DEBUG_uart_print_str("Mounting unsuccessful\n");
        return mount_result;
    }

    DEBUG_uart_print_str("Mounting successful\n");
    LFS_is_lfs_mounted = 1;
    return 0;
}

/**
 * @brief Unmounts LittleFS to the Memory Module
 * @param None
 * @retval 0 on success, 1 if LFS is already unmounted, negative LFS error codes on failure
 */
int8_t LFS_unmount()
{
    if (!LFS_is_lfs_mounted)
    {
        DEBUG_uart_print_str("LittleFS not mounted.\n");
        return 1;
    }

    // Unmount LittleFS to release any resources used by LittleFS
    const int8_t unmount_result = lfs_unmount(&LFS_filesystem);
    if (unmount_result < 0)
    {
        DEBUG_uart_print_str("Error un-mounting.\n");
        return unmount_result;
    }

    DEBUG_uart_print_str("Successfully un-mounted LittleFS.\n");
    LFS_is_lfs_mounted = 0;
    return 0;
}

/**
 * @brief Lists contents of LittleFS Directory
 * @param root_directory Pointer to cstring holding the root directory to open and read 
 * @retval 0 on success, 1 if LFS is unmounted, negative LFS error codes on failure
 */
int8_t LFS_list_directory(char *root_directory)
{
    if (!LFS_is_lfs_mounted)
    {
        DEBUG_uart_print_str("LittleFS not mounted.\n");
        return 1;
    }

    lfs_dir_t dir;
    int8_t open_dir_result = lfs_dir_open(&LFS_filesystem, &dir, root_directory);
    if (open_dir_result < 0)
    {
        DEBUG_uart_print_str("Error opening a directory.\n");
        return open_dir_result;
    }

    // result is positive on success, 0 at the end of directory, or  negative on failure.
    int8_t read_dir_result = 1;
    while (read_dir_result >= 0)
    {
        struct lfs_info info;
        read_dir_result = lfs_dir_read(&LFS_filesystem, &dir, &info);

        DEBUG_uart_print_str(info.name);
        DEBUG_uart_print_str(", ");
        // TODO: The info struct contains information about directory contents
    }
    DEBUG_uart_print_str("\n");

    if (read_dir_result < 0)
    {
        DEBUG_uart_print_str("Error reading directory contents.\n");
        return read_dir_result;
    }

    DEBUG_uart_print_str("Successfully Listed Directory Contents.\n");

    int8_t close_dir_result = lfs_dir_close(&LFS_filesystem, &dir);
    if (close_dir_result < 0)
    {
        DEBUG_uart_print_str("Error closing directory.\n");
        return close_dir_result;
    }

    return 0;
}

/**
 * @brief Removes / deletes the file specified
 * @param file_name Pointer to cstring holding the file name to remove
 * @retval 0 on success, 1 if LFS is unmounted, negative LFS error codes on failure
 */
int8_t LFS_delete_file(const char file_name[])
{
    if (!LFS_is_lfs_mounted)
    {
        DEBUG_uart_print_str("LittleFS not mounted.\n");
        return 1;
    }

    int8_t remove_result = lfs_remove(&LFS_filesystem, file_name);
    if (remove_result < 0)
    {
        DEBUG_uart_print_str("Error removing file/directory.\n");
        return remove_result;
    }

    DEBUG_uart_print_str("Successfully removed file/directory.\n");
    return 0;
}

/**
 * @brief Creates directory
 * @param dir_name Pointer to cstring holding the name of the directory
 * @retval 0 on success, 1 if LFS is unmounted, negative LFS error codes on failure
 */
int8_t LFS_make_directory(char *dir_name)
{
    if (!LFS_is_lfs_mounted)
    {
        DEBUG_uart_print_str("LittleFS not mounted.\n");
        return 1;
    }

    int8_t make_dir_result = lfs_mkdir(&LFS_filesystem, dir_name);
    if (make_dir_result < 0)
    {
        DEBUG_uart_print_str("Error creating directory.\n");
        return make_dir_result;
    }

    DEBUG_uart_print_str("Successfully created directory.\n");
    return 0;
}

/**
 * @brief Creates / Opens LittleFS File to write to the Memory Module
 * @param file_name - Pointer to cstring holding the file name to create or open
 * @param write_buffer - Pointer to buffer holding the data to write
 * @param write_buffer_len - Size of the data to write
 * @retval 0 on success, 1 if LFS is unmounted, negative LFS error codes on failure
 */
int8_t LFS_write_file(const char file_name[], uint8_t *write_buffer, uint32_t write_buffer_len)
{
    if (!LFS_is_lfs_mounted)
    {
        DEBUG_uart_print_str("LittleFS not mounted.\n");
        return 1;
    }

    // Create or Open a file with Write only flag
    lfs_file_t file;
    const int8_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, file_name, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC, &LFS_file_cfg);

	if (open_result < 0)
	{
		DEBUG_uart_print_str("Error opening/creating file.\n");
		return open_result;
	}
	
    if (LFS_enable_hot_path_debug_logs) {
        DEBUG_uart_print_str("Opened/created a file named: '");
        DEBUG_uart_print_str(file_name);
        DEBUG_uart_print_str("'\n");
    }

	// Write data to file
	const int8_t write_result = lfs_file_write(&LFS_filesystem, &file, write_buffer, write_buffer_len);
	if (write_result < 0)
	{
		DEBUG_uart_print_str("Error writing to file!\n");
		return write_result;
	}
	
    if (LFS_enable_hot_path_debug_logs) {
	    DEBUG_uart_print_str("Successfully wrote data to file!\n");
    }

	// Close the File, the storage is not updated until the file is closed successfully
	const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
	if (close_result < 0)
	{
		DEBUG_uart_print_str("Error closing the file!\n");
		return close_result;
	}
	
    if (LFS_enable_hot_path_debug_logs) {
	    DEBUG_uart_print_str("Successfully closed the file!\n");
    }

	return 0;
}

/**
 * @brief Opens LittleFS File to read from the Memory Module
 * @param file_name - Pointer to buffer holding the file name to open
 * @param read_buffer - Pointer to buffer where the read data will be stored
 * @param read_buffer_len - Size of the data to read
 * @retval Returns negative values if read or file open failed, else return 0
 */
int8_t LFS_read_file(const char file_name[], uint8_t *read_buffer, uint32_t read_buffer_len)
{
	lfs_file_t file;
	const int8_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, file_name, LFS_O_RDONLY, &LFS_file_cfg);
	if (open_result < 0)
	{
		DEBUG_uart_print_str("Error opening file to read\n");
		return open_result;
	}
	
    if (LFS_enable_hot_path_debug_logs) {
        DEBUG_uart_print_str("Opened file to read: ");
        DEBUG_uart_print_str(file_name);
        DEBUG_uart_print_str("\n");
    }

	const int8_t read_result = lfs_file_read(&LFS_filesystem, &file, read_buffer, read_buffer_len);
	if (read_result < 0)
	{
		DEBUG_uart_print_str("Error Reading File!\n");
		return read_result;
	}
	
    if (LFS_enable_hot_path_debug_logs) {
	    DEBUG_uart_print_str("Successfully read file!\n");
    }

	// Close the File, the storage is not updated until the file is closed successfully
	const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
	if (close_result < 0)
	{
		DEBUG_uart_print_str("Error closing the file!\n");
		return close_result;
	}
	
    if (LFS_enable_hot_path_debug_logs) {
	    DEBUG_uart_print_str("Successfully closed the file!\n");
    }

	return 0;	
}
