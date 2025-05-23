#include "telecommand_exec/telecommand_parser.h"
#include "telecommand_exec/telecommand_definitions.h"
#include "telecommand_exec/telecommand_executor.h"
#include "telecommand_exec/telecommand_types.h"
#include "debug_tools/debug_uart.h"
#include "timekeeping/timekeeping.h"
#include "log/log.h"
#include "transforms/arrays.h"
#include "config/configuration.h"
#include "littlefs/littlefs_helper.h"
#include "comms_drivers/comms_tx.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>


uint32_t TCMD_total_tcmd_queued_count = 0;
uint64_t TCMD_latest_received_tcmd_timestamp_sent = 0;


///@brief  The head of the circular buffer of timestamps of telecommands that have been sent.
uint16_t  TCMD_timestamp_sent_head = 0;
/// @brief The circular buffer of timestamps of telecommands that have been sent.
uint64_t TCMD_timestamp_sent_store[TCMD_TIMESTAMP_RECORD_SIZE] = {0};

/// @brief  The agenda (schedule queue) of telecommands to execute.
TCMD_parsed_tcmd_to_execute_t TCMD_agenda[TCMD_AGENDA_SIZE];
// TODO: consider an optimization to store the args_str_no_parens in a separate buffer, to save a ton of memory.

/// @brief  A flag indicating whether a given index in `TCMD_agenda` is valid
///         (i.e., filled with a not-yet-executed command).
uint8_t TCMD_agenda_is_valid[TCMD_AGENDA_SIZE] = {0};

/// @brief Converts a TCMD_TelecommandChannel_enum_t to a string representation.
/// @param channel Input TCMD_TelecommandChannel_enum_t
/// @return A pointer to a C-string representing the TCMD_TelecommandChannel_enum_t.
const char* telecommand_channel_enum_to_str(TCMD_TelecommandChannel_enum_t channel) {
    switch (channel) {
        case TCMD_TelecommandChannel_DEBUG_UART     :return "DEBUG_UART";
        case TCMD_TelecommandChannel_RADIO1         :return "RADIO1";
        default                                     :return "UNKNOWN_CHANNEL";
    }
}



/// @brief Adds a telecommand to the agenda (schedule/queue) of telecommands to execute.
/// @param parsed_tcmd The parsed telecommand to add to the agenda.
/// @return 0 on success, 1 if the agenda is full.
/// @note Performs a deep copy of the `parsed_tcmd` arg into the agenda.
uint8_t TCMD_add_tcmd_to_agenda(const TCMD_parsed_tcmd_to_execute_t *parsed_tcmd) {
    // Find the first empty slot in the agenda.
    for (uint16_t slot_num = 0; slot_num < TCMD_AGENDA_SIZE; slot_num++) {
        // Skip filled slots.
        if (TCMD_agenda_is_valid[slot_num]) {
            continue;
        }

        // Skip if timstamps are not needed.
        if(TCMD_require_unique_tssent) {
            // Check to see if timestamp is in the circular buffer.
            for (uint32_t i = 0; i < TCMD_timestamp_sent_head; i++) {
                if(parsed_tcmd->timestamp_sent == TCMD_timestamp_sent_store[i]) {
                    // Skip this telecommand.
                    LOG_message(
                        LOG_SYSTEM_TELECOMMAND, 
                        LOG_SEVERITY_WARNING, 
                        LOG_SINK_ALL, 
                        "Telecommand skipped due to repeated tssent."
                    );
                    return 1; 
                }
            }
        }

        // Add the timestamp to the circular buffer.
        // This mechanism prevents command replays (executing the same command twice inadvertently).
        if (parsed_tcmd->timestamp_sent > 0) {
            TCMD_timestamp_sent_store[TCMD_timestamp_sent_head] = parsed_tcmd->timestamp_sent;
            TCMD_timestamp_sent_head = (TCMD_timestamp_sent_head + 1) % TCMD_TIMESTAMP_RECORD_SIZE;
        }

        // Copy the parsed telecommand into the agenda.
        TCMD_agenda[slot_num].tcmd_idx = parsed_tcmd->tcmd_idx;
        TCMD_agenda[slot_num].tcmd_channel = parsed_tcmd->tcmd_channel;
        TCMD_agenda[slot_num].timestamp_sent = parsed_tcmd->timestamp_sent;
        TCMD_agenda[slot_num].timestamp_to_execute = parsed_tcmd->timestamp_to_execute;

        for (uint16_t j = 0; j < TCMD_ARGS_STR_NO_PARENS_SIZE; j++) {
            TCMD_agenda[slot_num].args_str_no_parens[j] = parsed_tcmd->args_str_no_parens[j];
        }

        // Copy the log filename into the agenda.
        for (uint16_t j = 0; j < TCMD_MAX_RESP_FNAME_LEN; j++) {
            TCMD_agenda[slot_num].resp_fname[j] = parsed_tcmd->resp_fname[j];
        }
        // Mark the slot as valid.
        TCMD_agenda_is_valid[slot_num] = 1;

        // Incrementing counters used for stats 
        TCMD_total_tcmd_queued_count++; 
        TCMD_latest_received_tcmd_timestamp_sent = parsed_tcmd->timestamp_sent;
        return 0;
    }
    return 1;
}


/// @brief Gets the number of used slots in the agenda.
/// @return The number of currently-filled slots in the agenda.
/// @note This function is mostly intended for "system stats" telecommands and logging.
uint16_t TCMD_get_agenda_used_slots_count() {
    // TODO: consider an easy O(1) optimization by keeping track of the number of used slots.
    uint16_t count = 0;
    for (uint16_t slot_num = 0; slot_num < TCMD_AGENDA_SIZE; slot_num++) {
        if (TCMD_agenda_is_valid[slot_num]) {
            count++;
        }
    }
    return count;
}

/// @brief Finds the index into `TCMD_agenda` (`slot_num`) of the next telecommand to execute.
/// @return The index into `TCMD_agenda` of the next telecommand to execute, or -1 if none are available/ready.
/// @note This function will return the `slot_num` which has the lowest `timestamp_to_execute` value.
///      If multiple slots have the same `timestamp_to_execute`, the lowest `slot_num` will be returned.
int16_t TCMD_get_next_tcmd_agenda_slot_to_execute() {
    // TODO: benchmark this, and consider an O(1) optimization by keeping track of the timestamp of the next upcoming telecommand timestamp.
    
    const uint64_t current_timestamp_ms = TIM_get_current_unix_epoch_time_ms();

    int16_t earliest_slot_num = -1;
    uint64_t earliest_timestamp = UINT64_MAX;
    for (uint16_t slot_num = 0; slot_num < TCMD_AGENDA_SIZE; slot_num++) {
        // Skip empty/invalid slots.
        if (! TCMD_agenda_is_valid[slot_num]) {
            continue;
        }

        // Optimization: return the first 0-timestamp slot found, if any.
        if (TCMD_agenda[slot_num].timestamp_to_execute == 0) {
            return slot_num;
        }
        
        if (TCMD_agenda[slot_num].timestamp_to_execute < earliest_timestamp && TCMD_agenda[slot_num].timestamp_to_execute <= current_timestamp_ms) {
            earliest_slot_num = slot_num;
            earliest_timestamp = TCMD_agenda[slot_num].timestamp_to_execute;
        }
    }

    return earliest_slot_num;
}

/// @brief Executes a telecommand immediately, based on the minimum info required to execute a telecommand.
/// @param tcmd_idx The index into `TCMD_telecommand_definitions` for the telecommand to execute.
/// @param args_str_no_parens A cstring containing the arguments for the telecommand. Null-terminated.
/// @param timestamp_sent The value of the `@tssent` field when the telecommand was received.
/// @param response_output_buf A buffer to store the response from the telecommand.
/// @param response_output_buf_size The size of the `response_output_buf`.
/// @return 0 on success, 254 if `tcmd_idx` is out of bounds, otherwise the error code from the telecommand function.
static uint8_t TCMD_execute_parsed_telecommand_now(
    const uint16_t tcmd_idx,
    const char args_str_no_parens[],
    const uint64_t timestamp_sent,
    TCMD_TelecommandChannel_enum_t tcmd_channel,
    const char * tcmd_resp_fname,
    char *response_output_buf, uint16_t response_output_buf_size
) {
    // Get the telecommand definition.
    if (tcmd_idx >= TCMD_NUM_TELECOMMANDS) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error: TCMD_execute_parsed_telecommand: tcmd_idx out of bounds (%u).",
            tcmd_idx
        );
        return 254;
    }
    TCMD_TelecommandDefinition_t tcmd_def = TCMD_telecommand_definitions[tcmd_idx];

    LOG_message(
        LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "🚀 Executing telecommand '%s'.",
        tcmd_def.tcmd_name
    );

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
    LOG_message(
        LOG_SYSTEM_TELECOMMAND,
        (tcmd_result == 0) ? LOG_SEVERITY_NORMAL : LOG_SEVERITY_ERROR,
        LOG_SINK_ALL,
        "%s Telecommand '%s' executed. Duration=%lums, err=%u",
        (tcmd_result == 0) ? "🟢" : "🔴",
        tcmd_def.tcmd_name,
        tcmd_exec_duration_ms,
        tcmd_result
    );

    COMMS_downlink_tcmd_response(
        timestamp_sent,
        tcmd_result,
        tcmd_exec_duration_ms,
        response_output_buf,
        strnlen(response_output_buf, response_output_buf_size) + 1 // +1 for null terminator
    );

    DEBUG_uart_print_str(response_output_buf);
    DEBUG_uart_print_str("\n");

    // If the filename is not empty, log the telecommand to the file.
    if (tcmd_resp_fname != NULL && strnlen(tcmd_resp_fname, TCMD_MAX_RESP_FNAME_LEN) > 0) {
        // Internally Logs errors, no point in collecting error here
        TCMD_log_to_file(tcmd_resp_fname, response_output_buf);
    }
    return tcmd_result;
}

/// @brief Executes a telecommand from the agenda immediately.
/// @param tcmd_agenda_slot_num The index into `TCMD_agenda` for the telecommand to execute.
/// @param response_output_buf A buffer to store the response from the telecommand.
/// @param response_output_buf_size The size of the `response_output_buf`.
/// @return 0 on success, 253/254 if an error occurs in the callers, otherwise the error code from the telecommand function.
uint8_t TCMD_execute_telecommand_in_agenda(const uint16_t tcmd_agenda_slot_num,
    char *response_output_buf, uint16_t response_output_buf_size
) {
    if (! TCMD_agenda_is_valid[tcmd_agenda_slot_num]) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error: TCMD_execute_telecommand_in_agenda: slot %u is invalidated",
            tcmd_agenda_slot_num
        );
        return 253;
    }

    // Mark the slot as invalid because it's executed.
    // Do it now, in case the execution does Undefined Behaviour (but could also do it after the call just fine).
    TCMD_agenda_is_valid[tcmd_agenda_slot_num] = 0;

    char tssent_str[32];
    GEN_uint64_to_str(TCMD_agenda[tcmd_agenda_slot_num].timestamp_sent, tssent_str);
    char tsexec_str[32];
    GEN_uint64_to_str(TCMD_agenda[tcmd_agenda_slot_num].timestamp_to_execute, tsexec_str);
    uint8_t resp_fname_len = strlen(TCMD_agenda[tcmd_agenda_slot_num].resp_fname);
    LOG_message(
        LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "Executing telecommand from agenda slot %d, sent at tssent=%s, scheduled for tsexec=%s, logging to file: '%s'.",
        tcmd_agenda_slot_num,
        tssent_str,
        tsexec_str,
        (resp_fname_len > 0) ? TCMD_agenda[tcmd_agenda_slot_num].resp_fname : "None"
    );

    // Execute the telecommand.
    return TCMD_execute_parsed_telecommand_now(
        TCMD_agenda[tcmd_agenda_slot_num].tcmd_idx,
        TCMD_agenda[tcmd_agenda_slot_num].args_str_no_parens,
        TCMD_agenda[tcmd_agenda_slot_num].timestamp_sent,
        TCMD_agenda[tcmd_agenda_slot_num].tcmd_channel,
        TCMD_agenda[tcmd_agenda_slot_num].resp_fname,
        response_output_buf,
        response_output_buf_size
    );
}

/// @brief Logs a message to a file.
/// @param filename The name of the file to log to.
/// @param message The message to log.
/// @return 0 on success, 1 if mounting LFS failed, 2 if writing to the file failed.
/// @note This function is used to log telecommand responses to a file.
uint8_t TCMD_log_to_file(const char *filename, const char *message)
{
    if (!LFS_is_lfs_mounted) {
        const int8_t mount_ret = LFS_mount();
        if (mount_ret < 0) {
            LOG_message(
                LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                "Error: TCMD_log_to_file: Failed to mount LFS. Error code: %d",
                mount_ret
            );
            return 1;
        }
    }

    // FIXME(Issue #389): Should write a timestamp probably. Maybe a telecommand name too, and maybe the arg string.
    const int8_t write_file_return = LFS_write_file(
        filename,
        (uint8_t *)message,
        strlen(message)
    );
    if (write_file_return != 0) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error: TCMD_log_to_file: Failed to write to file '%s'. Error code: %d",
            filename,
            write_file_return
        );
        return 2;
    }
    LOG_message(
        LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "TCMD_log_to_file: Successfully wrote to file '%s'.",
        filename
    );
    return 0;
} 

/// @brief Deletes all entries from the agenda.
/// @return Cannot fail, so no return value.
void TCMD_agenda_delete_all() {
    uint16_t num_deleted = 0;
    for (uint16_t slot_num = 0; slot_num < TCMD_AGENDA_SIZE; slot_num++) {
        if (TCMD_agenda_is_valid[slot_num]) {
            TCMD_agenda_is_valid[slot_num] = 0;
            num_deleted++;
        }
    }
    LOG_message(
        LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "TCMD_agenda_delete_all: Deleted all %d entries from the agenda.",
        num_deleted
    );
}

/// @brief Deletes a telecommand from the agenda by its `tssent` (timestamp sent) field.
/// @param tssent The `timestamp_sent` value of the telecommand to delete.
/// @return 0 on success, 1 if the telecommand was not found.
/// @note Calls `LOG_message()` to log the deletion before all returns.
uint8_t TCMD_agenda_delete_by_tssent(uint64_t tssent) {
    char tssent_str[32];
    GEN_uint64_to_str(tssent, tssent_str);

    // Loop through the agenda and check for valid agendas and if the timestamp matches
    for (uint16_t slot_num = 0; slot_num < TCMD_AGENDA_SIZE; slot_num++) {
        if (TCMD_agenda_is_valid[slot_num] && TCMD_agenda[slot_num].timestamp_sent == tssent) {

            // Set agenda as invalid
            TCMD_agenda_is_valid[slot_num] = 0;
            LOG_message(
                LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
                "TCMD_agenda_delete_by_tssent: Telecommand with tssent=%s (%s) deleted from agenda.",
                tssent_str,
                TCMD_telecommand_definitions[TCMD_agenda[slot_num].tcmd_idx].tcmd_name
            );
            return 0;
        }
    }
    
    // If agenda is not found with timestamp return 1
    LOG_message(
        LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "TCMD_agenda_delete_by_tssent: Telecommand with tssent=%s not found in agenda.",
        tssent_str
    );
    return 1;
}

/// @brief Fetches the active agendas.
/// @return 0 on success, 1 if there are no active agendas.
uint8_t TCMD_agenda_fetch(){
    uint16_t active_agendas = 0;
    uint16_t logged_agendas = 0;
    
    // Count the number of active agendas
    for (uint16_t slot_num = 0; slot_num < TCMD_AGENDA_SIZE; slot_num++) {
        if (TCMD_agenda_is_valid[slot_num]) {
            active_agendas++;
            }
    }

    // if no active agendas, return 1
    if(active_agendas == 0){
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "TCMD_agenda_fetch: No entries in the agenda."
        );
        return 1;
    }

    // Output the number of active agendas
    LOG_message(
        LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "TCMD_agenda_fetch: Active agendas: %u",
        active_agendas
    );

    // List all active agendas in JSONL format
    for (uint16_t slot_num = 0; slot_num < TCMD_AGENDA_SIZE; slot_num++) {
        if (TCMD_agenda_is_valid[slot_num]) {

            // Convert uint64_t to a string
            char tssent_str[32];
            GEN_uint64_to_str(TCMD_agenda[slot_num].timestamp_sent, tssent_str);
            char tsexec_str[32];
            GEN_uint64_to_str(TCMD_agenda[slot_num].timestamp_to_execute, tsexec_str);

            LOG_message(
                LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL, 
                "{\"slot_num\":\"%u\",\"telecommand_channel\":\"%s\",\"timestamp_sent\":%s,\"timestamp_to_execute\":%s}\n",
                slot_num,
                telecommand_channel_enum_to_str(TCMD_agenda[slot_num].tcmd_channel),
                tssent_str,
                tsexec_str
            );
        }

        logged_agendas++;

        // Early-exit optimization: Break the loop once all active agendas have been logged
        if (logged_agendas >= active_agendas) {
            break;
        }
    }

    return 0;
}


/// @brief Deletes all agenda entries with a telecommand name.
/// @param telecommand_name The name of the telecommand in the agenda to delete. (e.g, hello_world)
/// @return 0 on success, > 0 on error.
/// @note Calls `LOG_message()` before all returns.
uint8_t TCMD_agenda_delete_by_name(const char *telecommand_name) {

    // Get count of active agendas
    const uint8_t active_agendas = TCMD_get_agenda_used_slots_count();

    if(active_agendas == 0){
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "TCMD_agenda_delete_by_telecommand_name: No active telecommands in the agenda."
        );
        return 1;
    }

    // Loop through the telecommand definitions and check if the passed function name is valid
    bool is_valid_telecommand_name = false;
    for (uint16_t idx = 0; idx < TCMD_NUM_TELECOMMANDS; idx++) {
        if (strcasecmp(TCMD_telecommand_definitions[idx].tcmd_name,telecommand_name) == 0) {
            is_valid_telecommand_name = true;
            break;
        }
    }

    if(!is_valid_telecommand_name){
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "TCMD_agenda_delete_by_telecommand_name: Invalid telecommand name passed in the function."
        );
        return 2;
    }

    // Loop through the agenda and check for valid agendas
    for (uint16_t slot_num = 0; slot_num < TCMD_AGENDA_SIZE; slot_num++) {
        if (TCMD_agenda_is_valid[slot_num] ) {

            // Grab the index of the telecommand in the `TCMD_telecommand_definitions` array
            const uint8_t telecommand_index = TCMD_agenda[slot_num].tcmd_idx;

            // Perform a string comparision
            if(strcasecmp(TCMD_telecommand_definitions[telecommand_index].tcmd_name, telecommand_name) == 0) {
                // Set agenda as invalid
                TCMD_agenda_is_valid[slot_num] = 0;
            }
        }
    }
    
    LOG_message(
        LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "TCMD_agenda_delete_by_telecommand_name: Removed %d telecommands with the name = (%s) from agenda.",
        active_agendas,
        telecommand_name
    );

    return 0;

}
 