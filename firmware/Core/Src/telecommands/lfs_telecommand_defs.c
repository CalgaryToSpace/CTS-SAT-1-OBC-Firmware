
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
    else
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
    else
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
    else
        snprintf(response_output_buf, response_output_buf_len, "LittleFS Successfully Unounted!\n");
    return 0;
}

/// @brief Telecommand: Write data to a file in LittleFS
/// @param args_str Arg 1: File name, Arg 2: Data to write
uint8_t TCMDEXEC_fs_write_file(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    char *arg_string1 = NULL;
    uint8_t parse_result = TCMD_extract_string_arg((char*)args_str, strlen((char*)args_str), 0, &arg_string1);
    if (parse_result > 0) {
        // error parsing
        snprintf(
            &response_output_buf[strlen(response_output_buf)],
            response_output_buf_len - strlen(response_output_buf) - 1,
            "Arg%d=error%d, ", 0, parse_result);
        // FIXME: isn't the code just gonna keep running here? Should be return?
    }

    char *arg_string2 = NULL;
    parse_result = TCMD_extract_string_arg((char*)args_str, strlen((char*)args_str), 0, &arg_string2);
    if (parse_result > 0) {
        // error parsing
        snprintf(
            &response_output_buf[strlen(response_output_buf)],
            response_output_buf_len - strlen(response_output_buf) - 1,
            "Arg%d=error%d, ", 1, parse_result);
        // FIXME: isn't the code just gonna keep running here? Should be return?
    }
    int8_t result = LFS_write_file(arg_string1, (uint8_t*) arg_string2, sizeof(arg_string2));
    if (result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS Writing Error: %d\n", result);
        return 1;
    }
    else
        snprintf(response_output_buf, response_output_buf_len, "LittleFS Successfully Wrote Data!");
    return 0;
}

uint8_t TCMDEXEC_fs_read_file(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    char *arg_string1 = NULL;
    uint8_t rx_buffer[512] = {0};

    uint8_t parse_result = TCMD_extract_string_arg((char*)args_str, strlen((char*)args_str), 0, &arg_string1);
    if (parse_result > 0) {
        // error parsing
        snprintf(
            &response_output_buf[strlen(response_output_buf)],
            response_output_buf_len - strlen(response_output_buf) - 1,
            "Arg%d=error%d, ", 0, parse_result);
        // FIXME: isn't the code just gonna keep running here? Should be return?
    }

    int8_t result = LFS_write_file(arg_string1, rx_buffer, sizeof(rx_buffer));
    if (result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "LittleFS Reading Error: %d\n", result);
        return 1;
    }
    else
        snprintf(response_output_buf, response_output_buf_len, "LittleFS Successfully Read Data: %s!", rx_buffer);
    return 0;
}
