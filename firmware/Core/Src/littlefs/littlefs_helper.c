/*-----------------------------INCLUDES-----------------------------*/
#include <stdint.h>

#include "littlefs/littlefs_helper.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_driver.h"
#include "log/log.h"

/*-----------------------------VARIABLES-----------------------------*/
// Variables to track LittleFS on Flash Memory Module
uint8_t LFS_is_lfs_mounted = 0;

// NAND Flash Memory Datasheet https://www.farnell.com/datasheets/3151163.pdf
// Each page is divided into a 2048-byte data storage region, and a 128 bytes spare area (2176 bytes total).
#define FLASH_CHIP_PAGE_SIZE_BYTES 2048
#define FLASH_CHIP_BLOCK_SIZE_BYTES FLASH_CHIP_PAGE_SIZE_BYTES * FLASH_CHIP_PAGES_PER_BLOCK
#define FLASH_LOOKAHEAD_SIZE 16

// LittleFS Buffers for reading and writing
uint8_t LFS_read_buffer[FLASH_CHIP_PAGE_SIZE_BYTES];
uint8_t LFS_prog_buffer[FLASH_CHIP_PAGE_SIZE_BYTES];
uint8_t LFS_lookahead_buf[FLASH_LOOKAHEAD_SIZE];
uint8_t LFS_file_buffer[FLASH_CHIP_PAGE_SIZE_BYTES];

// TODO: look into the `LFS_F_INLINE` macro to increase the maximum number of files we can have

// Variables LittleFS uses for various functions (all externs in littlefs_helper.h)
lfs_t LFS_filesystem;
struct lfs_config LFS_cfg = {
    .read = LFS_block_device_read,
    .prog = LFS_block_device_prog,
    .erase = LFS_block_device_erase,
    .sync = LFS_block_device_sync,

    // block device configuration
    .read_size = FLASH_CHIP_PAGE_SIZE_BYTES,
    .prog_size = FLASH_CHIP_PAGE_SIZE_BYTES,
    .block_size = FLASH_CHIP_BLOCK_SIZE_BYTES,
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

// ----------------------------- LittleFS Functions -----------------------------

/// @brief Formats Memory Module so it can successfully mount LittleFS
/// @param None
/// @return 0 on success, negative LFS error codes on failure
int8_t LFS_format()
{
    int8_t format_result = lfs_format(&LFS_filesystem, &LFS_cfg);
    if (format_result < 0) {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error formatting FLASH memory!");
        return format_result;
    }
    
    LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "LittleFS Memory formatting successful!");
    return 0;
}

/// @brief Mounts LittleFS to the Memory Module
/// @param None
/// @return 0 on success, 1 if LFS is already mounted, negative LFS error codes on failure
int8_t LFS_mount() {
    if (LFS_is_lfs_mounted) {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "LittleFS already mounted!");
        return 1;
    }

    // Variable to store status of LittleFS mounting
    const int8_t mount_result = lfs_mount(&LFS_filesystem, &LFS_cfg);
    if (mount_result < 0) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE),
            "Error: lfs_mount() -> %d",
            mount_result
        );
        return mount_result;
    }

    LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "LittleFS mounting successful!");
    
    LFS_is_lfs_mounted = 1;
    return 0;
}

/// @brief Unmounts LittleFS to the Memory Module
/// @param None
/// @return 0 on success, 1 if LFS is already unmounted, negative LFS error codes on failure
int8_t LFS_unmount()
{
    if (!LFS_is_lfs_mounted) {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "LittleFS not mounted!");
        return 1;
    }

    // Unmount LittleFS to release any resources used by LittleFS
    const int8_t unmount_result = lfs_unmount(&LFS_filesystem);
    if (unmount_result < 0)
    {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE),
            "Error: lfs_unmount() -> %d",
            unmount_result
        );
        return unmount_result;
    }

    LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "LittleFS un-mounting successful!");
    LFS_is_lfs_mounted = 0;
    return 0;
}

/// @brief Lists contents of LittleFS Directory
/// @param root_directory cstring holding the root directory to open and read
/// @param offset Number of entries to skip before listing directory
/// @param count Number of entries to list in total (if 0, prints all entries)
/// @return 0 on success, 1 if LFS is unmounted, negative LFS error codes on failure
int8_t LFS_list_directory(const char root_directory[], uint16_t offset, int16_t count)
{
    // Check if LFS is mounted
    if (!LFS_is_lfs_mounted) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
            "LittleFS not mounted."
        );
        return 1;
    }

    // Try to open the directory
    lfs_dir_t dir;
    int8_t open_dir_result = lfs_dir_open(&LFS_filesystem, &dir, root_directory);
    if (open_dir_result < 0)
    {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE),
            "Error opening directory: %s", root_directory
        );
        return open_dir_result;
    }

    if (count == 0) {
        count = -1;
    }

    // Result is positive on success, 0 at the end of directory, or negative on failure.
    int8_t read_dir_result = 1;
    struct lfs_info info;
    LOG_message(
        LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "Name (Bytes)"
    );
    while (read_dir_result > 0)
    {
        read_dir_result = lfs_dir_read(&LFS_filesystem, &dir, &info);

        if (read_dir_result < 0)
        {
            LOG_message(
                LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE),
                "Error reading content from directory: %s", root_directory
            );
            break;
        }

        if (offset > 0) {
            offset--;
            continue;
        }

        if (count == 0) {
            break;
        } else {
            count--;
        }

        if (info.type == LFS_TYPE_REG)
        {
            LOG_message(
                LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE),
                "%s (%ld B)", info.name, info.size
            );
        } else if (info.type == LFS_TYPE_DIR){
            LOG_message(
                LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE),
                "%s/", info.name
            );
        }
    }

    int8_t close_dir_result = lfs_dir_close(&LFS_filesystem, &dir);
    if (close_dir_result < 0)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error closing directory: %s", root_directory);
        return close_dir_result;
    }

    if (read_dir_result < 0) {
        return read_dir_result;
    } else {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "Successfully listed contents from directory: %s", root_directory);
        return 0;
    }
}

/// @brief Creates directory
/// @param dir_name Pointer to cstring holding the name of the directory
/// @return 0 on success, 1 if LFS is unmounted, negative LFS error codes on failure
int8_t LFS_make_directory(const char dir_name[])
{
    if (!LFS_is_lfs_mounted)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "LittleFS not mounted!");
        return 1;
    }

    const int8_t make_dir_result = lfs_mkdir(&LFS_filesystem, dir_name);
    if (make_dir_result < 0)
    {
        if (make_dir_result == LFS_ERR_EXIST) {
            LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
            "Directory %s already exists.", dir_name);
        } else {
            LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
            "Recieved error: %d while creating directory: %s.", make_dir_result, dir_name);
        }
        return make_dir_result;
    }

    LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "Successfully created directory: %s", dir_name);
    return 0;
}

/// @brief Removes / deletes the file specified
/// @param file_name Pointer to cstring holding the file name to remove
/// @return 0 on success, 1 if LFS is unmounted, negative LFS error codes on failure
int8_t LFS_delete_file(const char file_name[])
{
    if (!LFS_is_lfs_mounted)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "LittleFS not mounted.");
        return 1;
    }

    int8_t remove_result = lfs_remove(&LFS_filesystem, file_name);
    if (remove_result < 0)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "Error removing file/directory: %s", file_name);
        return remove_result;
    }

    LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "Successfully removed file/directory: %s", file_name);
    return 0;
}

/// @brief Creates/Opens LittleFS File to write to the Memory Module
/// @param file_name - Pointer to cstring holding the file name to create or open
/// @param write_buffer - Pointer to buffer holding the data to write
/// @param write_buffer_len - Size of the data to write
/// @return 0 on success, 1 if LFS is unmounted, negative LFS error codes on failure
int8_t LFS_write_file(const char file_name[], uint8_t *write_buffer, uint32_t write_buffer_len)
{
    if (!LFS_is_lfs_mounted)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "LittleFS not mounted.");
        return 1;
    }

    // Create or Open a file with Write only flag
    lfs_file_t file;
    const int8_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, file_name, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC, &LFS_file_cfg);

    if (open_result < 0)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "Error opening/creating file: %s", file_name);
        return open_result;
    }
    
    LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "Opened/created file: %s", file_name);

    // Write data to file
    const lfs_ssize_t write_result = lfs_file_write(&LFS_filesystem, &file, write_buffer, write_buffer_len);
    if (write_result < 0)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "Error writing to file: %s", file_name);
        return write_result;
    }

    // Close the File, the storage is not updated until the file is closed successfully
    const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result < 0)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "Error closing file: %s", file_name);
        return close_result;
    }
    
    LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "Successfully closed file: %s", file_name);
    LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), "Successfully wrote data to file: %s", file_name);

    return 0;
}

/// @brief Creates/Opens LittleFS File to append contents
/// @param file_name - Pointer to cstring holding the file name to create or open
/// @param write_buffer - Pointer to buffer holding the data to write
/// @param write_buffer_len - Size of the data to write
/// @return 0 on success, 1 if LFS is unmounted, negative LFS error codes on failure
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
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error opening file: %s", file_name);
        return open_result;
    }
    
    const lfs_soff_t seek_result = lfs_file_seek(&LFS_filesystem, &file, 0, LFS_SEEK_END);
    if (seek_result < 0) {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error seeking within file: %s", file_name);
        return seek_result;
    }

    const lfs_ssize_t write_result = lfs_file_write(&LFS_filesystem, &file, write_buffer, write_buffer_len);
    if (write_result < 0) {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error writing to file %s", file_name);
        return write_result;
    }
    
    // Close the File, the storage is not updated until the file is closed successfully
    const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result < 0) {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error closing file: %s", file_name);
        return close_result;
    }
    
    return 0;
}

/// @brief Creates / Opens LittleFS File and writes data at a specific offset
/// @param file_name - Pointer to cstring holding the file name to create or open
/// @param offset - Position within the file to write the data
/// @param write_buffer - Pointer to buffer holding the data to write
/// @param write_buffer_len - Size of the data to write
/// @retval 0 on success, 1 if LFS is unmounted, negative LFS error codes on failure
int8_t LFS_write_file_with_offset(const char file_name[], lfs_soff_t offset, uint8_t *write_buffer, uint32_t write_buffer_len)
{
    if (!LFS_is_lfs_mounted)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "LittleFS not mounted");
        return 1;
    }

    // Open the file with read-write access, create if it doesn't exist
    lfs_file_t file;
    const int8_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, file_name, LFS_O_RDWR | LFS_O_CREAT, &LFS_file_cfg);
    if (open_result < 0)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), 
                   "Error opening file: %s (error: %d)", file_name, open_result);
        return open_result;
    }
    
    LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), 
               "Opened file for writing at offset: %s", file_name);

    // Get the current file size to determine if we need to extend it
    lfs_soff_t current_size = lfs_file_size(&LFS_filesystem, &file);
    if (current_size < 0)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), 
                   "Error getting file size: %s (error: %ld)", file_name, current_size);
        lfs_file_close(&LFS_filesystem, &file);
        return current_size;
    }

    // Check if we need to extend the file with zeros for a gap
    if (offset > current_size)
    {
        // Calculate the gap size between current file end and desired offset
        uint32_t gap_size = offset - current_size;
        
        // Fill the gap with zeros if needed
        if (gap_size > 0) 
        {
            // For efficiency, write zeros in chunks if the gap is large
            uint8_t zero_buffer[64] = {0}; // Buffer of zeros to write
            uint32_t chunk_size = sizeof(zero_buffer);
            
            LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), 
                       "Extending file from %ld to %ld bytes", current_size, offset);

            // Start writing zeros from the current file size to the desired offset
            const lfs_soff_t seek_result_1 = lfs_file_seek(&LFS_filesystem, &file, current_size, LFS_SEEK_SET);
            if (seek_result_1 < 0)
            {
                LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), 
                           "Error seeking to offset %ld in file: %s (error: %ld)", current_size, file_name, seek_result_1);
                lfs_file_close(&LFS_filesystem, &file);
                return seek_result_1;
            }
            
            while (gap_size > 0) 
            {
                uint32_t bytes_to_write = (gap_size > chunk_size) ? chunk_size : gap_size;
                
                const lfs_ssize_t write_zeros_result = lfs_file_write(&LFS_filesystem, &file, zero_buffer, bytes_to_write);
                if (write_zeros_result < 0) 
                {
                    LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), 
                               "Error extending file: %s (error: %ld)", file_name, write_zeros_result);
                    lfs_file_close(&LFS_filesystem, &file);
                    return write_zeros_result;
                }
                
                gap_size -= bytes_to_write;
            }
        }
    }
    else 
    {
        // Seek to the specified offset
        const lfs_soff_t seek_result_2 = lfs_file_seek(&LFS_filesystem, &file, offset, LFS_SEEK_SET);
        if (seek_result_2 < 0)
        {
            LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), 
                       "Error seeking to offset %ld in file: %s (error: %ld)", offset, file_name, seek_result_2);
            
            // Close the file before returning
            lfs_file_close(&LFS_filesystem, &file);
            return seek_result_2;
        }
    }

    // Write data to file at the specified offset
    const lfs_ssize_t write_result = lfs_file_write(&LFS_filesystem, &file, write_buffer, write_buffer_len);
    if (write_result < 0)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), 
                   "Error writing to file: %s at offset %ld (error: %ld)", file_name, offset, write_result);
        
        // Close the file before returning
        lfs_file_close(&LFS_filesystem, &file);
        return write_result;
    }

    // Close the File, the storage is not updated until the file is closed successfully
    const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result < 0)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE), 
                   "Error closing file: %s (error: %d)", file_name, close_result);
        return close_result;
    }
    
    LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), 
               "Successfully wrote %lu bytes to file: %s at offset %ld", write_buffer_len, file_name, offset);

    return 0;
}

/// @brief Opens LittleFS File to read from the Memory Module
/// @param file_name - Pointer to buffer holding the file name to open
/// @param offset - position within the file to read from
/// @param read_buffer - Pointer to buffer where the read data will be stored
/// @param read_buffer_len - Size of the data to read
/// @return Returns negative values if read or file open failed, else the
/// number of bytes read
lfs_ssize_t LFS_read_file(const char file_name[], lfs_soff_t offset, uint8_t *read_buffer, uint32_t read_buffer_len)
{
    if (!LFS_is_lfs_mounted)
    {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "LittleFS not mounted");
        return 1;
    }

    lfs_file_t file;
    const int8_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, file_name, LFS_O_RDONLY, &LFS_file_cfg);
    if (open_result < 0) {
        // TODO: confirm behaviour is desired: this assumes filesystem as a
        // whole as an issue, so does not send log message to file
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error opening file to read: %s", file_name);
        return open_result;
    }
    LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "Opened file to read: %s", file_name);

    const lfs_soff_t seek_result = lfs_file_seek(&LFS_filesystem, &file, offset, LFS_SEEK_SET);
    if (seek_result < 0) {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error seeking within file: %s", file_name);
        return seek_result;
    }

    const lfs_ssize_t read_result = lfs_file_read(&LFS_filesystem, &file, read_buffer, read_buffer_len);
    if (read_result < 0) {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error reading file: %s", file_name);
        return read_result;
    }
    LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "Successfully read file: %s", file_name);

    // Close the File, the storage is not updated until the file is closed successfully
    const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result < 0) {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error closing file: %s", file_name);
        return close_result;
    }
    LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "Successfully closed file: %s", file_name);

    return read_result;	
}

/// @brief Returns the file size
/// @param file_name - Pointer to buffer holding the file name to open
/// @return Returns negative values if read or file open failed, else the
/// number of bytes in the file
lfs_ssize_t LFS_file_size(const char file_name[])
{
    if (!LFS_is_lfs_mounted) {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "LittleFS not mounted");
        return -12512;
    }

    lfs_file_t file;
    const int8_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, file_name, LFS_O_RDONLY, &LFS_file_cfg);
    if (open_result < 0) {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error opening file: %s", file_name);
        return open_result;
    }
    LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "Successfully opened file: %s", file_name);

    const lfs_ssize_t size = lfs_file_size(&LFS_filesystem, &file);

    const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result < 0) {
        LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_CRITICAL, LOG_all_sinks_except(LOG_SINK_FILE), "Error closing file: %s", file_name);
        return close_result;
    }
    LOG_message(LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE), "Successfully closed file: %s", file_name);
    return size;
}