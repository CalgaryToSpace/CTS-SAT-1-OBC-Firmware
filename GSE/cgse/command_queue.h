#ifndef __INCLUDE__GUARD__COMMAND_QUEUE_H_
#define __INCLUDE__GUARD__COMMAND_QUEUE_H_

#include "main.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    char *label;
    uint64_t execution_time;
    bool execution_time_is_delta_ms;
    char *command_text;
    
} CGSE_command_queue_entry_t;


bool CGSE_command_queue_command_available(void);
CGSE_command_queue_entry_t *CGSE_command_queue_next(void);
void CGSE_command_queue_remove_next(void);
int CGSE_command_queue_add_command(CGSE_program_state_t *ps, char *cmd_text);
int CGSE_command_queue_read_commands(CGSE_program_state_t *ps);
void CGSE_free_command_queue(void);
bool CGSE_command_queue_command_is_queued(double *seconds_until_execution);
void CGSE_command_queue_list_commands(CGSE_program_state_t *ps);

#endif // __INCLUDE__GUARD__COMMAND_QUEUE_H_
