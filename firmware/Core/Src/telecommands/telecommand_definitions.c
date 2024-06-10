
#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_args_helpers.h"
#include "transforms/arrays.h"
#include "unit_tests/unit_test_executor.h"
#include "debug_tools/debug_uart.h"
#include "transforms/string_helpers.h"
#include "configuration/configuration.h"

// Additional telecommand definitions files:
#include "telecommands/flash_telecommand_defs.h"
#include "telecommands/lfs_telecommand_defs.h"
#include "telecommands/timekeeping_telecommand_defs.h"


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "telecommand_definitions.h"

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
        .tcmd_name = "alis_hello_world",
        .tcmd_func = TCMDEXEC_hello_ali,
        .number_of_args = 0,
    }

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
uint8_t TCMDEXEC_hello_ali(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                           char *response_output_buf, uint16_t response_output_buf_len)
{
    debug_uart_print_str((char *)args_str);
    debug_uart_print_str("\n");
    uint8_t args[2][20];
    uint8_t args_count = split_string_by_delimiter(args_str, strlen((char *)args_str), ',', args, 1);

    debug_uart_print_str("Args count: ");
    debug_uart_print_uint32(args_count);
    debug_uart_print_str("\n");
    debug_uart_print_str((char *)args[0]);
    debug_uart_print_str("\n");
    snprintf(response_output_buf, response_output_buf_len, "Hello, world From Ali!\n");

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
