
#include "telecommands/telecommand_definitions.h"
#include "littlefs/lfs.h"
#include "telecommands/telecommand_args_helpers.h"
#include "transforms/arrays.h"
#include "unit_tests/unit_test_executor.h"
#include "mpi_firmware.h"
#include "littlefs/lfs.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>


extern uint8_t LFS_mounted;
extern lfs_t lfs;

// Is there a better approach to MPI firmware management?
// This is temporary storage while MPI firmware pages are being sent to the satellite.
// Or maybe do an fset on the file descriptor? What if pages arrive out of
// order?
// OR: write each page to its own file, like mpi_firmware_page_0001.bin, etc.
// then writing to the MPI would just load the files in order.

// extern
const TCMD_TelecommandDefinition_t TCMD_telecommand_definitions[] = {
    {
        .tcmd_name = "hello_world",
        .tcmd_func = TCMDEXEC_hello_world,
        .number_of_args = 0,
    },
    {
        .tcmd_name = "core_system_stats",
        .tcmd_func = TCMDEXEC_core_system_stats,
        .number_of_args = 0,
    },
    {
        .tcmd_name = "echo_back_args",
        .tcmd_func = TCMDEXEC_echo_back_args,
        .number_of_args = 1, // TODO: support more than 1 arg
    },
    {
        .tcmd_name = "echo_back_uint32_args",
        .tcmd_func = TCMDEXEC_echo_back_uint32_args,
        .number_of_args = 10,
    },
    {
        .tcmd_name = "fs_format_storage",
        .tcmd_func = TCMDEXEC_echo_back_uint32_args,
        .number_of_args = 0,
    },
    {
        .tcmd_name = "run_all_unit_tests",
        .tcmd_func = TCMDEXEC_run_all_unit_tests,
        .number_of_args = 0,
    },
    {
        .tcmd_name = "available_telecommands",
        .tcmd_func = TCMDEXEC_available_telecommands,
        .number_of_args = 0,
    },
    {
        .tcmd_name = "upload_mpi_firmware_page",
        .tcmd_func = TCMDEXEC_upload_mpi_firmware_page,
        .number_of_args = 4,
    }
};

// extern
const int16_t TCMD_NUM_TELECOMMANDS = sizeof(TCMD_telecommand_definitions) / sizeof(TCMD_TelecommandDefinition_t);

// each telecommand function must have the following signature:
// uint8_t <function_name>(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
//                          char *response_output_buf, uint16_t response_output_buf_len)

uint8_t TCMDEXEC_hello_world(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    snprintf(response_output_buf, response_output_buf_len, "Hello, world!\n");
    return 0;
}

uint8_t TCMDEXEC_core_system_stats(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    snprintf(response_output_buf, response_output_buf_len, "System stats: TODO\n");
    return 0;
}

uint8_t TCMDEXEC_echo_back_args(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    snprintf(response_output_buf, response_output_buf_len, "SUCCESS: Echo Args: '%s'\n", args_str);
    // TODO: handle args_str being too long
    return 0;
}

uint8_t TCMDEXEC_echo_back_uint32_args(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    response_output_buf[0] = '\0'; // clear the response buffer

    for (uint8_t arg_num = 0; arg_num < 10; arg_num++) {
        uint64_t arg_uint64;
        uint8_t parse_result = TCMD_extract_uint64_arg(
            (char*)args_str, strlen((char*)args_str), arg_num, &arg_uint64);
        if (parse_result > 0) {
            // error parsing
            snprintf(
                &response_output_buf[strlen(response_output_buf)],
                response_output_buf_len - strlen(response_output_buf) - 1,
                "Arg%d=error%d, ", arg_num, parse_result);
        }
        else {
            // success parsing
            snprintf(
                &response_output_buf[strlen(response_output_buf)],
                response_output_buf_len - strlen(response_output_buf) - 1,
                "Arg%d=%" PRIu32 ", ",
                arg_num, (uint32_t)arg_uint64);
        }
    }
    return 0;
}

uint8_t TCMDEXEC_fs_format_storage(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len){
    // uint8_t result = LFS_FORMAT();
    // if (result < 0) {
    //     // debug_uart_print_str("Formatting Error: ");
    //     // debug_uart_print_uint32(result);
    //     // debug_uart_print_str("\n");
    //     return result;
    // } else {
    //     // debug_uart_print_str("Formatting Successfull");
    //     return result;
    // }
    return 0;
}

uint8_t TCMDEXEC_run_all_unit_tests(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    TEST_run_all_unit_tests_and_log(response_output_buf, response_output_buf_len);
    return 0;
}

uint8_t TCMDEXEC_available_telecommands(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    char response[512] = {0};
    char *p = response;
    ssize_t left = sizeof(response);
    size_t len = 0;
    snprintf(p, left, "%s", "Available_telecommands\n");
    p += 23;
    left -= 23;
    for (uint16_t i = 0; i < TCMD_NUM_TELECOMMANDS; i++) {
        len = strlen(TCMD_telecommand_definitions[i].tcmd_name) + 6;
        snprintf(p, left, "%3u) %s\n", i + 1, TCMD_telecommand_definitions[i].tcmd_name);
        p += len;
        if (left > len) {
            left -= len;
        }
        else {
            break;
        }
    }
    snprintf(response_output_buf, response_output_buf_len, "%s", response);

    return 0;
}

uint8_t TCMDEXEC_upload_mpi_firmware_page(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    response_output_buf[0] = '\0';
    
    // Store bytes to the MPI firmware binary file for updating the MPI firmware later
    
    uint8_t parse_result = 0;
    uint64_t file_start_address = 0;
    uint64_t mpi_firmware_file_size = 0;

    parse_result = TCMD_extract_uint64_arg((char*)args_str, strlen((char*)args_str), 0, &file_start_address);
    if (parse_result > 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: Unable to parse start address.");
        return 1;
    }

    uint32_t start_index = 0;
    uint32_t number_of_bytes_sent = 0;
    parse_result = TCMD_get_arg_info((char*)args_str, strlen((char*)args_str), 1, &start_index, NULL, &number_of_bytes_sent);
    if (parse_result > 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: Unable to determine index of requested argument");
        return 1;
    }

    uint8_t firmware_bytes[MPI_FIRMWARE_FILE_CHUNK_SIZE];
    uint32_t firmware_bytes_len = MPI_FIRMWARE_FILE_CHUNK_SIZE;
    parse_result = TCMD_arg_base64_decode((char*)args_str, strlen((char*)args_str), 1, firmware_bytes, &firmware_bytes_len); 
    if (parse_result > 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: Unable to decode argument as base 64.");
        return 1;
    }

    parse_result = TCMD_extract_uint64_arg((char*)args_str, strlen((char*)args_str), 3, &mpi_firmware_file_size);
    if (parse_result > 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: Unable to parse MPI firmware filesize.");
        return 1;
    }

    if (file_start_address >= mpi_firmware_file_size) {
        snprintf(response_output_buf, response_output_buf_len, "Error: Start address is larger than size of firmware file.");
        return 1;
    }

    char firmware_filename[TCMD_MAX_STRING_LEN] = {0};
    parse_result = TCMD_arg_as_string((char*)args_str, strlen((char*)args_str), 2, firmware_filename);
    if (parse_result > 0) {
        snprintf(response_output_buf, response_output_buf_len, "Unable to parse argument as filename");
    }

    // TODO: remove this: No LFS flash mem attached. Pretend it went well

    snprintf(response_output_buf, response_output_buf_len, "Received MPI memory page");
    return 0;

    lfs_file_t file;
    int result = 0;
    result = lfs_file_open(&lfs, &file, firmware_filename, LFS_O_APPEND); 
    if (result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "Unable to open MPI firmware file: %s; response: %d", firmware_filename, result);
        return 1;
    }

    if (file_start_address == 0) {
        result = lfs_file_truncate(&lfs, &file, mpi_firmware_file_size);
        if (result < 0) {
            snprintf(response_output_buf, response_output_buf_len, "Unable to resize MPI firmware file: %s; response: %d", firmware_filename, result);
            return 1;
        }
    }

    result = lfs_file_seek(&lfs, &file, file_start_address, LFS_SEEK_SET);
    if (result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "Unable to seek in MPI firmware file: %s: response: %d", firmware_filename, result);
        return 1;
    }

    result = lfs_file_write(&lfs, &file, firmware_bytes, firmware_bytes_len);
    if (result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "Unable to write MPI firmware page to file: %s; response: %d", firmware_filename, result);
        return 1;
    }

    result = lfs_file_close(&lfs, &file);
    if (result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "Unable to close MPI firmware file: %s; response: %d", firmware_filename, result);
        return 1;
    }

    snprintf(response_output_buf, response_output_buf_len, "Received MPI firmware page. Wrote %lu bytes to \"%s\" at address %lu", firmware_bytes_len, firmware_filename, (uint32_t)file_start_address);
    return 0;
}

