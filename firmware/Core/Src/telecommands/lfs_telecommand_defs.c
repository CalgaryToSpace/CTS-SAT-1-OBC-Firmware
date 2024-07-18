
#include <stdio.h>
#include <stdint.h>

#include "debug_tools/debug_uart.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"
#include "littlefs/littlefs_benchmark.h"
#include "log/log.h"
#include "telecommands/lfs_telecommand_defs.h"
#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_args_helpers.h"


uint8_t TCMDEXEC_fs_format_storage(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    int8_t result = LFS_format();
    if (result < 0) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "LittleFS Formatting Error: %d", result);
        return 1;
    }
    
    LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "LittleFS Successfully Formatted!");
    return 0;
}

uint8_t TCMDEXEC_fs_mount(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    int8_t result = LFS_mount();
    if (result < 0) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "LittleFS Mounting Error: %d", result);
        return 1;
    }
    
    LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "LittleFS Successfully Mounted!");
    return 0;
}

uint8_t TCMDEXEC_fs_unmount(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    int8_t result = LFS_unmount();
    if (result < 0) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "LittleFS Unmounting Error: %d", result);
        return 1;
    }
    
    LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "LittleFS Successfully Unounted!");
    return 0;
}

/// @brief Telecommand: Write data to a file in LittleFS
/// @param args_str
/// - Arg 0: File path as string
/// - Arg 1: String to write to file
uint8_t TCMDEXEC_fs_write_file(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    char arg_file_name[64] = {0};
    const uint8_t parse_file_name_result = TCMD_extract_string_arg(args_str, 0, arg_file_name, sizeof(arg_file_name));
    if (parse_file_name_result != 0) {
        // error parsing
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Error parsing file name arg: Error %d", parse_file_name_result);
        return 1;
    }

    char arg_file_content[512] = {0};
    const uint8_t parse_file_content_result = TCMD_extract_string_arg(args_str, 1, arg_file_content, sizeof(arg_file_content));
    if (parse_file_content_result != 0) {
        // error parsing
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Error parsing file content arg: Error %d", parse_file_content_result);
        return 2;
    }

    int8_t result = LFS_write_file(arg_file_name, (uint8_t*) arg_file_content, strlen(arg_file_content));
    if (result < 0) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "LittleFS Writing Error: %d", result);
        return 1;
    }
    
    LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "LittleFS Successfully Wrote Data!");
    return 0;
}

/// @brief Reads a file from LittleFS, and responds with its contents as 2-digit hex bytes.
/// @param args_str
/// - Arg 0: File path as string
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_fs_read_file_hex(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t rx_buffer[512] = {0};

    char arg_file_name[64] = {0};
    const uint8_t parse_file_name_result = TCMD_extract_string_arg(args_str, 0, arg_file_name, sizeof(arg_file_name));
    if (parse_file_name_result != 0) {
        // error parsing
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse file name arg: Error code %d", parse_file_name_result);
        return 1;
    }

    lfs_ssize_t file_size = LFS_file_size(arg_file_name);
    if (file_size <0) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "LittleFS error getting file size '%s': %ld", arg_file_name, file_size);
        return 1;
    }
    lfs_ssize_t bytes_read = 1;
    lfs_ssize_t total_bytes_read = 0;
    while (total_bytes_read < file_size && bytes_read > 0) {
        bytes_read = LFS_read_file(arg_file_name, total_bytes_read, rx_buffer, sizeof(rx_buffer));
        if (bytes_read < 0) {
            LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "LittleFS error reading file '%s': %ld", arg_file_name, bytes_read);
            return 1;
        }
        total_bytes_read += bytes_read;
        // print to uart and radio
        // No need to log the output
        DEBUG_uart_print_array_hex(rx_buffer, bytes_read);
        // TODO send to radio
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_CRITICAL, LOG_CHANNEL_ALL, "TODO: send data to radio from TCMD_fs_read_file_hex()");
    }
    
    LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "LittleFS Successfully Read File '%s': '%s'!", arg_file_name, rx_buffer);

    return 0;
}

/// @brief Reads a file from LittleFS, and responds with its contents as 2-digit hex bytes.
/// @param args_str
/// - Arg 0: File path as string
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_fs_read_text_file(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t rx_buffer[512] = {0};

    char arg_file_name[64] = {0};
    const uint8_t parse_file_name_result = TCMD_extract_string_arg((char*)args_str, 0, arg_file_name, sizeof(arg_file_name));
    if (parse_file_name_result != 0) {
        // error parsing
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Unable to parse file name arg: Error code %d", parse_file_name_result);
        return 1;
    }

    lfs_ssize_t file_size = LFS_file_size(arg_file_name);
    if (file_size <0) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "LittleFS error getting file size '%s': %ld", arg_file_name, file_size);
        return 1;
    }
    lfs_ssize_t bytes_read = 1;
    lfs_ssize_t total_bytes_read = 0;
    while (total_bytes_read < file_size && bytes_read > 0) {
        bytes_read = LFS_read_file(arg_file_name, total_bytes_read, rx_buffer, sizeof(rx_buffer) - 1);
        if (bytes_read < 0) {
            LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "LittleFS error reading file '%s': %ld", arg_file_name, bytes_read);
            return 1;
        }
        total_bytes_read += bytes_read;
        rx_buffer[bytes_read] = '\0';
        // print to uart and radio
        DEBUG_uart_print_str((char*)rx_buffer);
        // TODO send to radio
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_CRITICAL, LOG_CHANNEL_ALL, "TODO: send data to radio from TCMD_fs_read_text_file()");

    }
    
    LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "LittleFS Successfully Read File '%s': '%s'!", arg_file_name, rx_buffer);
    return 0;
}


uint8_t TCMDEXEC_fs_demo_write_then_read(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    char file_name[] = "demo_test.txt";

    char file_content[200];
    snprintf(file_content, sizeof(file_content), "Hello, World! Write timestamp: %lu", HAL_GetTick());

    const int8_t mount_result = LFS_mount();
    if (mount_result < 0) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "LittleFS mounting error: %d", mount_result);
        return 1;
    }

    const int8_t write_result = LFS_write_file(file_name, (uint8_t*) file_content, strlen(file_content));
    if (write_result < 0) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "LittleFS writing error: %d", write_result);
        return 2;
    }

    uint8_t read_buffer[200] = {0};
    const int8_t read_result = LFS_read_file(file_name, 0, read_buffer, sizeof(read_buffer));
    if (read_result < 0) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "LittleFS reading error: %d", read_result);
        return 3;
    }

    // Ensure safety for upcoming print.
    read_buffer[sizeof(read_buffer) - 1] = '\0';

    LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "LittleFS Successfully Read File '%s'. System uptime: %lu, File Content: '%s'!", file_name, HAL_GetTick(), (char*)read_buffer);
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

    const uint8_t parse_write_chunk_size_result = TCMD_extract_uint64_arg((char*)args_str, strlen((char*)args_str), 0, &arg_write_chunk_size);
    const uint8_t parse_write_chunk_count_result = TCMD_extract_uint64_arg((char*)args_str, strlen((char*)args_str), 1, &arg_write_chunk_count);
    if (parse_write_chunk_size_result != 0 || parse_write_chunk_count_result != 0) {
        // error parsing
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "Error parsing write chunk size arg: Arg 0 Err=%d, Arg 1 Err=%d", parse_write_chunk_size_result, parse_write_chunk_count_result);
        return 1;
    }

    const int8_t benchmark_result = LFS_benchmark_write_read(arg_write_chunk_size, arg_write_chunk_count, response_output_buf, response_output_buf_len);
    response_output_buf[response_output_buf_len - 1] = '\0'; // ensure null-terminated

    if (benchmark_result != 0) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Benchmark failed. Error: %d", benchmark_result);
        return 2;
    }
    return 0;
}
