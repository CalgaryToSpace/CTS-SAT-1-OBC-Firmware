
#include "telecommand_exec/telecommand_definitions.h"
#include "telecommand_exec/telecommand_args_helpers.h"

#include "telecommands/testing_telecommand_defs.h"
#include "debug_tools/debug_uart.h"

#include "unit_tests/unit_test_executor.h"

#include <stdio.h>
#include <inttypes.h>
#include <string.h>


/// @brief A demo telecommand that echoes back the argument it received.
/// @param args_str
/// - Arg 0: The string to echo back.
uint8_t TCMDEXEC_echo_back_args(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    snprintf(response_output_buf, response_output_buf_len, "SUCCESS: Echo Args: '%s'\n", args_str);
    // TODO: handle args_str being too long
    return 0;
}

/// @brief A demo telecommand that echoes back each integer argument it received.
/// @param args_str 3 integer arguments to echo back.
/// - Arg 0: The first integer to echo back.
/// - Arg 1: The second integer to echo back.
/// - Arg 2: The third integer to echo back.
/// @return 0 if all ints are parsed successfully, otherwise the error code of the first failed parse.
uint8_t TCMDEXEC_echo_back_uint32_args(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    response_output_buf[0] = '\0'; // clear the response buffer

    for (uint8_t arg_num = 0; arg_num < 3; arg_num++) {
        uint64_t arg_uint64;
        uint8_t parse_result = TCMD_extract_uint64_arg(
            args_str, strlen(args_str), arg_num, &arg_uint64);
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


uint8_t TCMDEXEC_run_all_unit_tests(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    TEST_run_all_unit_tests_and_log(response_output_buf, response_output_buf_len);
    return 0;
}

/// @brief Delay for a specified number of milliseconds, for testing purposes.
/// @param args_str 1 argument: delay_ms (uint64_t)
/// - Arg 0: delay_ms (uint64_t) - The number of milliseconds to delay for. <=300_000ms.
/// @return 0 on success, 1 on error
/// @note This is most useful for testing/triggering the watchdog timer.
uint8_t TCMDEXEC_demo_blocking_delay(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    uint64_t delay_ms;
    const uint8_t parse_result = TCMD_extract_uint64_arg(
        args_str, strlen(args_str), 0, &delay_ms
    );
    if (parse_result > 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error parsing delay_ms: Err=%d", parse_result);
        return 1;
    }
    
    if (delay_ms > 300000) {
        snprintf(response_output_buf, response_output_buf_len, "Delay too long. Must be <=300_000 ms.");
        return 1;
    }

    const uint32_t delay_ms_u32 = (uint32_t)delay_ms;
    
    HAL_Delay(delay_ms_u32);

    snprintf(response_output_buf, response_output_buf_len, "Delayed for %" PRIu32 " ms", delay_ms_u32);
    return 0;
}
