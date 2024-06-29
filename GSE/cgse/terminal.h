#ifndef __INCLUDE__GUARD_TERMINAL_H_
#define __INCLUDE__GUARD_TERMINAL_H_

#include "main.h"

int init_terminal_screen(CGSE_program_state_t *program_state);
int parse_input(CGSE_program_state_t *program_state);
void update_link_status(CGSE_program_state_t *ps);
void parse_telemetry(CGSE_program_state_t *ps);
void process_command_queue(CGSE_program_state_t *ps);

void reset_editing_cursor(CGSE_program_state_t *ps);
void update_editing_cursor(CGSE_program_state_t *ps);

void CGSE_terminal_shutdown(void);

#endif // __INCLUDE__GUARD_TERMINAL_H_
