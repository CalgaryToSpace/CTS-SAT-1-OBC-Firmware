#include "telecommands/agenda_telecommands_defs.h"

#include "telecommand_exec/telecommand_definitions.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "telecommand_exec/telecommand_executor.h"
#include "telecommand_exec/agenda_from_file.h"
#include "debug_tools/debug_uart.h"
#include "log/log.h"
#include "transforms/arrays.h"

#include <string.h>
#include <stdio.h>

/// @brief Telecommand: Delete all agendas
/// @param args_str No arguments needed
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success
uint8_t TCMDEXEC_agenda_delete_all(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    snprintf(response_output_buf, response_output_buf_len, "Cleared agenda.");
    TCMD_agenda_delete_all();

    return 0;
}

/// @brief Telecommand: Delete agenda entry by tssent timestamp
/// @param args_str
/// - Arg 0: Timestamp sent (uint64_t) - The tssent timestamp of the agenda entry to delete.
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_agenda_delete_by_tssent(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
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
/// @param args_str No arguments.
/// @return 0 on success, 1 if there are no active pending agenda items.
uint8_t TCMDEXEC_agenda_fetch_jsonl(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    const uint8_t result = TCMD_agenda_fetch();
    return result;
}


/// @brief Telecommand: Delete all agenda entries with a telecommand name
/// @param args_str
/// - Arg 0: telecommand name (string) - The name of the telecommand function in the agenda to delete. (e.g, hello_world)
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success, > 0 on error
uint8_t TCMDEXEC_agenda_delete_by_name(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    const uint8_t result = TCMD_agenda_delete_by_name(args_str);
    return result;
}

/// @brief Telecommand: Enqueue telecommands from a file.
/// @param args_str
/// - Arg 0: File name (string) - The name of the file to enqueue telecommands from.
/// - Arg 1: min_tsexec_inclusive (u64) - Filter to @tsexec >= value (nominal: 0)
/// - Arg 2: max_tsexec_exclusive (u64) - Filter to @tsexec < value (set to 0 for no filter)
/// @return 0 on success, > 0 on error
uint8_t TCMDEXEC_agenda_enqueue_from_file(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    const uint16_t pending_cmd_count_before = TCMD_get_agenda_used_slots_count();

    char arg_file_name[LFS_MAX_PATH_LENGTH];

    // Parse filename argument.
    const uint8_t parse_file_name_result = TCMD_extract_string_arg(
        args_str, 0, arg_file_name, sizeof(arg_file_name)
    );
    if (parse_file_name_result != 0) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing file name arg: Error %d", parse_file_name_result);
        return 99;
    }

    // Parse filter timestamp arguments.
    uint64_t min_tsexec_inclusive;
    const uint8_t parse_filter_arg_min_err = TCMD_extract_uint64_arg(
        args_str, strlen(args_str), 1, &min_tsexec_inclusive
    );
    uint64_t max_tsexec_exclusive;
    const uint8_t parse_filter_arg_max_err = TCMD_extract_uint64_arg(
        args_str, strlen(args_str), 2, &max_tsexec_exclusive
    );
    if (parse_filter_arg_min_err != 0 || parse_filter_arg_max_err != 0) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing file name arg: Error %d", parse_file_name_result);
        return 99;
    }
    // Per docstring, if max_tsexec_exclusive is 0, set it to UINT64_MAX to unrestrict filter.
    if (max_tsexec_exclusive == 0) {
        max_tsexec_exclusive = INT64_MAX - 2; // UINT64_MAX would probably be good too, but this is slightly safer.
    }

    // Parse the file and enqueue the telecommands.
    const uint8_t enqueue_result = TCMD_parse_tcmds_from_file_and_enqueue(
        arg_file_name,
        min_tsexec_inclusive,
        max_tsexec_exclusive,
        100 // Max number of telecommands to enqueue (safety).
    );

    if (enqueue_result != 0) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error enqueuing telecommands from file: Error %d",
            enqueue_result
        );
        return enqueue_result;
    }

    const uint16_t pending_cmd_count_after = TCMD_get_agenda_used_slots_count();

    snprintf(
        response_output_buf,
        response_output_buf_len,
        "Successfully enqueued telecommands from file. Pending Commands: %d -> %d (%d added)",
        pending_cmd_count_before,
        pending_cmd_count_after,
        pending_cmd_count_after - pending_cmd_count_before
    );

    return 0; // Success.
}
