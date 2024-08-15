
#ifndef INCLUDE_GUARD__TELECOMMAND_EXECUTOR_H
#define INCLUDE_GUARD__TELECOMMAND_EXECUTOR_H

#include <stdint.h>
#include "telecommands/telecommand_types.h"

// Max number of telecommands that can be stored in the agenda (schedule/queue).
// Memory-constrained.
#define TCMD_AGENDA_SIZE 1000

// NOTE: Not all functions in `telecommand_executor.c` are declared here. Only the ones that are
//      expected to be called from other modules are declared here. This is intentional.

uint8_t TCMD_add_tcmd_to_agenda(const TCMD_parsed_tcmd_to_execute_t *parsed_tcmd);

uint16_t TCMD_get_agenda_used_slots_count();

int16_t TCMD_get_next_tcmd_agenda_slot_to_execute();

uint8_t TCMD_execute_telecommand_in_agenda(const uint16_t tcmd_agenda_slot_num,
    char *response_output_buf, uint16_t response_output_buf_size
);

void TCMD_agenda_delete_all();

uint8_t TCMD_agenda_delete_by_tssent(uint64_t tssent);

uint8_t TCMD_agenda_fetch();

uint8_t TCMD_agenda_delete_by_name(const char *telecommand_name);

#endif // INCLUDE_GUARD__TELECOMMAND_EXECUTOR_H
