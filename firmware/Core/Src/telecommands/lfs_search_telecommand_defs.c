#include <stdio.h>
#include <stdint.h>

#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"
#include "littlefs/littlefs_searching.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "transforms/arrays.h"

/// @brief Count string occurrences in a file
/// @param args_str
/// - Arg 0: File path as string (haystack)
/// - Arg 1: Needle string to search for
uint8_t TCMDEXEC_fs_count_str_occurrences(
    const char *args_str,
    char *response_output_buf,
    uint16_t response_output_buf_len
) {
    char arg_file_path[LFS_MAX_PATH_LENGTH];
    char arg_needle[256];

    const uint8_t parse_file_result = TCMD_extract_string_arg(
        args_str, 0, arg_file_path, sizeof(arg_file_path)
    );
    if (parse_file_result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
                 "Error parsing file path: %d", parse_file_result);
        return 1;
    }

    const uint8_t parse_needle_result = TCMD_extract_string_arg(
        args_str, 1, arg_needle, sizeof(arg_needle)
    );
    if (parse_needle_result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
                 "Error parsing needle string: %d", parse_needle_result);
        return 2;
    }

    const int32_t count = LFS_search_count_occurrences(
        arg_file_path,
        (const uint8_t *)arg_needle,
        strlen(arg_needle)
    );

    if (count < 0) {
        snprintf(response_output_buf, response_output_buf_len,
                 "Search error: %ld", count);
        return 3;
    }

    snprintf(response_output_buf, response_output_buf_len, "Found %ld occurrences", count);
    return 0;
}

/// @brief Find Nth string occurrence in a file
/// @param args_str
/// - Arg 0: File path as string (haystack)
/// - Arg 1: Needle string to search for
/// - Arg 2: N (1-based)
uint8_t TCMDEXEC_fs_find_nth_str_occurrence(
    const char *args_str,
    char *response_output_buf,
    uint16_t response_output_buf_len
) {
    char arg_file_path[LFS_MAX_PATH_LENGTH];
    char arg_needle[256];
    uint64_t n;

    const uint8_t parse_file_result = TCMD_extract_string_arg(
        args_str, 0, arg_file_path, sizeof(arg_file_path)
    );
    if (parse_file_result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
                 "Error parsing file path: %d", parse_file_result);
        return 1;
    }

    const uint8_t parse_needle_result = TCMD_extract_string_arg(
        args_str, 1, arg_needle, sizeof(arg_needle)
    );
    if (parse_needle_result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
                 "Error parsing needle string: %d", parse_needle_result);
        return 2;
    }

    const uint8_t parse_n_result = TCMD_extract_uint64_arg(
        args_str, strlen(args_str), 2, &n
    );
    if (parse_n_result != 0 || n == 0 || n > UINT16_MAX) {
        snprintf(response_output_buf, response_output_buf_len,
                 "Invalid occurrence index");
        return 3;
    }

    lfs_soff_t output_offset = 0; // Final output.
    const int32_t result = LFS_search_find_nth_occurrence(
        arg_file_path,
        (const uint8_t *)arg_needle,
        strlen(arg_needle),
        (uint16_t)n,
        &output_offset
    );

    if (result < 0) {
        snprintf(response_output_buf, response_output_buf_len,
                 "Search error: %ld", result);
        return 4;
    }

    snprintf(
        response_output_buf, response_output_buf_len,
        "Occurrence #%d found at Offset %ld",
        (uint16_t)n, output_offset
    );
    return 0;
}

/// @brief Count hex byte sequence occurrences in a file
/// @param args_str
/// - Arg 0: File path as string (haystack)
/// - Arg 1: Hex string to search for (e.g. "DEADBEEF")
uint8_t TCMDEXEC_fs_count_hex_occurrences(
    const char *args_str,
    char *response_output_buf,
    uint16_t response_output_buf_len
) {
    char arg_file_path[LFS_MAX_PATH_LENGTH];
    uint8_t needle[256];
    uint16_t needle_len = 0;

    const uint8_t parse_file_result = TCMD_extract_string_arg(
        args_str, 0, arg_file_path, sizeof(arg_file_path)
    );
    if (parse_file_result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
                 "Error parsing file path: %d", parse_file_result);
        return 1;
    }

    const uint8_t parse_hex_result = TCMD_extract_hex_array_arg(
        args_str, 1, needle, sizeof(needle), &needle_len
    );
    if (parse_hex_result != 0 || needle_len == 0) {
        snprintf(response_output_buf, response_output_buf_len,
                 "Error parsing hex needle");
        return 2;
    }

    const int32_t count = LFS_search_count_occurrences(
        arg_file_path,
        needle,
        needle_len
    );

    if (count < 0) {
        snprintf(response_output_buf, response_output_buf_len,
                 "Search error: %ld", count);
        return 3;
    }

    snprintf(response_output_buf, response_output_buf_len, "Found %ld occurrences", count);
    return 0;
}

/// @brief Find Nth hex byte sequence occurrence in a file
/// @param args_str
/// - Arg 0: File path as string (haystack)
/// - Arg 1: Hex string to search for
/// - Arg 2: N (1-based)
uint8_t TCMDEXEC_fs_find_nth_hex_occurrence(
    const char *args_str,
    char *response_output_buf,
    uint16_t response_output_buf_len
) {
    char arg_file_path[LFS_MAX_PATH_LENGTH];
    uint8_t needle[256];
    uint16_t needle_len = 0;
    uint64_t n;

    const uint8_t parse_file_result = TCMD_extract_string_arg(
        args_str, 0, arg_file_path, sizeof(arg_file_path)
    );
    if (parse_file_result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
                 "Error parsing file path: %d", parse_file_result);
        return 1;
    }

    const uint8_t parse_hex_result = TCMD_extract_hex_array_arg(
        args_str, 1, needle, sizeof(needle), &needle_len
    );
    if (parse_hex_result != 0 || needle_len == 0) {
        snprintf(response_output_buf, response_output_buf_len,
                 "Error parsing hex needle");
        return 2;
    }

    const uint8_t parse_n_result = TCMD_extract_uint64_arg(
        args_str, strlen(args_str), 2, &n
    );
    if (parse_n_result != 0 || n == 0 || n > UINT16_MAX) {
        snprintf(response_output_buf, response_output_buf_len,
                 "Invalid occurrence index");
        return 3;
    }

    lfs_soff_t output_offset = 0;
    const int32_t result = LFS_search_find_nth_occurrence(
        arg_file_path,
        needle,
        needle_len,
        (uint16_t)n,
        &output_offset
    );

    if (result < 0) {
        snprintf(response_output_buf, response_output_buf_len,
                 "Search error: %ld", result);
        return 4;
    }

    snprintf(
        response_output_buf, response_output_buf_len,
        "Occurrence #%d found at Offset %ld",
        (uint16_t)n, output_offset
    );
    return 0;
}
