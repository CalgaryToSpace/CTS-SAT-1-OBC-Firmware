#ifndef __INCLUDE_GUARD__MAIN_H_
#define __INCLUDE_GUARD__MAIN_H_

#include <termios.h>
#include <ncurses.h>
#include <stdio.h>

#define CGSE_DEFAULT_BAUD_RATE 115200
#define CGSE_DEFAULT_TELECOMMAND_PREFIX "CTS1"

#define TCMD_BUFFER_SIZE 256
#define RECEIVE_BUFFER_SIZE 512
#define IO_WAIT_USEC 50000
#define COMMAND_BUFFER_SIZE 512

#define CGSE_COMMAND_HISTORY_FILENAME ".cgse_command_history"
#define CGSE_COMMAND_HISTORY_BUFFER_SIZE COMMAND_BUFFER_SIZE + 100
#define CGSE_COMMAND_HISTORY_MAX_LENGTH 5

#define FIRMWARE_CHUNK_SIZE 128
#define MPI_FIRMWARE_PAGE_TIMEOUT 10.0

#define CGSE_TM_WINDOW_SIZE 23

#define CGSE_TIME_STR_MAX_LEN 32


typedef struct 
{

    int nOptions;
    int argc;
    char **argv;

    speed_t baud_rate;
    char satellite_link_path[FILENAME_MAX];
    char *command_prefix;
    bool auto_connect;

    WINDOW *status_window;
    WINDOW *main_window;
    WINDOW *command_window;

    bool satellite_connected;
    int satellite_link;

    char time_buffer[CGSE_TIME_STR_MAX_LEN];

    uint8_t receive_buffer[RECEIVE_BUFFER_SIZE];
    ssize_t bytes_received;

    char command_buffer[COMMAND_BUFFER_SIZE];
    size_t command_history_index;
    int cursor_position;
    int command_index;
    char command_history_file_path[FILENAME_MAX];

    char telecommand_buffer[TCMD_BUFFER_SIZE];

    char current_directory[FILENAME_MAX];
    char file_to_load[FILENAME_MAX];

    bool prepend_timestamp;


} CGSE_program_state_t;

int init_terminal_screen(CGSE_program_state_t *program_state);

int parse_input(CGSE_program_state_t *program_state, int key);

int parse_args(CGSE_program_state_t *ps);

void CGSE_license(void);
void CGSE_about(void);
void CGSE_commandline_help(char *name);

void CGSE_time_string(char *time_str);

void update_link_status(CGSE_program_state_t *ps);
void parse_telemetry(CGSE_program_state_t *ps);

int CGSE_init(CGSE_program_state_t *ps);


#endif // __INCLUDE_GUARD__MAIN_H_
