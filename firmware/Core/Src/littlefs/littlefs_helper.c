

/*-----------------------------INCLUDES-----------------------------*/
#include <stdint.h>

#include "littlefs/littlefs_helper.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_driver.h"
#include "debug_tools/debug_uart.h"
#include "config/static_config.h"
#include "log/log.h"

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
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error formatting!");
        return result;
    }

    // UART replaced
    LOG_message(
        LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Formatting successful!\n");
    return 0;
}

/**
 * @brief Mounts LittleFS to the Memory Module
 * @param None
 * @retval 0 on success, 1 if LFS is already mounted, negative LFS error codes on failure
 */
int8_t LFS_mount()
{
    if (LFS_is_lfs_mounted)
    {
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "LittleFS already mounted!\n");
        return 1;
    }

    // Variable to store status of LittleFS mounting
    int8_t mount_result = lfs_mount(&LFS_filesystem, &LFS_cfg);
    if (mount_result < 0)
    {
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "Mounting unsuccessful\n");
        return mount_result;
    }

    // UART replaced
    LOG_message(
        LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Mounting successful\n");
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
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "LittleFS not mounted.\n");
        return 1;
    }

    // Unmount LittleFS to release any resources used by LittleFS
    const int8_t unmount_result = lfs_unmount(&LFS_filesystem);
    if (unmount_result < 0)
    {
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error un-mounting.\n");
        return unmount_result;
    }

    // UART replaced
    LOG_message(
        LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Successfully un-mounted LittleFS.\n");
    LFS_is_lfs_mounted = 0;
    return 0;
}

/**
 * @brief Lists contents of LittleFS Directory
 * @param root_directory Pointer to cstring holding the root directory to open and read
 * @retval 0 on success, 1 if LFS is unmounted, negative LFS error codes on failure
 */
int8_t LFS_list_directory(const char root_directory[])
{
    if (!LFS_is_lfs_mounted)
    {
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "LittleFS not mounted.\n");
        return 1;
    }

    lfs_dir_t dir;
    int8_t open_dir_result = lfs_dir_open(&LFS_filesystem, &dir, root_directory);
    if (open_dir_result < 0)
    {
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error opening a directory.\n");
        return open_dir_result;
    }

    // result is positive on success, 0 at the end of directory, or  negative on failure.
    int8_t read_dir_result = 1;
    while (read_dir_result >= 0)
    {
        struct lfs_info info;
        read_dir_result = lfs_dir_read(&LFS_filesystem, &dir, &info);

        DEBUG_uart_print_str(info.name);
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            ", ");
        // TODO: The info struct contains information about directory contents
    }
    // UART replaced
    // LOG_message(
    //  LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
    //"\n");

    if (read_dir_result < 0)
    {
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error reading directory contents.\n");
        return read_dir_result;
    }

    // UART replaced
    LOG_message(
        LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Successfully Listed Directory Contents.\n");

    int8_t close_dir_result = lfs_dir_close(&LFS_filesystem, &dir);
    if (close_dir_result < 0)
    {
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error closing directory.\n");
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
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "LittleFS not mounted.\n");
        return 1;
    }

    int8_t remove_result = lfs_remove(&LFS_filesystem, file_name);
    if (remove_result < 0)
    {
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error removing file/directory.\n");
        return remove_result;
    }

    // UART replaced
    LOG_message(
        LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Successfully removed file/directory.\n");
    return 0;
}

/**
 * @brief Creates directory
 * @param dir_name Pointer to cstring holding the name of the directory
 * @retval 0 on success, 1 if LFS is unmounted, negative LFS error codes on failure
 */
int8_t LFS_make_directory(const char dir_name[])
{
    if (!LFS_is_lfs_mounted)
    {
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "LittleFS not mounted.\n");
        return 1;
    }

    int8_t make_dir_result = lfs_mkdir(&LFS_filesystem, dir_name);
    if (make_dir_result < 0)
    {
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error creating directory.\n");
        return make_dir_result;
    }

    // UART replaced
    LOG_message(
        LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Successfully created directory.\n");
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
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "LittleFS not mounted.\n");
        return 1;
    }

    // Create or Open a file with Write only flag
    lfs_file_t file;
    const int8_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, file_name, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC, &LFS_file_cfg);

    if (open_result < 0)
    {
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error opening/creating file.\n");
        return open_result;
    }

    if (LFS_enable_hot_path_debug_logs)
    {
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Opened/created a file named: '");
        DEBUG_uart_print_str(file_name);
        // UART replaced
        // LOG_message(
        //  LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        //"'\n");
    }

    // Write data to file
    const int8_t write_result = lfs_file_write(&LFS_filesystem, &file, write_buffer, write_buffer_len);
    if (write_result < 0)
    {
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error writing to file!\n");
        return write_result;
    }

    if (LFS_enable_hot_path_debug_logs)
    {
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Successfully wrote data to file!\n");
    }

    // Close the File, the storage is not updated until the file is closed successfully
    const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result < 0)
    {
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error closing the file!\n");
        return close_result;
    }

    if (LFS_enable_hot_path_debug_logs)
    {
        // UART replaced
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Successfully closed the file!\n");
    }

    return 0;
}

/**
 * @brief Creates / Opens LittleFS File to append contents
 * @param file_name - Pointer to cstring holding the file name to create or open
 * @param write_buffer - Pointer to buffer holding the data to write
 * @param write_buffer_len - Size of the data to write
 * @retval 0 on success, 1 if LFS is unmounted, negative LFS error codes on failure
 */
int8_t LFS_append_file(const char file_name[], uint8_t *write_buffer, uint32_t write_buffer_len)
{
    if (!LFS_is_lfs_mounted)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "LittleFS not mounted");
        return 1;
    }

    lfs_file_t file;
    const int8_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, file_name, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND, &LFS_file_cfg);

    if (open_result < 0)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error opening file");
        return open_result;
    }

    const int8_t seek_result = lfs_file_seek(&LFS_filesystem, &file, 0, LFS_SEEK_END);
    if (seek_result < 0)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error seeking within file");
        return seek_result;
    }

    const int8_t write_result = lfs_file_write(&LFS_filesystem, &file, write_buffer, write_buffer_len);
    if (write_result < 0)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error writing to file");
        return write_result;
    }

    // Close the File, the storage is not updated until the file is closed successfully
    const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result < 0)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error closing file");
        return close_result;
    }

    return 0;
}

/**
 * @brief Opens LittleFS File to read from the Memory Module
 * @param file_name - Pointer to buffer holding the file name to open
 * @param offset - position within the file to read from
 * @param read_buffer - Pointer to buffer where the read data will be stored
 * @param read_buffer_len - Size of the data to read
 * @retval Returns negative values if read or file open failed, else the
 * number of bytes read
 */
lfs_ssize_t LFS_read_file(const char file_name[], lfs_soff_t offset, uint8_t *read_buffer, uint32_t read_buffer_len)
{
    lfs_file_t file;
    const int8_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, file_name, LFS_O_RDONLY, &LFS_file_cfg);
    if (open_result < 0)
    {
        // TODO: confirm behaviour is desired: this assumes filesystem as a
        // whole as an issue, so does not send log message to file
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error opening file to read");
        return open_result;
    }

    if (LFS_enable_hot_path_debug_logs)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_SINK_ALL, "Opened file to read: %s", file_name);
    }

    const lfs_soff_t seek_result = lfs_file_seek(&LFS_filesystem, &file, offset, LFS_SEEK_SET);
    if (seek_result < 0)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error seeking within file");
        return seek_result;
    }

    const lfs_ssize_t read_result = lfs_file_read(&LFS_filesystem, &file, read_buffer, read_buffer_len);
    if (read_result < 0)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error reading file");
        return read_result;
    }

    if (LFS_enable_hot_path_debug_logs)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_SINK_ALL, "Successfully read file");
    }

    // Close the File, the storage is not updated until the file is closed successfully
    const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result < 0)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error closing file");
        return close_result;
    }

    if (LFS_enable_hot_path_debug_logs)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Successfully close file");
    }

    return read_result;
}

/**
 * @brief Returns the file size
 * @param file_name - Pointer to buffer holding the file name to open
 * @retval Returns negative values if read or file open failed, else the
 * number of bytes in the file
 */
lfs_ssize_t LFS_file_size(const char file_name[])
{
    lfs_file_t file;
    const int8_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, file_name, LFS_O_RDONLY, &LFS_file_cfg);
    if (open_result < 0)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error opening file");
        return open_result;
    }
    const lfs_ssize_t size = lfs_file_size(&LFS_filesystem, &file);
    const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result < 0)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error closing file");
        return close_result;
    }
    return size;
}
