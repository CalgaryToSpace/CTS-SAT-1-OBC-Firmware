#include "telecommand_exec/telecommand_definitions.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "telecommand_exec/telecommand_executor.h"
#include "debug_tools/debug_uart.h"
#include "telecommands/agenda_telecommands_defs.h"
#include "log/log.h"
#include "transforms/arrays.h"

#include <string.h>
#include <stdio.h>

/// @brief Telecommand: Delete all agendas
/// @param args_str No arguments needed
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success
uint8_t TCMDEXEC_agenda_delete_all(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    snprintf(response_output_buf, response_output_buf_len, "Cleared agenda.");
    TCMD_agenda_delete_all();

    return 0;
}

/// @brief Telecommand: Delete agenda entry by tssent timestamp
/// @param args_str
/// - Arg 0: Timestamp sent (uint64_t) - The tssent timestamp of the agenda entry to delete.
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_agenda_delete_by_tssent(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    uint64_t tssent = 0;

    // Parse the arg string passed into a uint64_t for the timestamp sent
    const uint8_t parse_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &tssent);

    // Checking if the argument was valid.
    if (parse_result > 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error parsing timestamp sent: Err=%d", parse_result);
        return 1;
    }
    char tssent_str[32];
    GEN_uint64_to_str(tssent, tssent_str);

    // Pass the tssent into the function that handles the delete from the stack
    const uint8_t result = TCMD_agenda_delete_by_tssent(tssent);

    if (result != 0)
    {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Telecommand with tssent=%s not found in agenda.",
            tssent_str
        );
        return 1;
    }

    LOG_message(
        LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Telecommand with tssent=%s deleted from agenda.",
        tssent_str
    );
    return 0;
}

/// @brief Telecommand: Fetch all pending agenda items, and log them each as JSONL
/// @param args_str No arguments needed
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success, 1 if there are no active agendas.
uint8_t TCMDEXEC_agenda_fetch_jsonl(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
     
    const uint8_t result = TCMD_agenda_fetch();
    return result;
}


/// @brief Telecommand: Delete all agenda entries with a telecommand name
/// @param args_str
/// - Arg 0: telecommand name (string) - The name of the telecommand function in the agenda to delete. (e.g, hello_world)
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success, > 0 on error
uint8_t TCMDEXEC_agenda_delete_by_name(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    const uint8_t result = TCMD_agenda_delete_by_name(args_str);
    return result;
}
