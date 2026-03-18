#include "littlefs/littlefs_telecommands.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"

#include <stdint.h>
#include <stdio.h>

uint32_t LFS_debug_malloc_total_count = 0;
uint32_t LFS_debug_malloc_failed_count = 0;
uint32_t LFS_debug_free_total_count = 0;


int32_t LFS_list_directory_json_dict(
    const char root_directory[],
    uint16_t offset, int16_t count,
    char *json_output_buf, uint16_t json_output_buf_size
) {
    if (!LFS_is_lfs_mounted) {
        return 1;
    }

    lfs_dir_t dir;
    if (lfs_dir_open(&LFS_filesystem, &dir, root_directory) < 0) {
        return -1;
    }

    if (count == 0) {
        count = -1;
    }

    struct lfs_info info;
    int8_t read_result = 1;
    uint16_t buf_used = 0;

    // Start JSON object
    buf_used += snprintf(json_output_buf + buf_used, json_output_buf_size - buf_used, "{");

    bool first_entry = true;
    while ((read_result = lfs_dir_read(&LFS_filesystem, &dir, &info)) > 0) {
        if (offset > 0) {
            offset--;
            continue;
        }
        if (count == 0) {
            break;
        }
        if (count > 0) {
            count--;
        }

        // Entry separator
        if (!first_entry) {
            buf_used += snprintf(json_output_buf + buf_used, json_output_buf_size - buf_used, ",");
        }
        first_entry = false;

        // Construct full path
        char full_path[LFS_MAX_PATH_LENGTH * 3];
        if (strcmp(root_directory, "/") == 0) {
            // root_directory is exactly "/", so avoid double slash
            snprintf(full_path, sizeof(full_path), "/%s", info.name);
        } else {
            // Copy root_directory to a temporary buffer to trim any trailing slash
            char trimmed_root[LFS_MAX_PATH_LENGTH];
            strncpy(trimmed_root, root_directory, sizeof(trimmed_root) - 1);
            trimmed_root[sizeof(trimmed_root) - 1] = '\0';

            // Trim trailing slash if present
            size_t len = strlen(trimmed_root);
            if (len > 0 && trimmed_root[len - 1] == '/') {
                trimmed_root[len - 1] = '\0';
            }

            // Combine trimmed root and filename
            snprintf(
                full_path, sizeof(full_path), "%s%s/%s",
                (trimmed_root[0] == '/') ? "" : "/", // Ensure leading slash
                trimmed_root,
                info.name
            );
        }

        if (info.type == LFS_TYPE_REG) {
            buf_used += snprintf(
                json_output_buf + buf_used, json_output_buf_size - buf_used,
                "\"%s\":%" PRIu32, full_path, info.size
            );
        } else if (info.type == LFS_TYPE_DIR) {
            buf_used += snprintf(
                json_output_buf + buf_used, json_output_buf_size - buf_used,
                "\"%s/\":null", full_path
            );
        }

        if (buf_used >= json_output_buf_size) {
            lfs_dir_close(&LFS_filesystem, &dir);
            return -2; // Buffer overflow
        }
    }

    // Close JSON object
    buf_used += snprintf(json_output_buf + buf_used, json_output_buf_size - buf_used, "}");

    const int32_t close_result = lfs_dir_close(&LFS_filesystem, &dir);
    if (close_result < 0) {
        return close_result;
    }

    if (read_result < 0) {
        return read_result;
    }

    return 0;
}

/// @brief Generate a JSON string containing filesystem stats and debugging info.
/// @param json_output_buf 
/// @param json_output_buf_size 
/// @return 0 on success. Negative LFS error codes. Positive other error codes.
/// @note Used space may be approximate, due to the nature of LittleFS Copy-on-Write (COW).
int8_t LFS_get_filesystem_stats_json(
    char *json_output_buf, uint16_t json_output_buf_size
) {
    // Get filesystem info.
    struct lfs_fsinfo fs_info;
    const int err = lfs_fs_stat(&LFS_filesystem, &fs_info);
    if (err < 0) {
        return (int8_t)err;
    }

    // Get filesystem size (allocated blocks)
    const lfs_ssize_t fs_used_size_blocks = lfs_fs_size(&LFS_filesystem);
    if (fs_used_size_blocks < 0) {
        return fs_used_size_blocks;
    }

    // Format JSON
    const int written = snprintf(
        json_output_buf,
        json_output_buf_size,
        "{"
            "\"fs_used_size_bytes\":%ld,"
            "\"fs_total_size_bytes\":%ld,"
            "\"used_percent\":%.2f,"
            "\"count_malloc_total\":%" PRIu32 ","
            "\"count_malloc_failed\":%" PRIu32 ","
            "\"count_free_total\":%" PRIu32 ","
            "\"fs_used_size_blocks\":%ld,"
            "\"disk_version\":%" PRIu32 ","
            "\"block_size\":%lu,"
            "\"block_count\":%lu,"
            "\"name_max\":%lu,"
            "\"file_max\":%lu,"
            "\"attr_max\":%lu"
        "}",
        fs_used_size_blocks * fs_info.block_size, // fs_used_size_bytes
        fs_info.block_count * fs_info.block_size, // fs_total_size_bytes
        fs_used_size_blocks * 100.0 / fs_info.block_count, // used_percent
        LFS_debug_malloc_total_count, // count_malloc_total
        LFS_debug_malloc_failed_count, // count_malloc_failed
        LFS_debug_free_total_count, // count_free_total
        fs_used_size_blocks, // fs_used_size_blocks
        fs_info.disk_version,
        fs_info.block_size,
        fs_info.block_count,
        fs_info.name_max,
        fs_info.file_max,
        fs_info.attr_max
    );

    // Check for truncation or encoding error
    if (written < 0 || written >= json_output_buf_size) {
        return 2; // buffer too small or encoding error
    }

    return 0; // Success.
}
