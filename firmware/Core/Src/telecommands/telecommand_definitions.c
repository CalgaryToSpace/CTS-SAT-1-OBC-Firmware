
#include "telecommands/telecommand_definitions.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"
#include "telecommands/telecommand_args_helpers.h"
#include "transforms/arrays.h"
#include "unit_tests/unit_test_executor.h"
#include "mpi_firmware.h"

// Additional telecommand definitions files:
#include "telecommands/flash_telecommand_defs.h"
#include "telecommands/lfs_telecommand_defs.h"


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

extern volatile uint8_t TASK_heartbeat_is_on;

extern lfs_t lfs;
extern uint8_t LFS_is_lfs_mounted;

// extern
const TCMD_TelecommandDefinition_t TCMD_telecommand_definitions[] = {
    {
        .tcmd_name = "hello_world",
        .tcmd_func = TCMDEXEC_hello_world,
    .number_of_args = 0,
},
    {
        .tcmd_name = "heartbeat_off",
        .tcmd_func = TCMDEXEC_heartbeat_off,
        .number_of_args = 0,
    },
    {
        .tcmd_name = "heartbeat_on",
        .tcmd_func = TCMDEXEC_heartbeat_on,
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
    // ****************** SECTION: flash_telecommand_defs ******************
    {
        .tcmd_name = "flash_activate_each_cs",
        .tcmd_func = TCMDEXEC_flash_activate_each_cs,
        .number_of_args = 0,
    },
    {
        .tcmd_name = "flash_each_is_reachable",
        .tcmd_func = TCMDEXEC_flash_each_is_reachable,
        .number_of_args = 0,
    },
    {
        .tcmd_name = "flash_read_hex",
        .tcmd_func = TCMDEXEC_flash_read_hex,
        .number_of_args = 3,
    },
    {
        .tcmd_name = "flash_write_hex",
        .tcmd_func = TCMDEXEC_flash_write_hex,
        .number_of_args = 3,
    },
    {
        .tcmd_name = "flash_erase",
        .tcmd_func = TCMDEXEC_flash_erase,
        .number_of_args = 2,
    },
    // ****************** END SECTION: flash_telecommand_defs ******************

    // ****************** SECTION: lfs_telecommand_defs ******************
    {
        .tcmd_name = "fs_format_storage",
        .tcmd_func = TCMDEXEC_fs_format_storage,
        .number_of_args = 0,
    },
    {
        .tcmd_name = "fs_mount",
        .tcmd_func = TCMDEXEC_fs_mount,
        .number_of_args = 0,
    },
    {
        .tcmd_name = "fs_unmount",
        .tcmd_func = TCMDEXEC_fs_unmount,
        .number_of_args = 0,
    },
    {
        .tcmd_name = "fs_write_file",
        .tcmd_func = TCMDEXEC_fs_write_file,
        .number_of_args = 2,
    },
    {
        .tcmd_name = "fs_read_file",
        .tcmd_func = TCMDEXEC_fs_read_file,
        .number_of_args = 1,
    },
    {
        .tcmd_name = "fs_ls_dir",
        .tcmd_func = TCMDEXEC_fs_ls_dir,
        .number_of_args = 1,
    },
    {
        .tcmd_name = "fs_demo_write_then_read",
        .tcmd_func = TCMDEXEC_fs_demo_write_then_read,
        .number_of_args = 0,
    },
    // ****************** END SECTION: lfs_telecommand_defs ******************
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

uint8_t TCMDEXEC_heartbeat_off(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    TASK_heartbeat_is_on = 0;
    snprintf(response_output_buf, response_output_buf_len, "Heartbeat OFF");
    return 0;
}

uint8_t TCMDEXEC_heartbeat_on(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    TASK_heartbeat_is_on = 1;
    snprintf(response_output_buf, response_output_buf_len, "Heartbeat ON");
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
                        char *response_output_buf, uint16_t response_output_buf_len) 
{
    response_output_buf[0] = '\0';
    
    // Store bytes to the MPI firmware binary file for updating the MPI firmware later
    
    uint8_t parse_result = 0;
    uint32_t file_start_address = 0;
    uint32_t mpi_firmware_file_size = 0;

    char file_offset_arg[50] = {0};
    parse_result = TCMD_extract_string_arg((char*)args_str, 0, file_offset_arg, 50);
    if (parse_result > 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: Unable to parse start address.");
        return 1;
    }
    file_start_address = atoi(file_offset_arg);

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

    char file_size_arg[50] = {0};
    parse_result = TCMD_extract_string_arg((char*)args_str, 3, file_size_arg, 50);
    if (parse_result > 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error: Unable to parse firmware file size.");
        return 1;
    }
    mpi_firmware_file_size = atoi(file_size_arg);

    if (file_start_address >= mpi_firmware_file_size) {
        snprintf(response_output_buf, response_output_buf_len, "Error: Start address is larger than size of firmware file.");
        return 1;
    }

    char firmware_filename[TCMD_MAX_STRING_LEN] = {0};
    parse_result = TCMD_arg_as_string((char*)args_str, strlen((char*)args_str), 2, firmware_filename);
    if (parse_result > 0) {
        snprintf(response_output_buf, response_output_buf_len, "Unable to parse argument as filename");
    }


    lfs_file_t file;
    int result = 0;
    bool was_mounted = LFS_is_lfs_mounted;
    if (!was_mounted)
    {
        result = LFS_mount();
        if (result != 0)
        {
            snprintf(response_output_buf, response_output_buf_len, "Unable to mount filesystem");
            return 1;
        }
    }
    result = lfs_file_open(&lfs, &file, firmware_filename, LFS_O_WRONLY | LFS_O_CREAT); 
    if (result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "Unable to open MPI firmware file: %s; response: %d", firmware_filename, result);
        return 1;
    }

    if (file_start_address == 0) {
        result = lfs_file_truncate(&lfs, &file, (lfs_off_t)mpi_firmware_file_size);
        if (result < 0) {
            lfs_file_close(&lfs, &file);
            snprintf(response_output_buf, response_output_buf_len, "Unable to resize MPI firmware file: %s; response: %d", firmware_filename, result);
            return 1;
        }
    }

    result = lfs_file_seek(&lfs, &file, file_start_address, LFS_SEEK_SET);
    if (result < 0) {
        lfs_file_close(&lfs, &file);
        snprintf(response_output_buf, response_output_buf_len, "Unable to seek in MPI firmware file: %s: response: %d", firmware_filename, result);
        return 1;
    }

    result = lfs_file_write(&lfs, &file, firmware_bytes, firmware_bytes_len);
    if (result < 0) {
        lfs_file_close(&lfs, &file);
        snprintf(response_output_buf, response_output_buf_len, "Unable to write MPI firmware page to file: %s; response: %d", firmware_filename, result);
        return 1;
    }

    result = lfs_file_close(&lfs, &file);
    if (result < 0) {
        snprintf(response_output_buf, response_output_buf_len, "Unable to close MPI firmware file: %s; response: %d", firmware_filename, result);
        return 1;
    }

//    if (!was_mounted)
//    {
//        (void)LFS_unmount();
//    }

    snprintf(response_output_buf, response_output_buf_len, "Received MPI firmware page. Wrote %lu bytes to \"%s\" at address %lu", firmware_bytes_len, firmware_filename, (uint32_t)file_start_address);
    return 0;
}

