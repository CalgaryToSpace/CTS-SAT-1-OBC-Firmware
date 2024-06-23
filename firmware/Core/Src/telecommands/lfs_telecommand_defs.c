
#include <stdio.h>
#include <stdint.h>

#include "debug_tools/debug_uart.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"
#include "log/ulog.h"
#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_args_helpers.h"
#include "log/log.h"


uint8_t TCMDEXEC_fs_format_storage(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    int8_t result = LFS_format();
    if (result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS Formatting Error: %d\n", result);
        return 1;
    }
    
    snprintf(response_output_buf, response_output_buf_len, "LittleFS Successfully Formatted!\n");
    return 0;
}

uint8_t TCMDEXEC_fs_mount(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    int8_t result = LFS_mount();
    if (result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS Mounting Error: %d\n", result);
        return 1;
    }
    
    snprintf(response_output_buf, response_output_buf_len, "LittleFS Successfully Mounted!\n");
    return 0;
}

uint8_t TCMDEXEC_fs_unmount(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    int8_t result = LFS_unmount();
    if (result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS Unmounting Error: %d\n", result);
        return 1;
    }
    
    snprintf(response_output_buf, response_output_buf_len, "LittleFS Successfully Unounted!\n");
    return 0;
}

/// @brief Telecommand: Write data to a file in LittleFS
/// @param args_str Arg 0: File name, Arg 1: String to write to file
uint8_t TCMDEXEC_fs_write_file(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    char arg_file_name[64] = {0};
    const uint8_t parse_file_name_result = TCMD_extract_string_arg((char*)args_str, 0, arg_file_name, sizeof(arg_file_name));
    if (parse_file_name_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing file name arg: Error %d", parse_file_name_result);
        return 1;
    }

    char arg_file_content[512] = {0};
    const uint8_t parse_file_content_result = TCMD_extract_string_arg((char*)args_str, 1, arg_file_content, sizeof(arg_file_content));
    if (parse_file_content_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing file content arg: Error %d", parse_file_content_result);
        return 2;
    }

    int8_t result = LFS_write_file(arg_file_name, (uint8_t*) arg_file_content, strlen(arg_file_content));
    if (result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS Writing Error: %d\n", result);
        return 1;
    }
    
    snprintf(response_output_buf, response_output_buf_len, "LittleFS Successfully Wrote Data!");
    return 0;
}

uint8_t TCMDEXEC_fs_read_file(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t rx_buffer[512] = {0};

    char arg_file_name[64] = {0};
    const uint8_t parse_file_name_result = TCMD_extract_string_arg((char*)args_str, 0, arg_file_name, sizeof(arg_file_name));
    if (parse_file_name_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing file name arg: Error %d", parse_file_name_result);
        return 1;
    }

    lfs_ssize_t file_size = LFS_file_size(arg_file_name);
    if (file_size <0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS error getting file size '%s': %d", arg_file_name, file_size);
        return 1;
    }
    lfs_ssize_t bytes_read = 1;
    lfs_ssize_t total_bytes_read = 0;
    while (total_bytes_read < file_size && bytes_read > 0) {
        bytes_read = LFS_read_file(arg_file_name, total_bytes_read, rx_buffer, sizeof(rx_buffer));
        if (bytes_read < 0) {
            snprintf(response_output_buf, response_output_buf_len, "LittleFS error reading file '%s': %d", arg_file_name, bytes_read);
            return 1;
        }
        total_bytes_read += bytes_read;
        // print to uart and radio
        DEBUG_uart_print_array_hex(rx_buffer, bytes_read);
        // TODO send to radio
        DEBUG_uart_print_str("TODO: send data to radio from TCMD_fs_read_file()\n");

    }
    
    snprintf(response_output_buf, response_output_buf_len, "LittleFS Successfully Read File '%s': '%s'!", arg_file_name, rx_buffer);
    return 0;
}

uint8_t TCMDEXEC_fs_read_text_file(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t rx_buffer[512] = {0};

    char arg_file_name[64] = {0};
    const uint8_t parse_file_name_result = TCMD_extract_string_arg((char*)args_str, 0, arg_file_name, sizeof(arg_file_name));
    if (parse_file_name_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing file name arg: Error %d", parse_file_name_result);
        return 1;
    }

    lfs_ssize_t file_size = LFS_file_size(arg_file_name);
    if (file_size <0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS error getting file size '%s': %d", arg_file_name, file_size);
        return 1;
    }
    lfs_ssize_t bytes_read = 1;
    lfs_ssize_t total_bytes_read = 0;
    while (total_bytes_read < file_size && bytes_read > 0) {
        bytes_read = LFS_read_file(arg_file_name, total_bytes_read, rx_buffer, sizeof(rx_buffer) - 1);
        if (bytes_read < 0) {
            snprintf(response_output_buf, response_output_buf_len, "LittleFS error reading file '%s': %d", arg_file_name, bytes_read);
            return 1;
        }
        total_bytes_read += bytes_read;
        rx_buffer[bytes_read] = '\0';
        // print to uart and radio
        DEBUG_uart_print_str((char*)rx_buffer);
        // TODO send to radio
        DEBUG_uart_print_str("TODO: send data to radio from TCMD_fs_read_text_file()\n");

    }
    
    snprintf(response_output_buf, response_output_buf_len, "LittleFS Successfully Read File '%s': '%s'!", arg_file_name, rx_buffer);
    return 0;
}


uint8_t TCMDEXEC_fs_demo_write_then_read(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    char file_name[] = "demo_test.txt";

    char file_content[200];
    snprintf(file_content, sizeof(file_content), "Hello, World! Write timestamp: %lu", HAL_GetTick());

    const int8_t mount_result = LFS_mount();
    if (mount_result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS mounting error: %d\n", mount_result);
        return 1;
    }

    // TODO: Delete file first, if it exists, otherwise it just overwrites from the start, keeping anything extra longer in the file.
    const int8_t write_result = LFS_write_file(file_name, (uint8_t*) file_content, strlen(file_content));
    if (write_result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS writing error: %d\n", write_result);
        return 2;
    }

    uint8_t read_buffer[200] = {0};
    const int8_t read_result = LFS_read_file(file_name, 0, read_buffer, sizeof(read_buffer));
    if (read_result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS reading error: %d\n", read_result);
        return 3;
    }

    // ensure safety for upcoming print
    read_buffer[sizeof(read_buffer) - 1] = '\0';

    snprintf(
        response_output_buf, response_output_buf_len,
        "LittleFS Successfully Read File '%s'. System uptime: %lu, File Content: '%s'!",
        file_name, HAL_GetTick(), (char*)read_buffer);
    return 0;
}
