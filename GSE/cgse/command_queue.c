#include "command_queue.h"
#include "main.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

CGSE_command_queue_entry_t *CGSE_command_queue = NULL;
size_t CGSE_command_queue_length = 0;

bool CGSE_command_queue_command_available(void) 
{
    // Only checks the first queued command
    if (CGSE_command_queue_length > 0) {
        // Is it time to run this command?
        uint64_t execution_time = CGSE_command_queue[0].execution_time;
        struct timeval tv = {0};
        gettimeofday(&tv, NULL);
        uint64_t current_time = (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
        if (current_time >= execution_time) {
            return true;
        }
    }
    return false;
}

CGSE_command_queue_entry_t *CGSE_command_queue_next(void)
{
    if (!CGSE_command_queue_command_available()) {
        return NULL;
    }

    return &CGSE_command_queue[0];
}

void CGSE_command_queue_remove_next(void)
{
    if (CGSE_command_queue_length == 0) {
        return;
    }
    // Free command text
    free(CGSE_command_queue[0].command_text);

    if (CGSE_command_queue_length > 1) {
        memmove(CGSE_command_queue, &CGSE_command_queue[1], sizeof *CGSE_command_queue * (CGSE_command_queue_length - 1));
        CGSE_command_queue_entry_t *mem = realloc(CGSE_command_queue, sizeof *CGSE_command_queue * (CGSE_command_queue_length - 1));
        if (mem != NULL) {
            CGSE_command_queue = mem;
        }
    }
    else {
        free(CGSE_command_queue);
        CGSE_command_queue = NULL;
    }
    CGSE_command_queue_length--;

    return;
}

int CGSE_command_queue_add_command(CGSE_program_state_t *ps, char *cmd_text) {
    CGSE_command_queue_entry_t e = {0};

    // Each entry has the form
    // tnnnnnnnn <command-text>
    // where t is 'a' (time is ms since 1970 epoch) or 
    // 'r' (time is ms since program start time)
    // nnnnnnnn is the time value in ms
    // <command-text> is either a telecommand or a "."-leading terminal
    // command
    char *p = cmd_text;
    switch(*p) {
        // Absolute time since midnight on 1 Jan 1970 in ms
        case 'a':
            e.execution_time = atoll(p+1);
            break;
        // Time in milliseconds since the program start time
        case 'r':
            e.execution_time = ps->program_start_epoch_ms + atoll(p+1);
            break;
        default:
            return -1;
    }
    while (*p != '\0' && *p != ' ') {
        p++;
    }
    if (*p == ' ' && strlen(p) > 1) {
        char *cmd = p+1;
        // This memory is freed later
        e.command_text = strdup(cmd);
    }
    else {
        return -1;
    }

    CGSE_command_queue_entry_t *mem = realloc(CGSE_command_queue, sizeof *mem * (CGSE_command_queue_length + 1));
    if (mem == NULL) {
        return -1;
    }
    CGSE_command_queue = mem;
    CGSE_command_queue[CGSE_command_queue_length] = e;
    CGSE_command_queue_length++;

    return 0;
}

int CGSE_command_queue_read_commands(CGSE_program_state_t *ps)
{
    FILE *f = fopen(ps->command_queue_file_path, "r");
    if (f == NULL) {
        return -1;
    }

    char line_buffer[COMMAND_BUFFER_SIZE + 100];

    while (fgets(line_buffer, COMMAND_BUFFER_SIZE + 100, f) != NULL) {
        CGSE_command_queue_add_command(ps, line_buffer);
    }

    fclose(f);

    return 0;
}

void CGSE_free_command_queue(void)
{
    for (int i = 0; i < CGSE_command_queue_length; i++) {
        free(CGSE_command_queue[i].command_text);
    }
    free(CGSE_command_queue);
    CGSE_command_queue = NULL;

    return;
}

bool CGSE_command_queue_command_is_queued(double *seconds_until_execution)
{
    if (CGSE_command_queue_length == 0) {
        return false;
    }

    struct timeval tv = {0};
    gettimeofday(&tv, NULL);
    double now = tv.tv_sec  + (double)tv.tv_usec / 1.0e6;
    *seconds_until_execution = (double)CGSE_command_queue[0].execution_time / 1000.0 - now;

    return true;
}

void CGSE_command_queue_list_commands(CGSE_program_state_t *ps)
{
    if (CGSE_command_queue_length == 0) {
        return;
    }
    CGSE_command_queue_entry_t *e = NULL;
    wprintw(ps->command_window, "\nQueued commands:\n");
    for (int i = 0; i < CGSE_command_queue_length; i++) {
        e = &CGSE_command_queue[i];
        wprintw(ps->command_window, "%03d) @%llu: %s", i+1, e->execution_time, e->command_text);
    }

    return;
}
