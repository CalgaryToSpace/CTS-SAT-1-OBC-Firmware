
#include <stdio.h>
#include <stdint.h>

#include "debug_tools/debug_uart.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"
#include "littlefs/littlefs_benchmark.h"
#include "log/log.h"
#include "telecommands/lfs_telecommand_defs.h"
#include "telecommand_exec/telecommand_definitions.h"
#include "telecommand_exec/telecommand_args_helpers.h"

uint8_t TCMDEXEC_fs_format_storage(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    const int8_t result = LFS_format();
    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: LFS_format() -> %d", result);
        return 1;
    }
    
    snprintf(response_output_buf, response_output_buf_len, "LittleFS successfully formatted");
    return 0;
}

uint8_t TCMDEXEC_fs_mount(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    const int8_t result = LFS_mount();
    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: LFS_mount() -> %d", result);
        return 1;
    }
    
    snprintf(response_output_buf, response_output_buf_len, "LittleFS successfully mounted");
    return 0;
}

uint8_t TCMDEXEC_fs_unmount(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
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
uint8_t TCMDEXEC_fs_list_directory(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
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

/// @brief Telecommand: Create a directory
/// @param args_str
/// - Arg 0: Directory Name as string (e.g., "/dir1", "/dir1/subdir1")
uint8_t TCMDEXEC_fs_make_directory(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
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
uint8_t TCMDEXEC_fs_write_file_str(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
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
uint8_t TCMDEXEC_fs_write_file_hex(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
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
uint8_t TCMDEXEC_fs_delete_file(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
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

/// @brief Reads a file from LittleFS, and responds with its contents as 2-digit hex bytes.
/// @param args_str
/// - Arg 0: File path as string
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_fs_read_file_hex(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    uint8_t rx_buffer[512] = {0};

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

    const lfs_ssize_t file_size = LFS_file_size(arg_file_name);
    if (file_size < 0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS error getting file size '%s': %ld", arg_file_name, file_size);
        return 1;
    }
    lfs_ssize_t last_bytes_read = 1;
    lfs_ssize_t total_bytes_read = 0;
    while (total_bytes_read < file_size && last_bytes_read > 0) {
        last_bytes_read = LFS_read_file(arg_file_name, total_bytes_read, rx_buffer, sizeof(rx_buffer));
        if (last_bytes_read < 0) {
            snprintf(response_output_buf, response_output_buf_len, "LittleFS error reading file '%s': %ld", arg_file_name, last_bytes_read);
            return 1;
        }
        total_bytes_read += last_bytes_read;
        // Print to uart and radio.
        DEBUG_uart_print_array_hex(rx_buffer, last_bytes_read);
        // TODO: send to radio
        DEBUG_uart_print_str("TODO: send data to radio from TCMD_fs_read_file_hex()\n");
    }
    
    snprintf(
        response_output_buf, response_output_buf_len,
        "LittleFS successfully read file '%s': %ld bytes read",
        arg_file_name,
        total_bytes_read
    );
    return 0;
}

/// @brief Reads a file from LittleFS, and responds with its contents as 2-digit hex bytes.
/// @param args_str
/// - Arg 0: File path as string
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_fs_read_text_file(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    uint8_t rx_buffer[512] = {0};

    char arg_file_name[LFS_MAX_PATH_LENGTH];
    const uint8_t parse_file_name_result = TCMD_extract_string_arg(
        args_str, 0, arg_file_name, sizeof(arg_file_name)
    );
    if (parse_file_name_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing file name arg: Error %d", parse_file_name_result);
        return 1;
    }

    const lfs_ssize_t file_size = LFS_file_size(arg_file_name);
    if (file_size < 0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS error getting file size '%s': %ld", arg_file_name, file_size);
        return 1;
    }
    lfs_ssize_t last_bytes_read = 1;
    lfs_ssize_t total_bytes_read = 0;
    while (total_bytes_read < file_size && last_bytes_read > 0) {
        last_bytes_read = LFS_read_file(arg_file_name, total_bytes_read, rx_buffer, sizeof(rx_buffer) - 1);
        if (last_bytes_read < 0) {
            snprintf(response_output_buf, response_output_buf_len, "LittleFS error reading file '%s': %ld", arg_file_name, last_bytes_read);
            return 1;
        }
        total_bytes_read += last_bytes_read;
        rx_buffer[last_bytes_read] = '\0';
        // Print to uart and radio.
        DEBUG_uart_print_str((char*)rx_buffer);
        // TODO: send to radio
        DEBUG_uart_print_str("TODO: send data to radio from TCMD_fs_read_text_file()\n");

        // The following block is to enable printing out camera data. Not permissible in normal operation.
        // osDelay(300);
        // HAL_IWDG_Refresh(&hiwdg);
    }
    
    
    snprintf(
        response_output_buf, response_output_buf_len,
        "LittleFS successfully read file '%s': %ld bytes read",
        arg_file_name,
        total_bytes_read
    );
    return 0;
}


uint8_t TCMDEXEC_fs_demo_write_then_read(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
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
    if (read_result != 0) {
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
uint8_t TCMDEXEC_fs_benchmark_write_read(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
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
