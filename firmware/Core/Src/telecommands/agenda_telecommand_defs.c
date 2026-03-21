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
#include <stdbool.h>



/// @brief Fetches the active agendas and writes a minified JSON dict-of-lists to the response buffer.
/// @param args_str No arguments.
/// @param response_output_buf Buffer to write the JSON output to.
/// @param response_output_buf_size Size of the response buffer.
/// @return 0 on success, 1 if there are no pending entries, 2 if the buffer was too small (agenda too long).
uint8_t TCMDEXEC_agenda_fetch_json_grouped(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_size
) {
    // TODO: Could use the args to filter.
    const uint16_t pending_count = TCMD_get_agenda_used_slots_count();

    if (pending_count == 0) {
        snprintf(response_output_buf, response_output_buf_size, "{}");
        return 1;
    }

    // We'll build the JSON manually by iterating over all tcmd_names that appear in the agenda.
    // Output format: {"tcmd_name":[{"tssent":xxx,"tsexec":xxx}, ...], ...}

    uint16_t buf_pos = 0;
    bool first_key = true;

    // Write opening brace.
    buf_pos += snprintf(response_output_buf + buf_pos, response_output_buf_size - buf_pos, "{");

    // Outer loop: iterate over each unique telecommand index present in the agenda.
    for (uint16_t tcmd_idx = 0; tcmd_idx < TCMD_NUM_TELECOMMANDS; tcmd_idx++) {
        // Inner pass 1: check if this tcmd_idx appears at all in the pending agenda.
        bool found = false;
        for (uint16_t slot_num = 0; slot_num < TCMD_AGENDA_SIZE; slot_num++) {
            if (
                TCMD_agenda_is_valid[slot_num] == TCMD_AGENDA_ENTRY_VALID_AND_PENDING
                && TCMD_agenda[slot_num].tcmd_idx == tcmd_idx
            ) {
                found = true;
                break;
            }
        }
        if (!found) {
            continue;
        }

        // Write the key: ,"tcmd_name":[
        buf_pos += snprintf(
            response_output_buf + buf_pos, response_output_buf_size - buf_pos,
            "%s\"%s\":[",
            first_key ? "" : ",",
            TCMD_telecommand_definitions[tcmd_idx].tcmd_name
        );
        first_key = false;

        // Inner pass 2: write each matching slot as an entry in the list.
        bool first_entry = true;
        for (uint16_t slot_num = 0; slot_num < TCMD_AGENDA_SIZE; slot_num++) {
            if (
                TCMD_agenda_is_valid[slot_num] != TCMD_AGENDA_ENTRY_VALID_AND_PENDING
                || TCMD_agenda[slot_num].tcmd_idx != tcmd_idx
            ) {
                continue;
            }

            char tssent_str[32];
            GEN_uint64_to_str(TCMD_agenda[slot_num].timestamp_sent, tssent_str);
            char tsexec_str[32];
            GEN_uint64_to_str(TCMD_agenda[slot_num].timestamp_to_execute, tsexec_str);

            buf_pos += snprintf(
                response_output_buf + buf_pos, response_output_buf_size - buf_pos,
                "%s{\"tssent\":%s,\"tsexec\":%s}",
                first_entry ? "" : ",",
                tssent_str,
                tsexec_str
            );
            first_entry = false;
        }

        // Close the list for this key.
        buf_pos += snprintf(response_output_buf + buf_pos, response_output_buf_size - buf_pos, "]");
    }

    // Write closing brace.
    buf_pos += snprintf(response_output_buf + buf_pos, response_output_buf_size - buf_pos, "}");

    if (buf_pos >= response_output_buf_size) {
        // The buffer was too small; the output is truncated.
        snprintf(
            response_output_buf, response_output_buf_size,
            "Buffer too small (%u bytes, needed >%u).",
            response_output_buf_size, buf_pos
        );
        return 2;
    }

    return 0;
}


/// @brief Telecommand: Fetch all pending agenda items, and log them each as JSONL
/// @param args_str No arguments.
/// @return 0 on success, 1 if there are no active pending agenda items.
uint8_t TCMDEXEC_agenda_fetch_logged_jsonl(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    const uint8_t result = TCMD_log_pending_agenda_entries();
    return result;
}


/// @brief Telecommand: Delete all agendas
/// @param args_str No arguments needed
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success
uint8_t TCMDEXEC_agenda_delete_all(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    uint16_t num_deleted = 0;
    for (uint16_t slot_num = 0; slot_num < TCMD_AGENDA_SIZE; slot_num++) {
        if (TCMD_agenda_is_valid[slot_num] == TCMD_AGENDA_ENTRY_VALID_AND_PENDING) {
            TCMD_agenda_is_valid[slot_num] = TCMD_AGENDA_ENTRY_INVALID;
            num_deleted++;
        }
    }
    LOG_message(
        LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Deleted all %d entries from the agenda.",
        num_deleted
    );

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

    // Loop through the agenda and check for valid agendas and if the timestamp matches.
    char *tcmd_name = NULL;
    uint16_t deleted_count = 0;
    for (uint16_t slot_num = 0; slot_num < TCMD_AGENDA_SIZE; slot_num++) {
        if (
            (TCMD_agenda_is_valid[slot_num] == TCMD_AGENDA_ENTRY_VALID_AND_PENDING)
            && (TCMD_agenda[slot_num].timestamp_sent == tssent)
        ) {
            // Set agenda entry as invalid.
            TCMD_agenda_is_valid[slot_num] = TCMD_AGENDA_ENTRY_INVALID;
            deleted_count++;
            tcmd_name = TCMD_telecommand_definitions[TCMD_agenda[slot_num].tcmd_idx].tcmd_name;
        }
    }

    if (deleted_count == 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Telecommand with tssent=%s not found in agenda.",
            tssent_str
        );
        return 1;
    }
    
    snprintf(
        response_output_buf, response_output_buf_len,
        "Telecommand with tssent=%s (%s) deleted from agenda. Deleted %u telecommand(s).",
        tssent_str,
        tcmd_name,
        deleted_count
    );
    return 0;
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
    const char *telecommand_name = args_str; // Single argument, easy.

    // Get count of active agendas.
    const uint8_t pending_count_at_start = TCMD_get_agenda_used_slots_count();

    if (pending_count_at_start == 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "TCMD_agenda_delete_by_name: No pending telecommands in the agenda."
        );
        return 1;
    }

    // Loop through the telecommand definitions and check if the passed function name is valid
    bool is_valid_telecommand_name = false;
    uint16_t tcmd_idx = 0;
    for (uint16_t idx = 0; idx < TCMD_NUM_TELECOMMANDS; idx++) {
        if (strcasecmp(TCMD_telecommand_definitions[idx].tcmd_name, telecommand_name) == 0) {
            is_valid_telecommand_name = true;
            tcmd_idx = idx;
            break;
        }
    }

    if (!is_valid_telecommand_name) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "TCMD_agenda_delete_by_name: Invalid telecommand name passed in the function."
        );
        return 2;
    }

    // Loop through the agenda and check for valid agendas.
    uint16_t deleted_count = 0;
    for (uint16_t slot_num = 0; slot_num < TCMD_AGENDA_SIZE; slot_num++) {
        if (
            (TCMD_agenda_is_valid[slot_num] == TCMD_AGENDA_ENTRY_VALID_AND_PENDING) // It's valid.
            && (TCMD_agenda[slot_num].tcmd_idx == tcmd_idx) // It's the one we're searching for.
        ) {
            // Set agenda as invalid ("delete" it).
            TCMD_agenda_is_valid[slot_num] = TCMD_AGENDA_ENTRY_INVALID;
            deleted_count++;
        }
    }
    
    snprintf(
        response_output_buf, response_output_buf_len,
        "TCMD_agenda_delete_by_name: Removed %d/%d telecommands with name='%s' from agenda. %d remain.",
        deleted_count,
        pending_count_at_start,
        telecommand_name,
        TCMD_get_agenda_used_slots_count()
    );

    return 0;
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
    
    const uint16_t pending_cmd_count_after = TCMD_get_agenda_used_slots_count();

    if (enqueue_result != 0) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error enqueuing telecommands from file: Error %d. Pending Commands: %d -> %d (%d added).",
            enqueue_result,
            // Even if the error is non-zero, some of the telecommands may have been enqueued.
            pending_cmd_count_before,
            pending_cmd_count_after,
            pending_cmd_count_after - pending_cmd_count_before
        );
        return enqueue_result;
    }

    snprintf(
        response_output_buf,
        response_output_buf_len,
        "Successfully enqueued telecommands from file. Pending Commands: %d -> %d (%d added).",
        pending_cmd_count_before,
        pending_cmd_count_after,
        pending_cmd_count_after - pending_cmd_count_before
    );

    return 0; // Success.
}
