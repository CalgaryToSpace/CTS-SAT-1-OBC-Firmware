// Header-only self-rescheduling for repeating blobs (run via `exec_blob_from_fs`).
//
// Rule: after each run, a blob re-adds a copy of its own agenda entry to run again in
// `interval_ms`, UNLESS another pending `exec_blob_from_fs` of the same blob file is due within the
// next `2 * interval_ms`. In that case, that upcoming entry (e.g., a pre-scheduled reconfiguration
// with different args) takes over, and this chain ends. Pending entries further out are left alone,
// so several reconfigurations can be scheduled ahead of time.
//
// Usage:
//     #include "../blob_libs/agenda_self_reschedule.h"
//     const int8_t r = BLOB_reschedule_self_unless_upcoming(interval_ms);
//     // r: BLOB_RESCHEDULE_DONE, BLOB_RESCHEDULE_SKIPPED_UPCOMING, or a negative BLOB_RESCHEDULE_ERR_*.
//
// Notes:
// 1. Everything is `static`, so each blob gets its own copy, and unused functions are dropped.
// 2. Firmware symbols (TCMD_agenda, TIME_*, etc.) resolve at link time (--just-symbols).

#ifndef BLOB_LIBS_AGENDA_SELF_RESCHEDULE_H
#define BLOB_LIBS_AGENDA_SELF_RESCHEDULE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "timekeeping/timekeeping.h"
#include "telecommand_exec/telecommand_executor.h"
#include "telecommand_exec/telecommand_args_helpers.h"

extern int strcmp(const char *s1, const char *s2);
extern void *memcpy(void *__restrict dest, const void *__restrict src, size_t n);

enum {
    BLOB_RESCHEDULE_DONE = 0,
    BLOB_RESCHEDULE_SKIPPED_UPCOMING = 1, // Another run of this blob is due within 2*interval.
    BLOB_RESCHEDULE_ERR_NO_EXECUTING_SLOT = -1, // Couldn't find our own agenda slot.
    BLOB_RESCHEDULE_ERR_BAD_OWN_ARGS = -2, // Couldn't parse our own blob file name.
    BLOB_RESCHEDULE_ERR_AGENDA_ADD_FAILED = -3, // TCMD_add_tcmd_to_agenda() failed.
};

static int16_t BLOB_get_executing_agenda_slot_num(void) {
    for (uint16_t i = 0; i < TCMD_AGENDA_SIZE; i++) {
        if (TCMD_agenda_is_valid[i] == TCMD_AGENDA_ENTRY_EXECUTING) {
            return (int16_t)i;
        }
    }
    return -1;
}

/// @brief Is `slot_num` a pending `exec_blob_from_fs` entry (other than `current_slot_num`) that
///     runs the blob file named `own_blob_file_name`?
static bool BLOB_is_pending_run_of_same_blob(
    uint16_t slot_num, int16_t current_slot_num, const char *own_blob_file_name
) {
    if ((int16_t)slot_num == current_slot_num) {
        return false;
    }
    if (TCMD_agenda_is_valid[slot_num] != TCMD_AGENDA_ENTRY_VALID_AND_PENDING) {
        return false;
    }
    if (TCMD_agenda[slot_num].tcmd_idx != TCMD_agenda[current_slot_num].tcmd_idx) {
        return false;
    }

    char other_blob_file_name[TCMD_ARGS_STR_NO_PARENS_SIZE];
    if (TCMD_extract_string_arg(
        TCMD_agenda[slot_num].args_str_no_parens, 0,
        other_blob_file_name, sizeof(other_blob_file_name)
    ) != 0) {
        return false; // Can't tell what it runs; don't treat it as ours.
    }
    return strcmp(own_blob_file_name, other_blob_file_name) == 0;
}

/// @brief Extract the blob file name (arg 0) of the currently-executing `exec_blob_from_fs`.
/// @return Current slot number, or a negative BLOB_RESCHEDULE_ERR_* on failure.
static int16_t BLOB_get_own_blob_file_name(char *dest, uint16_t dest_len) {
    const int16_t current_slot_num = BLOB_get_executing_agenda_slot_num();
    if (current_slot_num < 0) {
        return BLOB_RESCHEDULE_ERR_NO_EXECUTING_SLOT;
    }
    if (TCMD_extract_string_arg(
        TCMD_agenda[current_slot_num].args_str_no_parens, 0, dest, dest_len
    ) != 0) {
        return BLOB_RESCHEDULE_ERR_BAD_OWN_ARGS;
    }
    return current_slot_num;
}

/// @brief Enqueue a copy of the currently executing tcmd to run again in `interval_ms`, unless
///     another pending run of this same blob file is due within the next `2 * interval_ms`.
/// @note Avoids incrementing the "total telecommands" counter, so that counter keeps measuring
///     how many uplinked commands were successful.
/// @return BLOB_RESCHEDULE_DONE, BLOB_RESCHEDULE_SKIPPED_UPCOMING, or a negative
///     BLOB_RESCHEDULE_ERR_* on failure.
static int8_t BLOB_reschedule_self_unless_upcoming(uint32_t interval_ms) {
    char own_blob_file_name[TCMD_ARGS_STR_NO_PARENS_SIZE];
    const int16_t current_slot_num = BLOB_get_own_blob_file_name(
        own_blob_file_name, sizeof(own_blob_file_name)
    );
    if (current_slot_num < 0) {
        return (int8_t)current_slot_num;
    }

    const uint64_t now_ms = TIME_get_current_unix_epoch_time_ms();
    const uint64_t window_end_ms = now_ms + (2 * (uint64_t)interval_ms);
    for (uint16_t slot_num = 0; slot_num < TCMD_AGENDA_SIZE; slot_num++) {
        if (
            BLOB_is_pending_run_of_same_blob(slot_num, current_slot_num, own_blob_file_name)
            && (TCMD_agenda[slot_num].timestamp_to_execute <= window_end_ms)
        ) {
            return BLOB_RESCHEDULE_SKIPPED_UPCOMING;
        }
    }

    TCMD_parsed_tcmd_to_execute_t new_tcmd;
    memcpy(&new_tcmd, &TCMD_agenda[current_slot_num], sizeof(TCMD_parsed_tcmd_to_execute_t));
    new_tcmd.timestamp_sent = now_ms;
    new_tcmd.timestamp_to_execute = now_ms + interval_ms;

    if (TCMD_add_tcmd_to_agenda(&new_tcmd) != 0) {
        return BLOB_RESCHEDULE_ERR_AGENDA_ADD_FAILED;
    }

    // Undo the counter increase in `TCMD_add_tcmd_to_agenda()`.
    TCMD_total_tcmd_queued_count--;

    return BLOB_RESCHEDULE_DONE;
}


#endif // BLOB_LIBS_AGENDA_SELF_RESCHEDULE_H
