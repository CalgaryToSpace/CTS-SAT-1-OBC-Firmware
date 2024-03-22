#include <stdlib.h>
#include <string.h>
#if __linux__ == 1
#include <stdio.h>
#include <sys/time.h>
#endif
#include "agenda.h"

command_t command_agenda[MAX_AGENDA_LEN];
uint64_t agenda_bitmap = 0x0000000000000000;

/**
 * Get the current time since epoch in milliseconds
*/
uint64_t epoch_time_ms()
{
#if __linux__ == 1
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long long millisecondsSinceEpoch =
        (unsigned long long)(tv.tv_sec) * 1000 +
        (unsigned long long)(tv.tv_usec) / 1000;
    return (uint64_t) millisecondsSinceEpoch;
#else
    return (uint64_t) 1709982595644ull;
#endif
}

/**
 * Add a command to the agenda
*/
int add_cmd_to_agenda(command_t agenda[], command_t *cmd)
{
    if (!cmd) return E_NULL;
    if (~agenda_bitmap == 0) return E_OOM;  // No spots left
    int index = __builtin_ffsll(~agenda_bitmap)-1;  // Get index of next available spot
    agenda_bitmap |= 1 << index;  // Set the bit for that slot
    memcpy(&agenda[index], cmd, sizeof(command_t));  // Copy command to agenda
    // printf("After add:\t0x%016llX\n", (unsigned long long)agenda_bitmap);
    return 0;
}

/**
 * Remove a command from the agenda (mark the slot as free)
*/
void remove_cmd_from_bitmap(int index, uint64_t *bitmap)
{
    *bitmap &= ~(1 << index);  // Clear the bit
    // printf("After removal:\t0x%016llX\n", (unsigned long long)*bitmap);
}

/**
 * Set cmd_mask indicating which commands are due to be executed
*/
int select_cmds_for_exec(uint64_t *cmd_mask, const command_t agenda[], uint64_t now)
{
    *cmd_mask = 0;
    uint64_t temp_map = agenda_bitmap;
    while (temp_map != 0) {
        int index = __builtin_ffsll(temp_map)-1;  // Find occupied slot
        remove_cmd_from_bitmap(index, &temp_map);  // Mark as visited
        if (agenda[index].timestamp <= now) {  // If due to execute
            *cmd_mask |= 1 << index;  // Mark command as selected in mask
        }
    }
    return 0;
}
