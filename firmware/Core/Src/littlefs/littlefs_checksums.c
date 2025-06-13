#include "littlefs/littlefs_checksums.h"

#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"
#include "crypto/sha256.h"
#include "log/log.h"
#include "debug_tools/debug_uart.h"

/// @brief Computes the SHA256 checksum of a file in LittleFS.
/// @param filepath Path to the file to read and compute the checksum for.
/// @param start_offset The offset in the file from which to start reading.
/// @param max_length The maximum number of bytes to read from the file. 0 means read the entire file.
/// @param sha256_dest 32-byte array to be filled with checksum.
/// @return 0 on success. Negative LFS error codes on error.
int8_t LFS_read_file_checksum_sha256(
    const char filepath[], uint32_t start_offset, uint32_t max_length,
    uint8_t sha256_dest[32]
) {
    const uint16_t chunk_size = 2048; // 2 KiB chunks
    uint8_t read_buffer[chunk_size];
    
    SHA256_CTX sha256_ctx;
    sha256_init(&sha256_ctx);

    lfs_file_t file;
    const int8_t open_result = lfs_file_opencfg(&LFS_filesystem, &file, filepath, LFS_O_RDONLY, &LFS_file_cfg);
    if (open_result < 0) {
        return open_result;
    }

    // Seek to the start offset
    const lfs_soff_t seek_result = lfs_file_seek(&LFS_filesystem, &file, start_offset, LFS_SEEK_SET);
    if (seek_result < 0) {
        lfs_file_close(&LFS_filesystem, &file);
        return seek_result;
    }

    // If max_length is 0, read the entire file.
    int32_t read_bytes_remaining = (max_length == 0 || max_length >= INT32_MAX) ? INT32_MAX : max_length;

    // Read in chunks and hash.
    int32_t total_calc_time_ms = 0;
    int32_t total_read_time_ms = 0;
    while (read_bytes_remaining > 0) {
        // Determine how many bytes to read in this chunk.
        const uint32_t bytes_to_read = (read_bytes_remaining < chunk_size) ? read_bytes_remaining : chunk_size;

        // Read the data from the file.
        const int32_t read_start_time = HAL_GetTick();
        const int32_t bytes_read = lfs_file_read(&LFS_filesystem, &file, read_buffer, bytes_to_read);
        total_read_time_ms += HAL_GetTick() - read_start_time;

        if (bytes_read < 0) {
            lfs_file_close(&LFS_filesystem, &file);
            return bytes_read; // Return error code
        }
        else if (bytes_read == 0) {
            // No data was read (end of file reached).
            // Can break out because there's nothing to update the SHA256 context with.
            break;
        }

        // Update the SHA256 context with the read data.
        const int32_t sha256_start_time = HAL_GetTick();
        sha256_update(&sha256_ctx, read_buffer, bytes_read);
        total_calc_time_ms += HAL_GetTick() - sha256_start_time;

        // Decrease the remaining bytes to read.
        read_bytes_remaining -= bytes_read;
    }

    // Close the file.
    const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result < 0) {
        return close_result; // Return error code
    }

    // Finalize the SHA256 hash.
    const int32_t sha256_final_start_time = HAL_GetTick();
    sha256_final(&sha256_ctx, sha256_dest);
    total_calc_time_ms += HAL_GetTick() - sha256_final_start_time;

    // Log the time taken for reading and calculating the checksum.
    LOG_message(
        LOG_SYSTEM_LFS, LOG_SEVERITY_DEBUG, LOG_all_sinks_except(LOG_SINK_FILE),
        "LFS_read_file_checksum_sha256: fs_read_time=%ldms, sha256_calc_time=%ldms",
        total_read_time_ms, total_calc_time_ms
    );

    return 0; // Success
}
