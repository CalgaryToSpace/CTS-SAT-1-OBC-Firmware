
#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_args_helpers.h"
#include "transforms/arrays.h"
#include "unit_tests/unit_test_executor.h"
#include "debug_tools/debug_uart.h"
#include "transforms/string_helpers.h"

// Additional telecommand definitions files:
#include "telecommands/flash_telecommand_defs.h"
#include "telecommands/lfs_telecommand_defs.h"
#include "telecommands/timekeeping_telecommand_defs.h"


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

extern volatile uint8_t TASK_heartbeat_is_on;

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
        .tcmd_name = "get_system_time",
        .tcmd_func = TCMDEXEC_get_system_time,
        .number_of_args = 0,
    },
    {
        .tcmd_name = "set_system_time",
        .tcmd_func = TCMDEXEC_set_system_time,
        .number_of_args = 1,
    },
    {
        .tcmd_name = "available_telecommands",
        .tcmd_func = TCMDEXEC_available_telecommands,
        .number_of_args = 0,
    },
    {
        .tcmd_name = "set_config_var",
        .tcmd_func = TCMDEXEC_set_configuration_variable,
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
    {
        .tcmd_name = "flash_benchmark_erase_write_read",
        .tcmd_func = TCMDEXEC_flash_benchmark_erase_write_read,
        .number_of_args = 3,
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
        .tcmd_name = "fs_demo_write_then_read",
        .tcmd_func = TCMDEXEC_fs_demo_write_then_read,
        .number_of_args = 0,
    },
    {
        .tcmd_name = "fs_benchmark_write_read",
        .tcmd_func = TCMDEXEC_fs_benchmark_write_read,
        .number_of_args = 2,
    },
    // ****************** END SECTION: lfs_telecommand_defs ******************

};

// extern
const int16_t TCMD_NUM_TELECOMMANDS = sizeof(TCMD_telecommand_definitions) / sizeof(TCMD_TelecommandDefinition_t);

// each telecommand function must have the following signature:
// uint8_t <function_name>(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
//                          char *response_output_buf, uint16_t response_output_buf_len)

uint8_t TCMDEXEC_hello_world(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                             char *response_output_buf, uint16_t response_output_buf_len)
{
    snprintf(response_output_buf, response_output_buf_len, "Hello, world!\n");
    return 0;
}

uint8_t TCMDEXEC_heartbeat_off(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                               char *response_output_buf, uint16_t response_output_buf_len)
{
    TASK_heartbeat_is_on = 0;
    snprintf(response_output_buf, response_output_buf_len, "Heartbeat OFF");
    return 0;
}

uint8_t TCMDEXEC_heartbeat_on(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                              char *response_output_buf, uint16_t response_output_buf_len)
{
    TASK_heartbeat_is_on = 1;
    snprintf(response_output_buf, response_output_buf_len, "Heartbeat ON");
}

uint8_t TCMDEXEC_set_configuration_variable(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len)
{
    DEBUG_uart_print_str((char *)args_str);
    DEBUG_uart_print_str("\n");
    char args[3][20];
    for (uint8_t i = 0; i < 3; i++)
    {
        memset(args[i], 0, 20);
    }
    const uint8_t args_len = strlen((char *)args_str);
    const uint8_t args_count = split_string_by_delimiter((const char *)args_str, args_len, ',', args, 3);
    char args_count_str[5];
    snprintf(args_count_str, 5, "%d", args_count);
    args_count_str[4] = '\0';

    memset(response_output_buf, 0, response_output_buf_len);

    strncat(response_output_buf, "Hello, world From Ali!\n", 24);

    strncat(response_output_buf, "Args count: ", 13);
    strncat(response_output_buf, args_count_str, 5);
    strcat(response_output_buf, "\n");

    for (uint8_t i = 0; i < args_count; i++)
    {
        strncat(response_output_buf, (char *)args[i], strlen((char *)args[i]));
        strcat(response_output_buf, "\n");
    }
    return 0;
}
uint8_t TCMDEXEC_heartbeat_off(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                               char *response_output_buf, uint16_t response_output_buf_len)
{
    TASK_heartbeat_is_on = 0;
    snprintf(response_output_buf, response_output_buf_len, "Heartbeat OFF");
    return 0;
}

uint8_t TCMDEXEC_core_system_stats(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len)
{
    snprintf(response_output_buf, response_output_buf_len, "System stats: TODO\n");
    return 0;
}

uint8_t TCMDEXEC_echo_back_args(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                char *response_output_buf, uint16_t response_output_buf_len)
{

    snprintf(response_output_buf, response_output_buf_len, "SUCCESS: Echo Args: '%s'\n", args_str);
    // TODO: handle args_str being too long
    return 0;
}

uint8_t TCMDEXEC_echo_back_uint32_args(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                       char *response_output_buf, uint16_t response_output_buf_len)
{
    response_output_buf[0] = '\0'; // clear the response buffer

    for (uint8_t arg_num = 0; arg_num < 10; arg_num++)
    {
        uint64_t arg_uint64;
        uint8_t parse_result = TCMD_extract_uint64_arg(
            (char *)args_str, strlen((char *)args_str), arg_num, &arg_uint64);
        if (parse_result > 0)
        {
            // error parsing
            snprintf(
                &response_output_buf[strlen(response_output_buf)],
                response_output_buf_len - strlen(response_output_buf) - 1,
                "Arg%d=error%d, ", arg_num, parse_result);
        }
        else
        {
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
                                    char *response_output_buf, uint16_t response_output_buf_len)
{
    TEST_run_all_unit_tests_and_log(response_output_buf, response_output_buf_len);
    return 0;
}

uint8_t TCMDEXEC_available_telecommands(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    char *p = response_output_buf;
    uint16_t remaining_space = response_output_buf_len;

    // Start response with header
    snprintf(p, remaining_space, "Available_telecommands\n");
    const uint16_t header_length = strlen(p);
    p += header_length;
    remaining_space -= header_length;

    // Append each telecommand name to the response
    for (uint16_t tcmd_idx = 0; tcmd_idx < TCMD_NUM_TELECOMMANDS; tcmd_idx++) {
        const uint16_t line_length = snprintf(
            p,
            remaining_space,
            "%3u) %s\n",
            tcmd_idx + 1,
            TCMD_telecommand_definitions[tcmd_idx].tcmd_name
        );
        if (line_length >= remaining_space) {
            // Not enough space left to append more telecommands
            break;
        }
        p += line_length;
        remaining_space -= line_length;
    }

    return 0;
}
