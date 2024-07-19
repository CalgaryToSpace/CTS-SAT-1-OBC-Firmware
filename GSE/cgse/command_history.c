#include "command_history.h"
#include "main.h"

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

CGSE_command_history_t *CGSE_command_history = NULL;
size_t CGSE_command_history_length = 0;


/// ... 
/// Return values: 
///   -1 memory issue
int CGSE_store_command(const char *cmd) 
{
    // history buffer in CGSE_COMMAND_HISTORY_MAX_LENGTH plus 1 to account for
    // the command line storage
    if (CGSE_command_history_length == CGSE_COMMAND_HISTORY_MAX_LENGTH + 1) {
        CGSE_command_history_length = CGSE_COMMAND_HISTORY_MAX_LENGTH;
        // Remove oldest command
        memmove(CGSE_command_history, &CGSE_command_history[1], (sizeof *CGSE_command_history) * CGSE_command_history_length); 
    }
    else {
        CGSE_command_history_t *mem = realloc(CGSE_command_history, sizeof *mem * (CGSE_command_history_length + 1));
        if (mem == NULL) {
            return -1;
        }
        CGSE_command_history = mem;
    }
    CGSE_command_history_length++;
    struct timeval tv = {0};
    gettimeofday(&tv, NULL);
    CGSE_command_history[CGSE_command_history_length - 1].time = tv.tv_sec;
    snprintf(CGSE_command_history[CGSE_command_history_length - 1].text, COMMAND_BUFFER_SIZE, "%s", cmd); 

    return 0;
}

int CGSE_remove_command(size_t index)
{
    if (CGSE_command_history_length > 0 && index < CGSE_command_history_length) {
        if (index < CGSE_command_history_length -1) {
            memmove(&CGSE_command_history[index], &CGSE_command_history[index + 1], sizeof *CGSE_command_history * (CGSE_command_history_length-(index + 1)));
        }
        CGSE_command_history_t *mem = realloc(CGSE_command_history, sizeof *CGSE_command_history * (CGSE_command_history_length - 1));
        if (mem == NULL) {
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
    if (index < CGSE_command_history_length) {
        cmd = CGSE_command_history[index].text;
    }

    return cmd;
}

size_t CGSE_number_of_stored_commands(void)
{
    return CGSE_command_history_length;
}

int CGSE_read_command_history(CGSE_program_state_t *ps)
{
    FILE *f = fopen(ps->command_history_file_path, "r");
    if (f == NULL) { return -1;
    }

    int lines = 0;
    // Cushion for time string at the start of each line
    char line_buf[CGSE_COMMAND_HISTORY_BUFFER_SIZE]; 

    while (fgets(line_buf, CGSE_COMMAND_HISTORY_BUFFER_SIZE, f) != NULL) {
        lines++;
    }

    int seek_res = fseek(f, 0, SEEK_SET);
    if (seek_res != 0) {
        fclose(f);
        return -1;
    }
    int recent_history_start = lines - CGSE_COMMAND_HISTORY_MAX_LENGTH - 1;
    if (recent_history_start < 0) {
        recent_history_start = 0;
    }

    lines = 0;
    int restored_lines = 0;

    while (fgets(line_buf, COMMAND_BUFFER_SIZE + 50, f) != NULL) {
        if (lines >= recent_history_start) {
            char *p = line_buf;
            size_t len = strlen(p);
            if (len > 1) {
                while(*p != '\0' && *p != ' ') {
                    p++;
                }
                if (*p == ' ') {
                    // Ignore \n
                    line_buf[strlen(line_buf)-1] = '\0';
                    CGSE_store_command(p + 1);
                    CGSE_command_history[restored_lines].time = atoll(line_buf);
                }
            }
            restored_lines++;
        }
        lines++;
    }

    // Set up command line for editing
    CGSE_store_command("");
    ps->command_history_index = CGSE_command_history_length - 1;

    fclose(f);

    return 0;
}

int CGSE_write_command_history(CGSE_program_state_t *ps)
{
    FILE *f = fopen(ps->command_history_file_path, "a");
    if (f == NULL) {
        return -1;
    }

   for (int i = 0; i < CGSE_command_history_length; i++) {
       if (strlen(CGSE_command_history[i].text) > 0) {
           fprintf(f, "%lu %s\n", CGSE_command_history[i].time, CGSE_command_history[i].text);
       }
       else {
           fprintf(f, "\n");
       }
   }

    fclose(f);

    return 0;
}

void CGSE_free_command_history(void) 
{
    free(CGSE_command_history);

    return;
}

