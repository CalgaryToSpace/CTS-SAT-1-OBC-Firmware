#include "command_history.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

char **CGSE_command_history = NULL;
size_t CGSE_command_history_length = 0;


/// ... 
/// Return values: 
///   -1 memory issue
int CGSE_store_command(char *cmd) 
{
    char **mem = realloc(CGSE_command_history, sizeof *mem * (CGSE_command_history_length + 1));
    if (mem == NULL)
    {
        return -1;
    }
    CGSE_command_history = mem;
    CGSE_command_history_length++;
    CGSE_command_history[CGSE_command_history_length-1] = strdup(cmd); 
    if (CGSE_command_history[CGSE_command_history_length-1] == NULL)
    {
        return -1;
    }

    return 0;
}

int CGSE_remove_command(size_t index)
{
    if (CGSE_command_history_length > 0 && index < CGSE_command_history_length)
    {
        free(CGSE_command_history[index]);
        if (index < CGSE_command_history_length -1)
        {
            memmove(&CGSE_command_history[index], &CGSE_command_history[index+1], sizeof *CGSE_command_history * (CGSE_command_history_length-(index+1)));
        }
        char **mem = realloc(CGSE_command_history, sizeof *CGSE_command_history * (CGSE_command_history_length - 1));
        if (mem == NULL)
        {
            return -1;
        }
        CGSE_command_history = mem;
        CGSE_command_history_length--;
    }

    return 0;
}

/// First command is accessed with index = 0
/// Returns NULL if index is equal to or larger than the number of stored commands.
char * CGSE_recall_command(size_t index)
{
    char *cmd = NULL;
    if (index < CGSE_command_history_length)
    {
        cmd = CGSE_command_history[index];
    }

    return cmd;
}

size_t CGSE_number_of_stored_commands(void)
{
    return CGSE_command_history_length;
}
