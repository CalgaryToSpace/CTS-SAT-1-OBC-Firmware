
#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_args_helpers.h"
#include "transforms/arrays.h"
#include "unit_tests/unit_test_executor.h"
#include "mpi_firmware.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

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
        snprintf(response_output_buf, response_output_buf_len, "Unable to parse start address.");
        return 1;
    }

    parse_result = TCMD_extract_uint64_arg((char*)args_str, strlen((char*)args_str), 3, &mpi_firmware_file_size);
    if (parse_result > 0) {
        snprintf(response_output_buf, response_output_buf_len, "Unable to parse MPI firmware filesize.");
        return 1;
    }

    if (file_start_address >= mpi_firmware_file_size) {
        snprintf(response_output_buf, response_output_buf_len, "Unable to parse MPI firmware filesize.");
        return 1;
    }

    size_t bytes_to_copy = mpi_firmware_file_size - file_start_address;
    if (bytes_to_copy > MPI_FIRMWARE_FILE_CHUNK_SIZE) {
        bytes_to_copy = MPI_FIRMWARE_FILE_CHUNK_SIZE;
    }

    uint8_t firmware_bytes[MPI_FIRMWARE_FILE_CHUNK_SIZE];
    uint32_t start_index = 0;
    parse_result = TCMD_get_arg_info((char*)args_str, strlen((char*)args_str), 1, &start_index, NULL, NULL);
    if (parse_result > 0) {
        snprintf(response_output_buf, response_output_buf_len, "Unable to determine index of requested argument");
        return 1;
    }

    // TODO bug-hunting
    memcpy(firmware_bytes, args_str + start_index, bytes_to_copy);

    
    char firmware_filename[TCMD_MAX_STRING_LEN] = {0};
    parse_result = TCMD_arg_as_string((char*)args_str, strlen((char*)args_str), 2, firmware_filename);
    if (parse_result > 0) {
        snprintf(response_output_buf, response_output_buf_len, "Unable to parse argument as filename");
    }

    // TODO write these bytes into the MPI file
    // ... 
    //
    // load the file bytes and overwrite / extend the array

    
    snprintf(response_output_buf, response_output_buf_len, "Received MPI firmware page. Wrote %u bytes to %s", bytes_to_copy, firmware_filename);
    return 0;
}

