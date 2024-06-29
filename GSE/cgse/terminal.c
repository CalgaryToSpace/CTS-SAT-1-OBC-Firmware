#include "terminal.h"
#include "commands.h"
#include "command_history.h"
#include "command_queue.h"

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <ncurses.h>

int init_terminal_screen(CGSE_program_state_t *ps)
{
    int status = 0;
    setlocale(LC_ALL, "");
    initscr();
    status |= noecho();
    status |= raw();
    status |= intrflush(NULL, false);

    ps->status_window = newwin(1, 0, 0, 0);
    if (ps->status_window == NULL) {
        status |= 1;
    }
    status |= wattron(ps->status_window, A_REVERSE);

    ps->main_window = newwin(CGSE_TM_WINDOW_SIZE, 0, 1, 0);
    if (ps->main_window == NULL) {
        status |= 1;
    }
    status |= keypad(ps->main_window, TRUE);
    status |= nodelay(ps->main_window, TRUE);
    status |= scrollok(ps->main_window, TRUE);
    status |= idlok(ps->main_window, TRUE);

    ps->command_window = newwin(0, 0, CGSE_TM_WINDOW_SIZE + 1, 0);
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

    struct timeval tv = {0};
    gettimeofday(&tv, NULL);
    double t1 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
    double t2 = t1;

    int key = 0;

    while ((key = wgetch(ps->command_window)) != ERR && (t2 - t1) < 0.5) {

        getyx(ps->command_window, ps->line, ps->col);
        buffer_len = strlen(ps->command_buffer);
        if (key == '\b' || key == 127 || key == KEY_BACKSPACE)
        {
            if (ps->command_history_index < CGSE_number_of_stored_commands() - 1) {
                CGSE_store_command(ps->command_buffer);
                ps->command_history_index = CGSE_number_of_stored_commands() - 1;
            }
            if (ps->cursor_position > 0) {
                ps->cursor_position--;
                ps->command_index--;
                for (int c = ps->cursor_position; c < buffer_len && c < COMMAND_BUFFER_SIZE - 2; c++) {
                    ps->command_buffer[c] = ps->command_buffer[c+1];
                }
                ps->command_buffer[ps->command_index] = '\0';
                ps->command_history_index = CGSE_number_of_stored_commands() - 1;
                CGSE_remove_command(ps->command_history_index);
                CGSE_store_command(ps->command_buffer);
            }
        }
        else if (key == KEY_UP) {
            if (ps->command_history_index > 0) {
                char *previous_command = CGSE_recall_command((size_t)ps->command_history_index - 1);
                if (previous_command != NULL) {
                    snprintf(ps->command_buffer, COMMAND_BUFFER_SIZE, "%s", previous_command);
                    reset_editing_cursor(ps);
                    ps->command_history_index--;
                }
            }
        }
        else if (key == KEY_DOWN) {
            if (ps->command_history_index < CGSE_number_of_stored_commands()) {
                char *next_command = CGSE_recall_command((size_t)(ps->command_history_index+1));
                if (next_command != NULL) {
                    snprintf(ps->command_buffer, COMMAND_BUFFER_SIZE, "%s", next_command);
                    reset_editing_cursor(ps);
                    ps->command_history_index++;
                }
            }
        }
        else if (key == KEY_LEFT) {
            if (ps->cursor_position > 0) {
                ps->cursor_position--;
            }
        }
        else if (key == KEY_RIGHT) {
            if (ps->cursor_position < buffer_len && ps->cursor_position < COMMAND_BUFFER_SIZE - 1) {
                ps->cursor_position++;
            }
        }
        else if (ps->command_index >= COMMAND_BUFFER_SIZE - 2) {
            ps->command_index = COMMAND_BUFFER_SIZE - 1;
            ps->command_buffer[ps->command_index] = '\0';
        }
        else if (key != '\n' && ps->command_index < COMMAND_BUFFER_SIZE - 1) {
            if (ps->cursor_position < buffer_len) {
                for (int k = buffer_len - 1; k >= ps->cursor_position; k--) {
                    ps->command_buffer[k+1] = ps->command_buffer[k];
                }
            }
            ps->command_buffer[ps->cursor_position++] = key;
            ps->command_index++;
            ps->command_buffer[ps->command_index] = '\0';
            buffer_len = strlen(ps->command_buffer);
            ps->command_history_index = CGSE_number_of_stored_commands() - 1;
            CGSE_remove_command(ps->command_history_index);
            CGSE_store_command(ps->command_buffer);
        }

        wmove(ps->command_window, ps->line, 0);
        wprintw(ps->command_window, "%s> %s", ps->command_prefix, ps->command_buffer);
        wclrtoeol(ps->command_window);
        update_editing_cursor(ps);

        if (key == '\n') {
            CGSE_execute_command(ps);
        }

        gettimeofday(&tv, NULL);
        t2 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
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

void process_command_queue(CGSE_program_state_t *ps)
{
    // TODO show time until next queued command is run
    // Queue up that command if it is time...
    // First command is up next 
    // It is removed once sent to the satellite

    // Run all commands that are due, up to a timeout
    struct timeval tv = {0};
    gettimeofday(&tv, NULL);
    double t1 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
    double t2 = t1;

    CGSE_command_queue_entry_t *e = NULL;

    while ((e = CGSE_command_queue_next()) != NULL && t2 - t1 < 0.25) {
        memcpy(ps->editing_buffer, ps->command_buffer, COMMAND_BUFFER_SIZE);
        snprintf(ps->command_buffer, COMMAND_BUFFER_SIZE, "%s", e->command_text);
        CGSE_execute_command(ps);
        wmove(ps->command_window, ps->line, 0);
        wprintw(ps->command_window, "queue-> %s", e->command_text);
        wclrtoeol(ps->command_window);
        wprintw(ps->command_window, "%s> %s", ps->command_prefix, ps->editing_buffer);
        memcpy(ps->command_buffer, ps->editing_buffer, COMMAND_BUFFER_SIZE);
        update_editing_cursor(ps);
        wrefresh(ps->command_window);
        CGSE_command_queue_remove_next();

        gettimeofday(&tv, NULL);
        t2 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
    }

    return;
}

// Moves cursor to end of editing line
void reset_editing_cursor(CGSE_program_state_t *ps)
{
    size_t buffer_len = strlen(ps->command_buffer);
    ps->cursor_position = buffer_len;
    ps->command_index = buffer_len;
    ps->col = strlen(ps->command_prefix) + 2 + buffer_len;
    if (ps->line > LINES - 1) {
        ps->line = LINES - 1;
    }
    wmove(ps->command_window, ps->line, ps->col);

    return;
}


// Moves cursor to new column of editing line
void update_editing_cursor(CGSE_program_state_t *ps)
{
    size_t buffer_len = strlen(ps->command_buffer);
    ps->command_index = buffer_len;
    ps->col = strlen(ps->command_prefix) + 2 + ps->cursor_position;
    size_t max_col = strlen(ps->command_prefix) + 2 + buffer_len; 
    if (ps->col < 0) {
        ps->col = 0;
    }
    else if (ps->col > max_col) {
        ps->col = max_col;
    }
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
