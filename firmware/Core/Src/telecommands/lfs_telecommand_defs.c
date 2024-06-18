
#include <stdio.h>
#include <stdint.h>

#include "debug_tools/debug_uart.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"
#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_args_helpers.h"

extern lfs_t lfs;

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

    int8_t result = LFS_read_file(arg_file_name, rx_buffer, sizeof(rx_buffer));
    if (result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS error reading file '%s': %d", arg_file_name, result);
        return 1;
    }
    
    snprintf(response_output_buf, response_output_buf_len, "LittleFS Successfully Read File '%s': '%s'!", arg_file_name, rx_buffer);
    return 0;
}

uint8_t TCMDEXEC_fs_ls_dir(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    // args_str points to the first character of the first argument

    char arg_dir_name[64] = {0};
    const uint8_t parse_dir_name_result = TCMD_extract_string_arg((char*)args_str, 0, arg_dir_name, sizeof(arg_dir_name));
    if (parse_dir_name_result != 0) {
        // error parsing
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing dir name arg: Error %d", parse_dir_name_result);
        return 1;
    }

    // Based on https://github.com/littlefs-project/littlefs/issues/2
    lfs_dir_t dir = {0};
    int8_t result = lfs_dir_open(&lfs, &dir, arg_dir_name);
    if (result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS error reading dir '%s': %d", arg_dir_name, result);
        return 1;
    }

    struct lfs_info info = {0};
    const char item[512] = {0};
    while (1) {
        int read_result = lfs_dir_read(&lfs, &dir, &info);
        if (read_result < 0) {
            return 1;
        }
        if (read_result == 0) {
            break;
        }
        snprintf((char*)item, 512, "%s%s\n", info.name, info.type == LFS_TYPE_DIR ? "/" : "");
        DEBUG_uart_print_str(item);
    }
    response_output_buf[0] = '\0';

    int close_result = lfs_dir_close(&lfs, &dir);
    if (close_result) {
        return 1;
    }

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
    const int8_t read_result = LFS_read_file(file_name, read_buffer, sizeof(read_buffer));
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
