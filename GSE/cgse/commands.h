#ifndef __INCLUDE__GUARD__COMMAND_H_
#define __INCLUDE__GUARD__COMMAND_H_

#include "main.h"

#include <stdbool.h>

typedef struct {
    char *name;
    char *help_template;
    char *description;
    int (*function)(CGSE_program_state_t *ps, char *cmd_string);
    bool is_telecommand;

} CGSE_command_t;

int CGSE_quit(CGSE_program_state_t *ps, char *cmd_string);
int CGSE_help(CGSE_program_state_t *ps, char *cmd_string);
int CGSE_connect(CGSE_program_state_t *ps, char *cmd_string);
int CGSE_disconnect(CGSE_program_state_t *ps, char *cmd_string);
int CGSE_show_timestamp(CGSE_program_state_t *ps, char *cmd_string);
int CGSE_hide_timestamp(CGSE_program_state_t *ps, char *cmd_string);
int CGSE_sync_timestamp(CGSE_program_state_t *ps, char *cmd_string);
int CGSE_list_telecommands(CGSE_program_state_t *ps, char *cmd_string);
int CGSE_list_current_directory(CGSE_program_state_t *ps, char *cmd_string);
int CGSE_list_queued_commands(CGSE_program_state_t *ps, char *cmd_string);
int CGSE_upload_mpi_firmware(CGSE_program_state_t *ps, char *cmd_string);

char * CGSE_get_args_from_str(char* args, int *nargs, char **arg_vector);
int CGSE_ls_dir(CGSE_program_state_t *ps);
int CGSE_find_link_path(char *link_path);

int CGSE_execute_command(CGSE_program_state_t *ps);

#endif // __INCLUDE__GUARD__COMMAND_H_
