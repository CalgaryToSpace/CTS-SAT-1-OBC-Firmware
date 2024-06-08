
#include <stdio.h>
#include <stdint.h>

#include "littlefs/littlefs_helper.h"
#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_args_helpers.h"


uint8_t TCMDEXEC_fs_format_storage(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    int8_t result = LFS_format();
    if (result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS Formatting Error: %d\n", result);
        return 1;
    }
    else {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS Successfully Formatted!\n");
    }
    return 0;
}

uint8_t TCMDEXEC_fs_mount(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    int8_t result = LFS_mount();
    if (result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS Mounting Error: %d\n", result);
        return 1;
    }
    else {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS Successfully Mounted!\n");
    }
    return 0;
}

uint8_t TCMDEXEC_fs_unmount(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    int8_t result = LFS_unmount();
    if (result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS Unmounting Error: %d\n", result);
        return 1;
    }
    else {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS Successfully Unounted!\n");
    }
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
    else {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS Successfully Wrote Data!");
    }
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
        snprintf(response_output_buf, response_output_buf_len, "LittleFS Reading Error: %d\n", result);
        return 1;
    }
    else {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS Successfully Read Data: %s!", rx_buffer);
    }
    return 0;
}
