#include "littlefs/littlefs_telecommands.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"

#include <stdint.h>
#include <stdio.h>

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
        char full_path[LFS_MAX_PATH_LENGTH + 100];
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

