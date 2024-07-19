#ifndef __INCLUDE__GUARD_TERMINAL_H_
#define __INCLUDE__GUARD_TERMINAL_H_

#include "main.h"

int init_terminal_screen(CGSE_program_state_t *program_state);
int parse_input(CGSE_program_state_t *program_state);
void update_link_status(CGSE_program_state_t *ps);
void parse_telemetry(CGSE_program_state_t *ps);

typedef enum {
    CGSE_CURSOR_START_OF_LINE,
    CGSE_CURSOR_END_OF_LINE,
} CGSE_cursor_edit_position_enum_t;

void move_cursor(CGSE_program_state_t *ps, CGSE_cursor_edit_position_enum_t position);
void update_editing_cursor(CGSE_program_state_t *ps);
void store_editing_cursor(CGSE_program_state_t *ps);
void restore_editing_cursor(CGSE_program_state_t *ps);

void print_command_line(CGSE_program_state_t *ps);

void CGSE_terminal_shutdown(void);

typedef enum {
    CGSE_CMD_ACTION_BACKSPACE,
    CGSE_CMD_ACTION_CURSOR_LEFT,
    CGSE_CMD_ACTION_CURSOR_RIGHT,
    CGSE_CMD_ACTION_CURSOR_UP,
    CGSE_CMD_ACTION_CURSOR_DOWN,
    CGSE_CMD_ACTION_INSERT,
    CGSE_CMD_ACTION_ENTER,
} CGSE_commandline_action_enum_t;

void commandline_action(CGSE_program_state_t *ps, CGSE_commandline_action_enum_t action, int key);
void commandline_redraw(CGSE_program_state_t *ps);
void command_window_print(CGSE_program_state_t *ps, const char *fmt, ...);

double current_time(void);

#endif // __INCLUDE__GUARD_TERMINAL_H_
