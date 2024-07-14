
#include "telecommands/telecommand_parser.h"
#include "telecommands/telecommand_definitions.h"
#include "debug_tools/debug_uart.h"


/// @brief Executes a telecommand immediately.
/// @param tcmd_idx The index into `TCMD_telecommand_definitions` for the telecommand to execute.
/// @param args_str_no_parens A cstring containing the arguments for the telecommand. Null-terminated.
/// @param response_output_buf A buffer to store the response from the telecommand.
/// @param response_output_buf_size The size of the `response_output_buf`.
/// @return 0 on success, 254 if `tcmd_idx` is out of bounds, otherwise the error code from the telecommand function.
uint8_t TCMD_execute_parsed_telecommand_now(const uint16_t tcmd_idx, const char args_str_no_parens[],
    TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_size
) {
    // Get the telecommand definition.
    if (tcmd_idx >= TCMD_NUM_TELECOMMANDS) {
        DEBUG_uart_print_str("Error: TCMD_execute_parsed_telecommand: tcmd_idx out of bounds.\n");
        return 254;
    }
    TCMD_TelecommandDefinition_t tcmd_def = TCMD_telecommand_definitions[tcmd_idx];

    DEBUG_uart_print_str("======== Executing telecommand '");
    DEBUG_uart_print_str(tcmd_def.tcmd_name);
    DEBUG_uart_print_str("' ========\n");

    // Handle the telecommand by calling the appropriate function.
    // Null-terminate the args string.
    const uint32_t uptime_before_tcmd_exec_ms = HAL_GetTick();
    const uint8_t tcmd_result = tcmd_def.tcmd_func(
        args_str_no_parens,
        tcmd_channel,
        response_output_buf,
        response_output_buf_size);
    const uint32_t uptime_after_tcmd_exec_ms = HAL_GetTick();
    const uint32_t tcmd_exec_duration_ms = uptime_after_tcmd_exec_ms - uptime_before_tcmd_exec_ms;

    // Print back the response.
    DEBUG_uart_print_str("======== Response (duration=");
    DEBUG_uart_print_int32(tcmd_exec_duration_ms);
    DEBUG_uart_print_str("ms, err=");
    DEBUG_uart_print_uint32(tcmd_result);
    if (tcmd_result != 0) {
        DEBUG_uart_print_str(" !!!!!! ERROR !!!!!!");
    }
    DEBUG_uart_print_str(") ========\n");
    DEBUG_uart_print_str(response_output_buf);
    DEBUG_uart_print_str("\n==========================\n");

    return tcmd_result;
}
