#ifndef AGENDA_H
#define AGENDA_H

#include "command_parse.h"

#define MAX_AGENDA_LEN 64

extern command_t command_agenda[MAX_AGENDA_LEN];

uint64_t epoch_time_ms();
int add_cmd_to_agenda(command_t agenda[], command_t *cmd);
void remove_cmd_from_bitmap(int index, uint64_t *bitmap);
int select_cmds_for_exec(uint64_t *cmd_mask, const command_t agenda[], uint64_t now);

#endif  // AGENDA_H