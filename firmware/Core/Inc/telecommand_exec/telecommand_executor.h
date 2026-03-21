
#ifndef INCLUDE_GUARD__TELECOMMAND_EXECUTOR_H
#define INCLUDE_GUARD__TELECOMMAND_EXECUTOR_H

#include <stdint.h>
#include "telecommand_exec/telecommand_types.h"

// Max number of telecommands that can be stored in the agenda (schedule/queue).
// Memory-constrained.
#define TCMD_AGENDA_SIZE 750

/// Max number of tssent timestamp values that can be stored at a time (for unique telecommand tssent validation).
#define TCMD_TIMESTAMP_RECORD_SIZE 750

/// @brief  The agenda (schedule queue) of telecommands to execute.
extern TCMD_parsed_tcmd_to_execute_t TCMD_agenda[TCMD_AGENDA_SIZE];

/// @brief  A flag indicating whether a given index in `TCMD_agenda` is valid
///         (i.e., filled with a not-yet-executed command).
/// @note The values in here are tri-state via TCMD_agenda_entry_state_enum_t.
extern uint8_t TCMD_agenda_is_valid[TCMD_AGENDA_SIZE];

typedef enum {
    TCMD_AGENDA_ENTRY_INVALID = 0, // Initial state.
    TCMD_AGENDA_ENTRY_VALID_AND_PENDING = 1,
    TCMD_AGENDA_ENTRY_EXECUTING = 2,
} TCMD_agenda_entry_state_enum_t;


uint8_t TCMD_add_tcmd_to_agenda(const TCMD_parsed_tcmd_to_execute_t *parsed_tcmd);

uint16_t TCMD_get_agenda_used_slots_count();

int16_t TCMD_get_next_tcmd_agenda_slot_to_execute();

uint8_t TCMD_execute_telecommand_in_agenda(const uint16_t tcmd_agenda_slot_num,
    char *response_output_buf, uint16_t response_output_buf_size
);

uint8_t TCMD_log_pending_agenda_entries();


extern uint32_t TCMD_total_tcmd_queued_count;
extern uint64_t TCMD_latest_received_tcmd_timestamp_sent;

#endif // INCLUDE_GUARD__TELECOMMAND_EXECUTOR_H
