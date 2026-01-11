#include <stdint.h>
#include <stdlib.h>
#include "littlefs/lfs.h"

/// @brief Count the number of occurrences of a string/byte pattern in an LFS file.
/// @param lfs Filesystem.
/// @param file Open file. Must be opened in read mode.
/// @param needle Pattern to search for.
/// @param needle_len Length of pattern to search for.
/// @return The number of matches. 0 if no matches. Negative LFS error code on error.
int32_t LFS_search_count_occurrences(
    lfs_t *lfs,
    lfs_file_t *file,
    const uint8_t *needle,
    size_t needle_len
) {
    if (needle_len == 0) {
        return 0;
    }

    const lfs_soff_t err = lfs_file_seek(lfs, file, 0, LFS_SEEK_SET);
    if (err < 0) {
        return err;
    }

    uint8_t buf[64];
    size_t matched_chars_count = 0;
    int count = 0;

    while (1) {
        const lfs_ssize_t r = lfs_file_read(lfs, file, buf, sizeof(buf));
        if (r < 0) {
            return (int)r;  // Propagate read error.
        }
        if (r == 0) {
            // Reached EOF (end-of-file).
            break;
        }

        for (lfs_ssize_t i = 0; i < r; i++) {
            if (buf[i] == needle[matched_chars_count]) {
                matched_chars_count++;
                if (matched_chars_count == needle_len) {
                    count++;
                    matched_chars_count = 0;  // non-overlapping
                }
            } else {
                matched_chars_count = (buf[i] == needle[0]) ? 1 : 0;
            }
        }
    }

    return count;
}


/// @brief Find the byte offset of the nth occurrence of a string/byte pattern in an LFS file.
/// @param lfs Filesystem.
/// @param file Open file. Must be opened in read mode.
/// @param needle Pattern to search for.
/// @param needle_len Length of pattern to search for.
/// @param n Nth occurrence to find.
/// @param out_offset Pointer to store the byte offset of the found occurrence.
/// @return 0 on success, or a negative LFS error code, or a positive error code indicating a problem in function.
///     Returns 1 if invalid arguments. Returns 3 if needle not found.
int32_t LFS_search_find_nth_occurrence(
    lfs_t *lfs,
    lfs_file_t *file,
    const uint8_t *needle,
    size_t needle_len,
    uint16_t n,
    lfs_soff_t *out_offset
) {
    if (needle_len == 0 || n == 0 || out_offset == NULL) {
        return 1;
    }

    int err = lfs_file_seek(lfs, file, 0, LFS_SEEK_SET);
    if (err < 0) {
        return err;
    }

    uint8_t buf[64];
    size_t matched_chars_count = 0;
    uint16_t found = 0;
    lfs_soff_t pos = 0;

    while (1) {
        lfs_ssize_t r = lfs_file_read(lfs, file, buf, sizeof(buf));
        if (r < 0) {
            return (int)r;  // Propagate read error.
        }
        if (r == 0) {
            // Reached EOF (end-of-file).
            break;
        }

        for (lfs_ssize_t i = 0; i < r; i++, pos++) {
            if (buf[i] == needle[matched_chars_count]) {
                matched_chars_count++;
                if (matched_chars_count == needle_len) {
                    found++;
                    if (found == n) {
                        // Nominal successful exit.
                        *out_offset = pos - (needle_len - 1);
                        return 0;
                    }
                    matched_chars_count = 0;
                }
            } else {
                matched_chars_count = (buf[i] == needle[0]) ? 1 : 0;
            }
        }
    }

    // Error: Final case where the nth occurrence was not found.
    return 3;
}
