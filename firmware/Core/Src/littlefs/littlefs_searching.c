#include <stdint.h>
#include <stdlib.h>
#include "littlefs/lfs.h"

#include "littlefs/littlefs_searching.h"
#include "littlefs/littlefs_helper.h"

/// @brief Count the number of occurrences of a string/byte pattern in an LFS file.
/// @param filename File name/path to open.
/// @param needle Pattern to search for.
/// @param needle_len Length of pattern to search for.
/// @return The number of matches. 0 if no matches. Negative LFS error code on error.
int32_t LFS_search_count_occurrences(
    const char *filename,
    const uint8_t *needle,
    size_t needle_len
) {
    if (needle_len == 0 || filename == NULL) {
        return 0;
    }

    lfs_file_t file;
    const int32_t err_open = lfs_file_opencfg(
        &LFS_filesystem, &file, filename, LFS_O_RDONLY, 
        &LFS_file_cfg
    );
    if (err_open < 0) {
        return err_open;
    }

    uint8_t buf[64];
    size_t matched_chars_count = 0;
    int32_t count = 0;

    while (1) {
        const lfs_ssize_t r = lfs_file_read(&LFS_filesystem, &file, buf, sizeof(buf));
        if (r < 0) {
            lfs_file_close(&LFS_filesystem, &file);
            return (int32_t)r;
        }
        if (r == 0) {
            break; // EOF
        }

        for (lfs_ssize_t i = 0; i < r; i++) {
            if (buf[i] == needle[matched_chars_count]) {
                matched_chars_count++;
                if (matched_chars_count == needle_len) {
                    count++;
                    matched_chars_count = 0; // non-overlapping
                }
            } else {
                matched_chars_count = (buf[i] == needle[0]) ? 1 : 0;
            }
        }
    }

    const int32_t err_close = lfs_file_close(&LFS_filesystem, &file);
    if (err_close < 0) {
        return err_close;
    }

    return count;
}


/// @brief Find the byte offset of the nth occurrence of a string/byte pattern in an LFS file.
/// @param lfs Filesystem.
/// @param filename File name/path to open.
/// @param needle Pattern to search for.
/// @param needle_len Length of pattern to search for.
/// @param n Nth occurrence to find.
/// @param out_offset Pointer to store the byte offset of the found occurrence.
/// @return 0 on success, or a negative LFS error code, or a positive error code indicating a problem in function.
///     Returns 1 if invalid arguments. Returns 3 if needle not found.
int32_t LFS_search_find_nth_occurrence(
    const char *filename,
    const uint8_t *needle,
    size_t needle_len,
    uint16_t n,
    lfs_soff_t *out_offset
) {
    if (needle_len == 0 || n == 0 || out_offset == NULL || filename == NULL) {
        return 1;
    }

    lfs_file_t file;
    const int32_t err_open = lfs_file_opencfg(
        &LFS_filesystem, &file, filename, LFS_O_RDONLY, 
        &LFS_file_cfg
    );
    if (err_open < 0) {
        return err_open;
    }

    uint8_t buf[64];
    size_t matched_chars_count = 0;
    uint16_t found = 0;
    lfs_soff_t pos = 0;

    while (1) {
        const lfs_ssize_t r = lfs_file_read(&LFS_filesystem, &file, buf, sizeof(buf));
        if (r < 0) {
            lfs_file_close(&LFS_filesystem, &file);
            return (int32_t)r;
        }
        if (r == 0) {
            break; // EOF
        }

        for (lfs_ssize_t i = 0; i < r; i++, pos++) {
            if (buf[i] == needle[matched_chars_count]) {
                matched_chars_count++;
                if (matched_chars_count == needle_len) {
                    found++;
                    if (found == n) {
                        *out_offset = pos - (needle_len - 1);
                        lfs_file_close(&LFS_filesystem, &file); // Steamroll error here.
                        return 0;
                    }
                    matched_chars_count = 0;
                }
            } else {
                matched_chars_count = (buf[i] == needle[0]) ? 1 : 0;
            }
        }
    }

    const int32_t err_close = lfs_file_close(&LFS_filesystem, &file);
    if (err_close < 0) {
        return err_close;
    }
    
    // Error: Final case where the nth occurrence was not found.
    return 3;
}
