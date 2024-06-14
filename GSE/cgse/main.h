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

#define FIRMWARE_CHUNK_SIZE 192
#define MPI_FIRMWARE_PAGE_TIMEOUT 2.0

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

    char telecommand_buffer[TCMD_BUFFER_SIZE];

    char current_directory[FILENAME_MAX];
    char file_to_load[FILENAME_MAX];

} CGSE_program_state_t;

int init_terminal_screen(CGSE_program_state_t *program_state);
int CGSE_connect(CGSE_program_state_t *program_state);

int parse_input(CGSE_program_state_t *program_state, int key);

int parse_args(CGSE_program_state_t *ps);

void CGSE_license(void);
void CGSE_about(void);
void CGSE_help(char *name);
int find_link_path(char *link_path);

void CGSE_time_string(char *time_str);

void CGSE_list_telecommands(CGSE_program_state_t *ps);
void update_link_status(CGSE_program_state_t *ps);
void parse_telemetry(CGSE_program_state_t *ps);

void CGSE_disconnect(CGSE_program_state_t *ps);
int CGSE_init(CGSE_program_state_t *ps);

char * CGSE_parse_command_args(CGSE_program_state_t *ps, int *nargs, char **arg_vector);

int CGSE_ls_dir(CGSE_program_state_t *ps);

char * CGSE_base64_encode_from_file(CGSE_program_state_t *ps, char *file_name, size_t *file_size);
char * CGSE_base64_encode_bytes(CGSE_program_state_t *ps, uint8_t *byte_array, int len);
char CGSE_base64_encode_character(uint8_t bits);

#endif // __INCLUDE_GUARD__MAIN_H_
