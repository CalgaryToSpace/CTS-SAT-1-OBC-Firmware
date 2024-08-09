#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_args_helpers.h"
#include "telecommands/telecommand_executor.h"
#include "debug_tools/debug_uart.h"
#include "telecommands/agenda_telecommands_defs.h"

#include <string.h>
#include <stdio.h>

/// @brief Telecommand: Delete all agendas
/// @param args_str No arguments needed
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success
uint8_t TCMDEXEC_agenda_delete_all(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    DEBUG_uart_print_str("Deleting all entires from the Agenda...\n"); 
    TCMD_agenda_delete_all();

    return 0;
}

/// @brief Telecommand: Delete agenda entry by timestamp
/// @param args_str
/// - Arg 0: Timestamp sent (uint64_t) - The timestamp of the agenda entry to delete.
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_agenda_delete_by_tssent(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t tssent = 0;

    // Parse the arg string passed into a uint64_t for the timestamp sent
    uint8_t parse_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &tssent);

    //Checking if the parsing was right
    if (parse_result > 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error parsing timestamp sent: Err=%d", parse_result);
        return 1;
    }

    // Pass the tssent into the function that handles the delete from the stack
    uint8_t result = TCMD_agenda_delete_by_tssent(&tssent);

    if (result == 1)
    {
        DEBUG_uart_print_str("Telecommand with tssent: ");
        DEBUG_uart_print_uint64(tssent);
        DEBUG_uart_print_str(" not found in agenda.\n");
    }

    if (result == 0)
    {
        DEBUG_uart_print_str("Telecommand with tssent: ");
        DEBUG_uart_print_uint64(tssent);
        DEBUG_uart_print_str(" deleted from agenda");
        DEBUG_uart_print_str("\n");
    }
    
    return result;
}