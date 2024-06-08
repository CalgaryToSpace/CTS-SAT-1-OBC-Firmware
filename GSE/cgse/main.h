#ifndef __INCLUDE_GUARD__MAIN_H_
#define __INCLUDE_GUARD__MAIN_H_

#include <termios.h>
#include <ncurses.h>

#define TCMD_BUFFER_SIZE 256
#define RECEIVE_BUFFER_SIZE 512
#define IO_WAIT_USEC 50000
#define COMMAND_BUFFER_SIZE 512


typedef struct 
{
    speed_t baud_rate;
    char *device_path;
    char *command_prefix;

    WINDOW *main_window;
    WINDOW *command_window;

    bool satellite_connected;
    int satellite_link;

} GSE_program_state_t;

int init_terminal_screen(GSE_program_state_t *program_state);
int connect_to_satellite(GSE_program_state_t *program_state);

#endif // __INCLUDE_GUARD__MAIN_H_
