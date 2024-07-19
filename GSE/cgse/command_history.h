#ifndef __INCLUDE_GUARD__COMMAND_HISTORY_H_
#define __INCLUDE_GUARD__COMMAND_HISTORY_H_

#include "main.h"

#include <stddef.h>
#include <time.h>

typedef struct {
    time_t time;
    char text[COMMAND_BUFFER_SIZE];
} CGSE_command_history_t;

int CGSE_store_command(const char *cmd);
int CGSE_remove_command(size_t index);
char * CGSE_recall_command(size_t index);
size_t CGSE_number_of_stored_commands(void);
int CGSE_read_command_history(CGSE_program_state_t *ps);
int CGSE_write_command_history(CGSE_program_state_t *ps);
void CGSE_free_command_history(void);

#endif // __INCLUDE_GUARD__COMMAND_HISTORY_H_
