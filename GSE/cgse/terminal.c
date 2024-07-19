#include "terminal.h"
#include "commands.h"
#include "command_history.h"
#include "command_queue.h"
#include "main.h"

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <ncurses.h>
#include <stdarg.h>

int init_terminal_screen(CGSE_program_state_t *ps)
{
    int status = 0;
    setlocale(LC_ALL, "");
    initscr();
    status |= noecho();
    status |= raw();
    status |= intrflush(NULL, false);

    ps->status_window_height = 1;
    ps->main_window_height = CGSE_TM_WINDOW_DEFAULT_SIZE;
    ps->command_window_height = LINES - ps->status_window_height - ps->main_window_height;
    // TODO check for 0 or negative heights

    ps->status_window = newwin(ps->status_window_height, 0, 0, 0);
    if (ps->status_window == NULL) {
        status |= 1;
    }
    status |= wattron(ps->status_window, A_REVERSE);

    ps->main_window = newwin(ps->main_window_height, 0, 1, 0);
    if (ps->main_window == NULL) {
        status |= 1;
    }
    status |= keypad(ps->main_window, TRUE);
    status |= nodelay(ps->main_window, TRUE);
    status |= scrollok(ps->main_window, TRUE);
    status |= idlok(ps->main_window, TRUE);

    ps->command_window = newwin(ps->command_window_height, 0, ps->status_window_height + ps->main_window_height, 0);
    if (ps->command_window == NULL) {
        status |= 1;
    }
    status |= keypad(ps->command_window, TRUE);
    status |= nodelay(ps->command_window, TRUE);
    status |= scrollok(ps->command_window, TRUE);
    status |= idlok(ps->command_window, TRUE);

    return status;

}

int parse_input(CGSE_program_state_t *ps)
{
    int status = 0;
    ssize_t bytes_sent = 0;
    size_t buffer_len = 0;

    double t1 = current_time();
    double t2 = t1;

    int key = 0;

    while ((key = wgetch(ps->command_window)) != ERR && (t2 - t1) < 0.5) {
        buffer_len = strlen(ps->command_buffer);
        // Erase character to the left
        if (key == '\b' || key == 127 || key == KEY_BACKSPACE)
        {
            commandline_action(ps, CGSE_CMD_ACTION_BACKSPACE, key);
        }
        else if (key == KEY_UP) {
            commandline_action(ps, CGSE_CMD_ACTION_CURSOR_UP, key);
        }
        else if (key == KEY_DOWN) {
            commandline_action(ps, CGSE_CMD_ACTION_CURSOR_DOWN, key);
        }
        else if (key == KEY_LEFT) {
            commandline_action(ps, CGSE_CMD_ACTION_CURSOR_LEFT, key);
        }
        else if (key == KEY_RIGHT) {
            commandline_action(ps, CGSE_CMD_ACTION_CURSOR_RIGHT, key);
        }
        else if (key != '\n' && ps->cursor_position < COMMAND_BUFFER_SIZE - 1) {
            commandline_action(ps, CGSE_CMD_ACTION_INSERT, key);
        }
        else if (key == '\n') {
            // Print the command to the command window
            command_window_print(ps, "%s> %s", ps->command_prefix, ps->command_buffer);
            CGSE_execute_command(ps, ps->command_buffer);
            // Reset the editing buffer
            ps->cursor_position = 0;
            ps->command_buffer[0] = '\0';
        }

        // Update command window text and cursor position
        commandline_redraw(ps);


        // Update timer
        t2 = current_time();
    }

    wrefresh(ps->command_window);
    return 0;
}

void update_link_status(CGSE_program_state_t *ps)
{
    if (ps->satellite_connected) {
        mvwprintw(ps->status_window, 0, 0, "Connected on %s @ %lu", ps->satellite_link_path, ps->baud_rate);
    }
    else {
        mvwprintw(ps->status_window, 0, 0, "NOT connected on %s", ps->satellite_link_path);
    }
    double time_to_next_command = 0.0;
    bool command_is_queued = CGSE_command_queue_command_is_queued(&time_to_next_command);
    if (command_is_queued) {
        wprintw(ps->status_window, ", sending next command in %.1f s", time_to_next_command);
    }

    wclrtoeol(ps->status_window);
    wrefresh(ps->status_window);

    return;
}

void parse_telemetry(CGSE_program_state_t *ps)
{
    // select() to see if data are ready?
    memset(ps->receive_buffer, 0, RECEIVE_BUFFER_SIZE);
    size_t stop = 0;
    static bool last_line_complete = true;
    static int8_t prepend_timestamp = -1;

    ps->bytes_received = 0;
    if (ps->satellite_connected) {
        CGSE_time_string(ps->time_buffer);
        struct timeval tv = {0};
        gettimeofday(&tv, NULL);
        double t1 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
        double t2 = t1;

        while (ps->bytes_received >= 0 && (t2-t1) < 0.25) {
            ps->bytes_received = read(ps->satellite_link, ps->receive_buffer + stop, RECEIVE_BUFFER_SIZE);
            if (ps->bytes_received > 0) {
                stop += ps->bytes_received;
            }
            gettimeofday(&tv, NULL);
            t2 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
        }
    }
    else {
       stop = 0; 
    }
    if (stop > 0) {
        wclrtoeol(ps->main_window);
        if (stop >= RECEIVE_BUFFER_SIZE) {
            stop--;
        }
        ps->receive_buffer[stop] = '\0';
        // TODO convert binary to base64 to allow printing to the screen
        // TODO log received message and timestamp to a file
        if (ps->prepend_timestamp) {
            if (prepend_timestamp != ps->prepend_timestamp) {
                prepend_timestamp = ps->prepend_timestamp;
            }
            char *string = strdup((char*)ps->receive_buffer);
            if (string == NULL) {
                goto done_parsing_telemetry;
            }
            char *bufline = NULL;
            uint32_t lines_treated = 0;
            
            while ((bufline = strsep(&string, "\n")) != NULL) {
                if (strlen(bufline) > 0) {
                    if (lines_treated == 0 && !last_line_complete) {
                        wprintw(ps->main_window, "%s", bufline);
                    }
                    else {
                        wprintw(ps->main_window, "\n%s: %s", ps->time_buffer, bufline);
                    }
                }
                lines_treated++;
            }
            free(string);
            last_line_complete = ps->receive_buffer[strlen((char*)ps->receive_buffer)-1] == '\n';
        }
        else {
            if (prepend_timestamp != ps->prepend_timestamp) {
                wprintw(ps->main_window, "\n");
                prepend_timestamp = ps->prepend_timestamp;
            }
            wprintw(ps->main_window, "%s", ps->receive_buffer);
        }
    }

done_parsing_telemetry:
    wrefresh(ps->main_window);
    wrefresh(ps->command_window);
    return;
}

// Moves cursor to end of editing line
void move_cursor(CGSE_program_state_t *ps, CGSE_cursor_edit_position_enum_t position)
{
    size_t buffer_len = strlen(ps->command_buffer);

    switch (position) {
        case CGSE_CURSOR_START_OF_LINE:
            ps->cursor_position = 0;
            break;
        case CGSE_CURSOR_END_OF_LINE:
            ps->cursor_position = buffer_len;
            break;
        default:
            break;
    }

    ps->col = strlen(ps->command_prefix) + 2 + ps->cursor_position;

    return;
}

void store_editing_cursor(CGSE_program_state_t *ps)
{
    memcpy(ps->editing_buffer, ps->command_buffer, COMMAND_BUFFER_SIZE);
    ps->editing_cursor_position = ps->cursor_position;

    return;
}

void restore_editing_cursor(CGSE_program_state_t *ps)
{
    memcpy(ps->command_buffer, ps->editing_buffer, COMMAND_BUFFER_SIZE);
    ps->cursor_position = ps->editing_cursor_position;

    return;
}

void print_command_line(CGSE_program_state_t *ps) 
{
    wmove(ps->command_window, ps->line, 0);
    wprintw(ps->command_window, "%s> %s", ps->command_prefix, ps->command_buffer);
    wmove(ps->command_window, ps->line, strlen(ps->command_prefix) + 2 + ps->cursor_position);
    wclrtoeol(ps->command_window);
    wmove(ps->command_window, ps->line, ps->col);

    return;
}

void CGSE_terminal_shutdown(void)
{
    clear();
    refresh();
    endwin();

    return;
}

void commandline_action(CGSE_program_state_t *ps, CGSE_commandline_action_enum_t action, int key)
{
    if (ps == NULL) {
        return;
    }

    size_t buffer_len = strlen(ps->command_buffer);
    switch (action) {
        case CGSE_CMD_ACTION_CURSOR_UP:
            if (ps->command_history_index > 0) {
                char *previous_command = CGSE_recall_command((size_t)ps->command_history_index - 1);
                if (previous_command != NULL) {
                    snprintf(ps->command_buffer, COMMAND_BUFFER_SIZE, "%s", previous_command);
                    ps->command_history_index--;
                    move_cursor(ps, CGSE_CURSOR_END_OF_LINE);
                }
            }
            break;

        case CGSE_CMD_ACTION_CURSOR_DOWN:
            if (ps->command_history_index < CGSE_number_of_stored_commands()) {
                char *next_command = CGSE_recall_command((size_t)(ps->command_history_index+1));
                if (next_command != NULL) {
                    snprintf(ps->command_buffer, COMMAND_BUFFER_SIZE, "%s", next_command);
                    ps->command_history_index++;
                    move_cursor(ps, CGSE_CURSOR_END_OF_LINE);
                }
            }
            break;
        
        case CGSE_CMD_ACTION_CURSOR_LEFT:
            if (ps->cursor_position > 0) {
                ps->cursor_position--;
            }
            break;

        case CGSE_CMD_ACTION_CURSOR_RIGHT:
            if (ps->cursor_position < buffer_len && ps->cursor_position < COMMAND_BUFFER_SIZE - 1) {
                ps->cursor_position++;
            }
            break;

        case CGSE_CMD_ACTION_INSERT:
            if (ps->cursor_position < COMMAND_BUFFER_SIZE - 1) {
                // Move characters to the right 
                if (ps->cursor_position < buffer_len && buffer_len < COMMAND_BUFFER_SIZE - 1) {
                    for (int k = buffer_len - 1; k >= ps->cursor_position; k--) {
                        ps->command_buffer[k+1] = ps->command_buffer[k];
                    }
                }
                ps->command_buffer[ps->cursor_position] = key;
                ps->cursor_position++;
                ps->col++;
                buffer_len++;
                ps->command_buffer[buffer_len] = '\0';
                ps->command_history_index = CGSE_number_of_stored_commands() - 1;
                CGSE_remove_command(ps->command_history_index);
                CGSE_store_command(ps->command_buffer);
            }
            break;

        case CGSE_CMD_ACTION_BACKSPACE:
            if (ps->cursor_position > 0) {
                ps->cursor_position--;
                ps->col--;
                for (int c = ps->cursor_position; c < buffer_len && c < COMMAND_BUFFER_SIZE - 2; c++) {
                    ps->command_buffer[c] = ps->command_buffer[c+1];
                }
                buffer_len--;
                ps->command_buffer[buffer_len] = '\0';
                ps->command_history_index = CGSE_number_of_stored_commands() - 1;
                CGSE_remove_command(ps->command_history_index);
                CGSE_store_command(ps->command_buffer);
            }
            break;

        default:
            break;
    }

    return;
}

void commandline_redraw(CGSE_program_state_t *ps)
{
    if (ps == NULL) {
        return;
    }

    // Scroll window up?
    if (ps->line > ps->command_window_height - 1) {
        wscrl(ps->command_window, 1);
        ps->line = ps->command_window_height - 1;
    }

    size_t buffer_len = strlen(ps->command_buffer);
    ps->col = ps->cursor_position + strlen(ps->command_prefix) + 2;

    // Redraw the command line
    wmove(ps->command_window, ps->line, 0);
    wprintw(ps->command_window, "%s> %s", ps->command_prefix, ps->command_buffer);
    wclrtoeol(ps->command_window);
    wmove(ps->command_window, ps->line, ps->col);

    return;
}


void command_window_print(CGSE_program_state_t *ps, const char *fmt, ...)
{
    wmove(ps->command_window, ps->line, 0);
    va_list ap;
    va_start(ap, fmt);
    vw_printw(ps->command_window, fmt, ap);
    va_end(ap);
    wclrtoeol(ps->command_window);
    ps->line++;

    return;
}

double current_time(void) 
{
    struct timeval tv = {0};
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
}

