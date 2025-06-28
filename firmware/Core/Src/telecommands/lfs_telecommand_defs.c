
#include <stdio.h>
#include <stdint.h>

#include "debug_tools/debug_uart.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"
#include "littlefs/littlefs_telecommands.h"
#include "littlefs/littlefs_benchmark.h"
#include "littlefs/littlefs_checksums.h"
#include "log/log.h"
#include "telecommands/lfs_telecommand_defs.h"
#include "telecommand_exec/telecommand_definitions.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "transforms/arrays.h"
#include "stm32/stm32_internal_flash_drivers.h"

uint8_t TCMDEXEC_fs_format_storage(
    const char *args_str,
        char *response_output_buf, uint16_t response_output_buf_len
) {
    LFS_ensure_unmounted();

    const int8_t result = LFS_format();
    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: LFS_format() -> %d", result);
        return 1;
    }
    
    snprintf(response_output_buf, response_output_buf_len, "LittleFS successfully formatted");
    return 0;
}

uint8_t TCMDEXEC_fs_mount(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    const int8_t result = LFS_mount();
    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: LFS_mount() -> %d", result);
        return 1;
    }
    
    snprintf(response_output_buf, response_output_buf_len, "LittleFS successfully mounted");
    return 0;
}

uint8_t TCMDEXEC_fs_unmount(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    const int8_t result = LFS_unmount();
    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: LFS_unmount() -> %d", result);
        return 1;
    }
    
    snprintf(response_output_buf, response_output_buf_len, "LittleFS successfully unmounted");
    return 0;
}

/// @brief Telecommand: List all the files and directories within a given directory
/// @param args_str
/// - Arg 0: Root Directory path as string
/// - Arg 1: (Offset) Number of entries to skip at the beginning
/// - Arg 2: (Count) Number entries to display
uint8_t TCMDEXEC_fs_list_directory(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    char arg_root_directory_path[LFS_MAX_PATH_LENGTH];
    const uint8_t parse_directory_path_result = TCMD_extract_string_arg(
        args_str, 0, arg_root_directory_path, sizeof(arg_root_directory_path)
    );
    if (parse_directory_path_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing directory path arg: Error %d", parse_directory_path_result);
        return 1;
    }

    uint64_t arg_listing_offset = 0;
    const uint8_t parse_listing_offset_result = TCMD_extract_uint64_arg(
        args_str, strlen(args_str), 1, &arg_listing_offset
    );
    if (parse_listing_offset_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing offset arg: Error %d", parse_listing_offset_result);
        return 1;
    }

    uint64_t arg_listing_count = 0;
    const uint8_t parse_listing_count_result = TCMD_extract_uint64_arg(
        args_str, strlen(args_str), 2, &arg_listing_count
    );
    if (parse_listing_count_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing count arg: Error %d", parse_listing_count_result);
        return 1;
    }

    const int8_t list_directory_result = LFS_list_directory(
        arg_root_directory_path, (uint16_t) arg_listing_offset, (int16_t) arg_listing_count
    );
    if (list_directory_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "LittleFS List Directory Error: %d", list_directory_result
        );
        return 1;
    }
    
    return 0;
}

/// @brief Telecommand: List all the files and directories within a given directory, as a JSON dict (key are paths, values are sizes)
/// @param args_str
/// - Arg 0: Root Directory path as string
/// - Arg 1: (Offset) Number of entries to skip at the beginning
/// - Arg 2: (Count) Number entries to display
/// @note In the resulting JSON, the JSON value of directories is "null".
uint8_t TCMDEXEC_fs_list_directory_json(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    char arg_root_directory_path[LFS_MAX_PATH_LENGTH];
    const uint8_t parse_directory_path_result = TCMD_extract_string_arg(
        args_str, 0, arg_root_directory_path, sizeof(arg_root_directory_path)
    );
    if (parse_directory_path_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing directory path arg: Error %d", parse_directory_path_result);
        return 1;
    }

    uint64_t arg_listing_offset = 0;
    const uint8_t parse_listing_offset_result = TCMD_extract_uint64_arg(
        args_str, strlen(args_str), 1, &arg_listing_offset
    );
    if (parse_listing_offset_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing offset arg: Error %d", parse_listing_offset_result);
        return 2;
    }

    uint64_t arg_listing_count = 0;
    const uint8_t parse_listing_count_result = TCMD_extract_uint64_arg(
        args_str, strlen(args_str), 2, &arg_listing_count
    );
    if (parse_listing_count_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing count arg: Error %d", parse_listing_count_result);
        return 3;
    }

    const int8_t list_directory_result = LFS_list_directory_json_dict(
        arg_root_directory_path, (uint16_t) arg_listing_offset, (int16_t) arg_listing_count,
        response_output_buf, response_output_buf_len
    );
    if (list_directory_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "LittleFS List Directory Error: %d", list_directory_result
        );
        return 4;
    }
    
    return 0;
}

/// @brief Telecommand: Create a directory
/// @param args_str
/// - Arg 0: Directory Name as string (e.g., "/dir1", "/dir1/subdir1")
uint8_t TCMDEXEC_fs_make_directory(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len
) {
    char arg_root_directory_path[LFS_MAX_PATH_LENGTH];
    const uint8_t parse_directory_path_result = TCMD_extract_string_arg(
        args_str, 0, arg_root_directory_path, sizeof(arg_root_directory_path)
    );
    if (parse_directory_path_result != 0) {
        // Error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing directory path arg: Error %d", parse_directory_path_result);
        return 1;
    }

    const int8_t make_directory_result = LFS_make_directory(arg_root_directory_path);
    if (make_directory_result != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: LFS_make_directory() -> %d", make_directory_result);
        return 1;
    }
    
    return 0;
}

/// @brief Telecommand: Write data to a file in LittleFS
/// @param args_str
/// - Arg 0: File path as string
/// - Arg 1: String to write to file (up to 512 bytes)
uint8_t TCMDEXEC_fs_write_file_str(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    char arg_file_name[LFS_MAX_PATH_LENGTH];
    const uint8_t parse_file_name_result = TCMD_extract_string_arg(args_str, 0, arg_file_name, sizeof(arg_file_name));
    if (parse_file_name_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing file name arg: Error %d", parse_file_name_result);
        return 1;
    }

    char arg_file_content[512] = {0};
    const uint8_t parse_file_content_result = TCMD_extract_string_arg(args_str, 1, arg_file_content, sizeof(arg_file_content));
    if (parse_file_content_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing file content arg: Error %d", parse_file_content_result);
        return 2;
    }

    const int8_t result = LFS_write_file(arg_file_name, (uint8_t*) arg_file_content, strlen(arg_file_content));
    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: LFS_write_file() -> %d", result);
        return 1;
    }
    
    snprintf(response_output_buf, response_output_buf_len, "LittleFS write successful!");
    return 0;
}

/// @brief Telecommand: Write hex data to a file in LittleFS with offset support
/// @param args_str
/// - Arg 0: File path as string
/// - Arg 1: Offset within the file to start writing (uint64)
/// - Arg 2: Hex string to write to file (e.g., "DEADBEEF" or "DE AD BE EF")
/// @note The maximum number of bytes that can be written is 105 bytes
uint8_t TCMDEXEC_fs_write_file_hex(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    char arg_file_name[LFS_MAX_PATH_LENGTH];
    const uint8_t parse_file_name_result = TCMD_extract_string_arg(args_str, 0, arg_file_name, sizeof(arg_file_name));
    if (parse_file_name_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing file name arg: Error %d", parse_file_name_result);
        return 1;
    }

    // Extract the offset parameter
    uint64_t file_offset = 0;
    const uint8_t parse_offset_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &file_offset);
    if (parse_offset_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing offset arg: Error %d", parse_offset_result);
        return 2;
    }

    // Buffer to hold the converted hex data
    uint8_t binary_data[105] = {0};
    uint16_t binary_data_length = 0;
    
    // Extract and convert hex string to binary data
    const uint8_t parse_hex_result = TCMD_extract_hex_array_arg(
        args_str, 2, binary_data, sizeof(binary_data), &binary_data_length
    );
    
    if (parse_hex_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing hex data arg: Error %d", parse_hex_result);
        return 3;
    }

    // Use our new helper function to write the data at the specified offset
    const int8_t result = LFS_write_file_with_offset(arg_file_name, (lfs_soff_t)file_offset, binary_data, binary_data_length);
    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS Writing Error: %d", result);
        return 4;
    }
    
    snprintf(response_output_buf, response_output_buf_len, 
             "LittleFS Successfully Wrote %d bytes of hex data!", 
             binary_data_length);
    return 0;
}

/// @brief Telecommand: Deletes a specified file in LittleFS
/// @param args_str
/// - Arg 0: File name to be deleted
/// @note Do not add quotations around the argument, write as is.
uint8_t TCMDEXEC_fs_delete_file(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    char arg_file_name[LFS_MAX_PATH_LENGTH];
    const uint8_t parse_file_name_result = TCMD_extract_string_arg(args_str, 0, arg_file_name, sizeof(arg_file_name));
    if (parse_file_name_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing file name arg: Error %d", parse_file_name_result);
        return 1;
    }

    int8_t result = LFS_delete_file(arg_file_name);
    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: LFS_delete_file() -> %d", result);
        return 1;
    }

    snprintf(response_output_buf, response_output_buf_len, "LittleFS successfully deleted file!");
    return 0;
}

/// @brief Telecommand: Deletes a specified directory in LittleFS
/// @param args_str
/// - Arg 0: Directory name to be deleted
/// @note Do not add quotations around the argument, write as is.
uint8_t TCMDEXEC_fs_delete_dir(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    char arg_file_name[LFS_MAX_PATH_LENGTH];
    const uint8_t parse_dir_name_result = TCMD_extract_string_arg(args_str, 0, arg_file_name, sizeof(arg_file_name));
    if (parse_dir_name_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing dir name arg: Error %d", parse_dir_name_result);
        return 1;
    }

    const int8_t result = LFS_recursively_delete_directory(arg_file_name);
    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: LFS_recursively_delete_directory() -> %d", result);
        return 2;
    }

    snprintf(response_output_buf, response_output_buf_len, "LittleFS successfully deleted directory!");
    return 0;
}

/// @brief Parse a telecommand argument str for the standard filename/offset/length arguments for reading files.
/// @param args_str 
/// @param dest_filename 
/// @param dest_filename_size 
/// @param dest_offset 
/// @param dest_length 
/// @return 0 on success, >0 on error
static uint8_t parse_arg_str_for_file_offset_length(
    const char args_str[], char *dest_filename, uint16_t dest_filename_size, uint32_t *dest_offset, uint32_t *dest_length
) {
    // Extract the file name
    const uint8_t parse_file_name_result = TCMD_extract_string_arg(args_str, 0, dest_filename, dest_filename_size);
    if (parse_file_name_result != 0) {
        // error parsing
        return parse_file_name_result;
    }

    // Extract the offset parameter
    uint64_t offset_u64;
    const uint8_t parse_offset_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &offset_u64);
    if (parse_offset_result != 0) {
        // error parsing
        return parse_offset_result;
    }
    if (offset_u64 > UINT32_MAX) {
        // offset too large
        return 40;
    }
    *dest_offset = (uint32_t)offset_u64;

    // Extract the length parameter
    uint64_t length_u64;
    const uint8_t parse_length_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 2, &length_u64);
    if (parse_length_result != 0) {
        // error parsing
        return parse_length_result;
    }
    if (length_u64 > UINT32_MAX) {
        // length too large
        return 41;
    }
    *dest_length = (uint32_t)length_u64;

    return 0;
}

/// @brief Reads a file from LittleFS, and responds with its contents as 2-digit hex bytes (no spaces).
/// @param args_str
/// - Arg 0: File path as string
/// - Arg 1: Start offset (bytes). Nominally, pick 0.
/// - Arg 2: Length to read (bytes). 0 to read max.
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_fs_read_file_hex(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    char arg_file_name[LFS_MAX_PATH_LENGTH];
    uint32_t file_offset = 0;
    uint32_t max_length_bytes;

    const uint8_t parse_result = parse_arg_str_for_file_offset_length(
        args_str, arg_file_name, sizeof(arg_file_name), &file_offset, &max_length_bytes
    );
    if (parse_result != 0) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing file name/offset/length args: Error %d", parse_result);
        return 1;
    }
    
    // Restrict the read length.
    if ((max_length_bytes > (response_output_buf_len/2 - 1u)) || (max_length_bytes == 0)) {
        // Ensure we have space for null-termination.
        max_length_bytes = response_output_buf_len/2 - 1u;
    }

    uint8_t read_buf[max_length_bytes];
    const int32_t read_result = LFS_read_file(
        arg_file_name, file_offset, read_buf, max_length_bytes
    );
    if (read_result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error reading file: %ld", read_result);
        return 10;
    }

    // Note: `read_result` is now the number of bytes read into `read_buf`.
    GEN_byte_array_to_hex_str(read_buf, read_result, response_output_buf, response_output_buf_len);
    return 0;
}

/// @brief Reads a file from LittleFS, and responds with its contents as a string.
/// @param args_str
/// - Arg 0: File path as string
/// - Arg 1: Start offset (bytes). Nominally, pick 0.
/// - Arg 2: Length to read (bytes). 0 to read max.
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_fs_read_text_file(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    char arg_file_name[LFS_MAX_PATH_LENGTH];
    uint32_t file_offset = 0;
    uint32_t max_length;

    const uint8_t parse_result = parse_arg_str_for_file_offset_length(
        args_str, arg_file_name, sizeof(arg_file_name), &file_offset, &max_length
    );
    if (parse_result != 0) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing file name/offset/length args: Error %d", parse_result);
        return 1;
    }

    // Restrict the read length.
    if ((max_length > (response_output_buf_len - 1u)) || (max_length == 0)) {
        // Ensure we have space for null-termination.
        max_length = response_output_buf_len - 1;
    }

    // Read the file directly into the response buffer.
    const int32_t read_result = LFS_read_file(arg_file_name, file_offset, (uint8_t*)response_output_buf, response_output_buf_len-1);
    if (read_result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error reading file: %ld", read_result);
        return 10;
    }

    // Ensure null-termination.
    response_output_buf[response_output_buf_len - 1] = '\0';
    
    return 0;
}

/// @brief Calculates the SHA256 hash of a file in LittleFS and returns it as a little-endian hex string.
/// @param args_str
/// - Arg 0: File path as string
/// - Arg 1: Start offset (bytes). Nominally, pick 0.
/// - Arg 2: Length to read (bytes). 0 to read max.
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_fs_read_file_sha256_hash_json(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    char arg_file_name[LFS_MAX_PATH_LENGTH];
    uint32_t file_offset = 0;
    uint32_t max_length;

    const uint8_t parse_result = parse_arg_str_for_file_offset_length(
        args_str, arg_file_name, sizeof(arg_file_name), &file_offset, &max_length
    );
    if (parse_result != 0) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing file name/offset/length args: Error %d",
            parse_result
        );
        return 1;
    }

    // Prepare the SHA256 destination buffer.
    uint8_t sha256_dest[32] = {0}; // 32 bytes for SHA256

    // Calculate the SHA256 hash of the file.
    const int8_t sha256_result = LFS_read_file_checksum_sha256(
        arg_file_name, file_offset, max_length, sha256_dest
    );

    if (sha256_result != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error calculating SHA256: Err=%d", sha256_result);
        return 2;
    }

    // Fetch the full file size.
    const int32_t file_size_bytes = LFS_file_size(arg_file_name);
    if (file_size_bytes < 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error getting file size: Err=%ld", file_size_bytes);
        return 3;
    }
    const uint32_t real_length_hashed = (max_length == 0 || max_length >= (uint32_t)file_size_bytes) ? (uint32_t)file_size_bytes : max_length;

    // Convert the SHA256 hash to a little-endian hex string.
    char hex_hash_str[100]; // Should be 64 chars.
    GEN_byte_array_to_hex_str(sha256_dest, sizeof(sha256_dest), hex_hash_str, sizeof(hex_hash_str));

    // Format like JSON.
    snprintf(
        response_output_buf, response_output_buf_len,
        "{\"sha256\":\"%s\",\"offset\":%lu,\"length\":%lu,\"file_size\": %ld}",
        hex_hash_str,
        file_offset, real_length_hashed,
        file_size_bytes
    );
    return 0;
}


uint8_t TCMDEXEC_fs_demo_write_then_read(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    char file_name[] = "demo_test.txt";

    char file_content[200];
    snprintf(file_content, sizeof(file_content), "Hello, World! Write timestamp: %lu", HAL_GetTick());

    const int8_t mount_result = LFS_mount();
    if (mount_result != 0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS mounting error: %d", mount_result);
        return 1;
    }

    const int8_t write_result = LFS_write_file(file_name, (uint8_t*) file_content, strlen(file_content));
    if (write_result != 0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS writing error: %d", write_result);
        return 2;
    }

    uint8_t read_buffer[200] = {0};
    const int8_t read_result = LFS_read_file(file_name, 0, read_buffer, sizeof(read_buffer));
    if (read_result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS reading error: %d", read_result);
        return 3;
    }

    // Ensure safety for upcoming print.
    read_buffer[sizeof(read_buffer) - 1] = '\0';

    snprintf(
        response_output_buf, response_output_buf_len,
        "LittleFS Successfully Read File '%s'. System uptime: %lu, File Content: '%s'!",
        file_name, HAL_GetTick(), (char*)read_buffer);
    return 0;
}

/// @brief Telecommand: Benchmark LittleFS write and read operations
/// @param args_str
/// - Arg 0: Write chunk size (bytes)
/// - Arg 1: Write chunk count
/// @return 0 on success, 1 if error parsing args, 2 if benchmark failed
/// @note The maximum write chunk size is 127 bytes, apparently; need to investigate why so small.
uint8_t TCMDEXEC_fs_benchmark_write_read(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t arg_write_chunk_size, arg_write_chunk_count;

    const uint8_t parse_write_chunk_size_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &arg_write_chunk_size);
    const uint8_t parse_write_chunk_count_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &arg_write_chunk_count);
    if (parse_write_chunk_size_result != 0 || parse_write_chunk_count_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing write chunk size arg: Arg 0 Err=%d, Arg 1 Err=%d", parse_write_chunk_size_result, parse_write_chunk_count_result);
        return 1;
    }

    const int8_t benchmark_result = LFS_benchmark_write_read_single_and_new(arg_write_chunk_size, arg_write_chunk_count, response_output_buf, response_output_buf_len);
    response_output_buf[response_output_buf_len - 1] = '\0'; // ensure null-terminated

    if (benchmark_result != 0) {
        snprintf(
            &response_output_buf[strlen(response_output_buf)],
            response_output_buf_len - strlen(response_output_buf) - 1,
            "Benchmark failed. Error: %d", benchmark_result);
        return 2;
    }
    return 0;
}


/// @brief Telecommand: Write a file to internal flash memory from LittleFS.
/// @param args_str
/// - Arg 0: File name to read from LittleFS
/// - Arg 1: Length to read from the file (in bytes, number)
/// - Arg 2: Offset within the file to start reading (in bytes, hex value)
/// - Arg 3: Address in internal flash memory to write to (in hex, 8 characters for 32-bit address)
/// @note The maximum length to read is 1024 bytes (1kB).
uint8_t TCMDEXEC_fs_write_file_to_internal_flash(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len)
{
    char arg_file_name[LFS_MAX_PATH_LENGTH];
    const uint8_t parse_file_name_result = TCMD_extract_string_arg(args_str, 0, arg_file_name, sizeof(arg_file_name));
    if (parse_file_name_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing file name arg: Error %d", parse_file_name_result);
        return 1;
    }
    
    // Get length to data to read
    uint64_t read_length = 0;
    const uint8_t parse_length_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &read_length);
    if (parse_length_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing write length arg: Error %d", parse_length_result);
        return 2;
    }
 
    // Ensure the write length is not too large
    if (read_length > 1024) { // 1kB max
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error: Write length too large. Max is 64kB.");
        return 4;
    }
    // Get offset (in hex)
    uint8_t write_offset_arr[4] = {0}; // 4 bytes for 32-bit address
    uint16_t offset_arr_result_len = 0;
    const uint8_t parse_offset_result = TCMD_extract_hex_array_arg(
        args_str, 2, write_offset_arr, sizeof(write_offset_arr), &offset_arr_result_len
    );
    if (parse_offset_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing write offset arg: Error %d", parse_offset_result);
        return 3;
    }

    const uint32_t offset = write_offset_arr[0] << 24 |
                            write_offset_arr[1] << 16 |
                            write_offset_arr[2] << 8 |
                            write_offset_arr[3]; // Convert to 32-bit address

    // Get address to write to (in hex)
    uint8_t write_address_arr[4] = {0}; // 4 bytes for 32-bit address
    uint16_t address_arr_result_len = 0;
    const uint8_t parse_address_result = TCMD_extract_hex_array_arg(
        args_str, 3, write_address_arr, sizeof(write_address_arr), &address_arr_result_len
    );
    if (parse_address_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing write address arg: Error %d", parse_address_result);
        return 4;
    }
    const uint32_t write_address = write_address_arr[0] << 24 |
                                   write_address_arr[1] << 16 |
                                   write_address_arr[2] << 8 |
                                   write_address_arr[3]; // Convert to 32-bit address
        
        LOG_message(LOG_SYSTEM_ALL, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
                    "TCMDEXEC_fs_write_file_to_internal_flash: Writing %lu bytes from file '%s' to internal flash at address 0x%08lX, offset %lu",
        (unsigned long)read_length, arg_file_name, (unsigned long)write_address, (unsigned long)offset);
    
    // Read the file from LittleFS
    uint8_t read_buf[read_length];
    const int32_t read_result = LFS_read_file(arg_file_name, offset, read_buf, read_length);
    if (read_result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error reading file: %ld", read_result);
        return 5;
    }
    // Write the data to internal flash
    STM32_Internal_Flash_Write_Status_t write_status = {0};
    const uint8_t write_result = STM32_internal_flash_write(write_address, read_buf, read_length, &write_status);
    LOG_message(LOG_SYSTEM_ALL, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
                "Write status: Write Status=%d, Lock Status=%d",
        write_status.write_status, write_status.lock_status);
    // Handle specific error codes
    switch (write_result)
    {
        case 0:
            snprintf(response_output_buf, response_output_buf_len, "Successfully wrote %lu bytes from file '%s' to internal flash at address 0x%08lX", (uint32_t)read_length, arg_file_name, (uint32_t)write_address);
            return 0;
        case 1:
            snprintf(response_output_buf, response_output_buf_len, "Error: Address too low for internal flash write.");
            break;
        case 2:
            snprintf(response_output_buf, response_output_buf_len, "Error: Write exceeds internal flash memory bounds.");
            break;
        case 3:
            snprintf(response_output_buf, response_output_buf_len, "Error: Failed to unlock internal flash for writing.");
            break;
        case 4:
            snprintf(response_output_buf, response_output_buf_len, "Error: Failed to lock internal flash after writing.");
            break;
        case 5:
            snprintf(response_output_buf, response_output_buf_len, "Error: Internal flash write operation failed.");
            break;
        default:
            snprintf(response_output_buf, response_output_buf_len, "Error: Unknown error during internal flash write.");
            break;
    }
    return write_result <= 5 ? write_result : 6; // Return the error code if it's within the known range, otherwise return 6 for unknown error.

}
