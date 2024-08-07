
#include "telecommands/telecommand_parser.h"
#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_executor.h"
#include "telecommands/telecommand_types.h"
#include "debug_tools/debug_uart.h"
#include "timekeeping/timekeeping.h"

#include <stdint.h>

/// @brief  The agenda (schedule queue) of telecommands to execute.
TCMD_parsed_tcmd_to_execute_t TCMD_agenda[TCMD_AGENDA_SIZE];
// TODO: consider an optimization to store the args_str_no_parens in a separate buffer, to save a ton of memory.

/// @brief  A flag indicating whether a given index in `TCMD_agenda` is valid
///         (i.e., filled with a not-yet-executed command).
uint8_t TCMD_agenda_is_valid[TCMD_AGENDA_SIZE] = {0};


/// @brief Adds a telecommand to the agenda (schedule/queue) of telecommands to execute.
/// @param parsed_tcmd The parsed telecommand to add to the agenda.
/// @return 0 on success, 1 if the agenda is full.
/// @note Performs a deep copy of the `parsed_tcmd` arg into the agenda.
uint8_t TCMD_add_tcmd_to_agenda(const TCMD_parsed_tcmd_to_execute_t *parsed_tcmd) {
    // FIXME: Issue #53 - Check `parsed_tcmd.timestamp_sent` against the yet-to-be-created `TCMD_timestamp_sent_record[]`, and skip this command if it is (to prevent replays).
    // ^ Use a config variable to override this check, if desired.

    // Find the first empty slot in the agenda.
    for (uint16_t slot_num = 0; slot_num < TCMD_AGENDA_SIZE; slot_num++) {
        // Skip filled slots.
        if (TCMD_agenda_is_valid[slot_num]) {
            continue;
        }

        // Copy the parsed telecommand into the agenda.
        TCMD_agenda[slot_num].tcmd_idx = parsed_tcmd->tcmd_idx;
        TCMD_agenda[slot_num].tcmd_channel = parsed_tcmd->tcmd_channel;
        TCMD_agenda[slot_num].timestamp_sent = parsed_tcmd->timestamp_sent;
        TCMD_agenda[slot_num].timestamp_to_execute = parsed_tcmd->timestamp_to_execute;

        for (uint16_t j = 0; j < TCMD_ARGS_STR_NO_PARENS_SIZE; j++) {
            TCMD_agenda[slot_num].args_str_no_parens[j] = parsed_tcmd->args_str_no_parens[j];
        }

        // Mark the slot as valid.
        TCMD_agenda_is_valid[slot_num] = 1;

        // DEBUG_uart_print_str("Telecommand added to agenda at slot ");
        // DEBUG_uart_print_uint32(slot_num);
        // DEBUG_uart_print_str("\n");
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

    DEBUG_uart_print_str("=========================");
    DEBUG_uart_print_str(" Executing telecommand '");
    DEBUG_uart_print_str(tcmd_def.tcmd_name);
    DEBUG_uart_print_str("'=========================\n");
    
    

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
    DEBUG_uart_print_str("=========================");
    DEBUG_uart_print_str(" Response (duration=");
    DEBUG_uart_print_int32(tcmd_exec_duration_ms);
    DEBUG_uart_print_str("ms, err=");
    DEBUG_uart_print_uint32(tcmd_result);
    if (tcmd_result != 0) {
        DEBUG_uart_print_str(" !!!!!! ERROR !!!!!!");
    }
    DEBUG_uart_print_str(") =========================\n");
    DEBUG_uart_print_str(response_output_buf);
    DEBUG_uart_print_str("\n===========================================================================\n");

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
        DEBUG_uart_print_str("Error: TCMD_execute_telecommand_in_agenda: slot is invalid.\n");
        return 253;
    }

    // Mark the slot as invalid because it's executed.
    // Do it now, in case the execution does Undefined Behaviour (but could also do it after the call just fine).
    TCMD_agenda_is_valid[tcmd_agenda_slot_num] = 0;

    // Execute the telecommand.
    return TCMD_execute_parsed_telecommand_now(
        TCMD_agenda[tcmd_agenda_slot_num].tcmd_idx,
        TCMD_agenda[tcmd_agenda_slot_num].args_str_no_parens,
        TCMD_agenda[tcmd_agenda_slot_num].tcmd_channel,
        response_output_buf,
        response_output_buf_size
    );
}
