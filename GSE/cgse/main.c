#include "main.h"

#include "command_history.h"
#include "command_queue.h"
#include "commands.h"

#include "telecommands/telecommand_definitions.h"

#include <math.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/termios.h>
#include <termios.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>

#include <locale.h>
#include <ncurses.h>
#include <errno.h>

#include <time.h>
#include <sys/time.h>


volatile sig_atomic_t running = 1;
volatile uint8_t TASK_heartbeat_is_on = 0;
int line = 0;
int col = 0;

extern const int16_t TCMD_NUM_TELECOMMANDS;
extern const TCMD_TelecommandDefinition_t TCMD_telecommand_definitions[];
extern const int CGSE_NUM_TERMINAL_COMMANDS; 
extern const CGSE_command_t CGSE_terminal_commands[];

static void interrupthandler(int sig)
{
    (void)sig;
    running = 0;
}

int main(int argc, char **argv)
{

    CGSE_program_state_t program_state = {0};
    CGSE_program_state_t *ps = &program_state;
    ps->argc = argc;
    ps->argv = argv;

    int arg_status = parse_args(ps);
    if (arg_status != 0)
    {
        return EXIT_FAILURE;
    }

    int init_status = CGSE_init(ps);
    if (init_status != 0)
    {
        return EXIT_FAILURE;
    }

    while(running)
    {
        update_link_status(ps);
        wrefresh(ps->status_window);

        parse_telemetry(ps);

        process_command_queue(ps);

        // Check for user input
        parse_input(ps);

        usleep(IO_WAIT_USEC);
    }

    clear();
    refresh();
    if (ps->satellite_connected && ps->satellite_link > 0)
    {
        close(ps->satellite_link);
    }

    endwin();

    int write_status = CGSE_write_command_history(ps);
    if (write_status != 0) {
        fprintf(stderr, "Error writing command history.\n");
    }
    CGSE_free_command_history();

    // TODO maybe write out remaining commands in the command queue for later
    // processing?
    CGSE_free_command_queue();

    return EXIT_SUCCESS;
}

int init_terminal_screen(CGSE_program_state_t *ps)
{
    int status = 0;
    setlocale(LC_ALL, "");
    initscr();
    status |= noecho();
    status |= raw();
    status |= intrflush(NULL, false);

    ps->status_window = newwin(1, 0, 0, 0);
    if (ps->status_window == NULL)
    {
        status |= 1;
    }
    status |= wattron(ps->status_window, A_REVERSE);

    ps->main_window = newwin(CGSE_TM_WINDOW_SIZE, 0, 1, 0);
    if (ps->main_window == NULL)
    {
        status |= 1;
    }
    status |= keypad(ps->main_window, TRUE);
    status |= nodelay(ps->main_window, TRUE);
    status |= scrollok(ps->main_window, TRUE);
    status |= idlok(ps->main_window, TRUE);

    ps->command_window = newwin(0, 0, CGSE_TM_WINDOW_SIZE + 1, 0);
    if (ps->command_window == NULL)
    {
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

    while ((key = wgetch(ps->command_window)) != ERR && (t2 - t1) < 0.5)
    {

        getyx(ps->command_window, line, col);
        buffer_len = strlen(ps->command_buffer);
        if (key == '\b' || key == 127 || key == KEY_BACKSPACE)
        {
            if (ps->command_history_index < CGSE_number_of_stored_commands() - 1)
            {
                CGSE_store_command(ps->command_buffer);
                ps->command_history_index = CGSE_number_of_stored_commands() - 1;
            }
            if (ps->cursor_position > 0)
            {
                ps->command_index--;
                ps->cursor_position--;
                col--;
                for (int c = ps->cursor_position; c < buffer_len && c < COMMAND_BUFFER_SIZE - 2; c++)
                {
                    ps->command_buffer[c] = ps->command_buffer[c+1];
                }
                ps->command_buffer[ps->command_index] = '\0';
                ps->command_history_index = CGSE_number_of_stored_commands() - 1;
                CGSE_remove_command(ps->command_history_index);
                CGSE_store_command(ps->command_buffer);
            }
        }
        else if (key == KEY_UP)
        {
            if (ps->command_history_index > 0)
            {
                char *previous_command = CGSE_recall_command((size_t)ps->command_history_index - 1);
                if (previous_command != NULL)
                {
                    snprintf(ps->command_buffer, COMMAND_BUFFER_SIZE, "%s", previous_command);
                    buffer_len = strlen(ps->command_buffer);
                    ps->cursor_position = buffer_len;
                    col = strlen(ps->command_prefix) + 2 + ps->cursor_position;
                    ps->command_history_index--;
                    ps->command_index = buffer_len;
                }
            }
        }
        else if (key == KEY_DOWN)
        {
            if (ps->command_history_index < CGSE_number_of_stored_commands())
            {
                char *next_command = CGSE_recall_command((size_t)(ps->command_history_index+1));
                if (next_command != NULL)
                {
                    snprintf(ps->command_buffer, COMMAND_BUFFER_SIZE, "%s", next_command);
                    buffer_len = strlen(ps->command_buffer);
                    ps->cursor_position = buffer_len;
                    col = strlen(ps->command_prefix) + 2 + ps->cursor_position;
                    ps->command_history_index++;
                    ps->command_index = buffer_len;
                }
            }
        }
        else if (key == KEY_LEFT)
        {
            if (ps->cursor_position > 0)
            {
                ps->cursor_position--;
                col--;
            }
        }
        else if (key == KEY_RIGHT)
        {
            if (ps->cursor_position < buffer_len && ps->cursor_position < COMMAND_BUFFER_SIZE - 1)
            {
                ps->cursor_position++;
                col++;
            }
        }
        else if (ps->command_index >= COMMAND_BUFFER_SIZE - 2)
        {
            ps->command_index = COMMAND_BUFFER_SIZE - 1;
            ps->command_buffer[ps->command_index] = '\0';
        }
        else if (key != '\n' && ps->command_index < COMMAND_BUFFER_SIZE - 1) 
        {
            if (ps->cursor_position < buffer_len)
            {
                for (int k = buffer_len - 1; k >= ps->cursor_position; k--)
                {
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
            col++;
        }

        wmove(ps->command_window, line, 0);
        wprintw(ps->command_window, "%s> %s", ps->command_prefix, ps->command_buffer);
        wclrtoeol(ps->command_window);
        wmove(ps->command_window, line, col);

        if (key == '\n')
        {
            CGSE_execute_command(ps);
        }

        gettimeofday(&tv, NULL);
        t2 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
    }

    wrefresh(ps->command_window);
    return 0;
}

int parse_args(CGSE_program_state_t *ps)
{
    int status = 0;

    struct timeval tv = {0};
    gettimeofday(&tv, NULL);
    ps->program_start_epoch_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    ps->baud_rate = CGSE_DEFAULT_BAUD_RATE;
    ps->command_prefix = CGSE_DEFAULT_TELECOMMAND_PREFIX;
    ps->auto_connect = true;
    ps->prepend_timestamp = false;
    snprintf(ps->current_directory, FILENAME_MAX, "%s", ".");
    
    char *history_path = getenv("HOME");
    snprintf(ps->command_history_file_path, FILENAME_MAX, "%s/%s", history_path != NULL ? history_path : "", CGSE_COMMAND_HISTORY_FILENAME);

    char *arg = NULL;

    for (int i = 1; i < ps->argc; i++)
    {
        arg = ps->argv[i];
        if (strncmp("--link=", arg, 7) == 0)
        {
            if (strlen(arg) < 8)
            {
                fprintf(stderr, "Unable to interpret %s\n", arg);
                return EXIT_FAILURE;
            }
            snprintf(ps->satellite_link_path, FILENAME_MAX, "%s", arg + 7);
            ps->nOptions++;
        }
        else if (strncmp("--baud-rate=", arg, 12) == 0)
        {
            if (strlen(arg) < 13)
            {
                fprintf(stderr, "Unable to interpret %s\n", arg);
                return EXIT_FAILURE;
            }
            ps->baud_rate = atoi(arg + 12);
            ps->nOptions++;
        }
        else if (strncmp("--command-prefix=", arg, 17) == 0)
        {
            if (strlen(arg) < 18)
            {
                fprintf(stderr, "Unable to interpret %s\n", arg);
                return EXIT_FAILURE;
            }
            ps->command_prefix = arg + 17;
            ps->nOptions++;
        }
        else if (strncmp("--command-history-filename=", arg, 27) == 0)
        {
            if (strlen(arg) < 28)
            {
                fprintf(stderr, "Unable to interpret %s\n", arg);
                return EXIT_FAILURE;
            }
            snprintf(ps->command_history_file_path, FILENAME_MAX, "%s", arg + 27);
            ps->nOptions++;
        }
        else if (strncmp("--commands=", arg, 11) == 0)
        {
            if (strlen(arg) < 12)
            {
                fprintf(stderr, "Unable to interpret %s\n", arg);
                return EXIT_FAILURE;
            }
            snprintf(ps->command_queue_file_path, FILENAME_MAX, "%s", arg + 11);
            ps->nOptions++;
            // Import commands from file
            int queue_read_res = CGSE_command_queue_read_commands(ps);
            if (queue_read_res != 0) {
                wprintw(ps->command_window, "\n Error loading commands from %s\n", ps->command_queue_file_path);
                return EXIT_FAILURE;
            }
        }
        else if (strcmp("--no-auto-connect", arg) == 0)
        {
            ps->auto_connect = false;
            ps->nOptions++;
        }
        else if (strcmp("--about", arg) == 0)
        {
            CGSE_about();
            return 1;
        }
        else if (strcmp("--license", arg) == 0)
        {
            CGSE_license();
            return 1;
        }
        else if (strcmp("--help", arg) == 0)
        {
            CGSE_commandline_help(ps->argv[0]);
            return 1;
        }
        else 
        {
            fprintf(stderr, "Unrecognized option %s\n", arg);
            return EXIT_FAILURE;
        }

    }

    if (ps->argc - ps->nOptions != 1) {
        CGSE_commandline_help(ps->argv[0]);
        return EXIT_FAILURE;
    }

    if (strlen(ps->satellite_link_path) == 0)
    {
        snprintf(ps->satellite_link_path, FILENAME_MAX, "/dev");
        CGSE_find_link_path(ps->satellite_link_path);
    }



    return 0;

}

void CGSE_license(void)
{
    fprintf(stdout, "MIT License. Etc.\n");
    return;
}

void CGSE_about(void)
{
    fprintf(stdout, "Lightweight GSE terminal for FrontierSat development.\n");
    fprintf(stdout, "Copyright (C) CalgaryToSpace (2024)\n");
    return;
}

void CGSE_commandline_help(char *name)
{
    fprintf(stdout, "usage: %s [option] ...\n", name);

    fprintf(stdout, "Options:\n");
    fprintf(stdout, "%30s -- %s\n", "--link=<path>", "Set the satellite link path. Chosen automatically by default.");
    fprintf(stdout, "%30s -- %s%d.\n", "--baud-rate=<rate>", "Set the satellite link baud rate. Default: ", CGSE_DEFAULT_BAUD_RATE);
    fprintf(stdout, "%30s -- %s\n", "--no-auto-connect", "Start program without connecting to satellite.");
    fprintf(stdout, "%30s -- %s\n", "--command-prefix=<prefix>", "Telecommand prefix. Default: " CGSE_DEFAULT_TELECOMMAND_PREFIX );
    fprintf(stdout, "%30s -- %s\n", "--command-history-filename=<filename>", "Command history filename. Default: ${HOME}/" CGSE_COMMAND_HISTORY_FILENAME);
    fprintf(stdout, "%30s -- %s\n", "--commands=<filename>", "Queue commands from <filename>.");

    return;
}

/// time_str must be large enough to store 32 chars
void CGSE_time_string(char *time_str)
{
    struct timeval tv = {0};
    gettimeofday(&tv, NULL);
    struct tm *t = gmtime(&tv.tv_sec);
    snprintf(time_str, CGSE_TIME_STR_MAX_LEN, "UTC=%4d-%02d-%02dT%02d:%02d:%02d.%06d", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, tv.tv_usec);

    char *result = strdup(time_str);
    return;
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

}

void parse_telemetry(CGSE_program_state_t *ps)
{
    // select() to see if data are ready?
    memset(ps->receive_buffer, 0, RECEIVE_BUFFER_SIZE);
    size_t stop = 0;
    static bool last_line_complete = true;
    static int8_t prepend_timestamp = -1;

    ps->bytes_received = 0;
    if (ps->satellite_connected)
    {
        CGSE_time_string(ps->time_buffer);
        struct timeval tv = {0};
        gettimeofday(&tv, NULL);
        double t1 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
        double t2 = t1;
        while (ps->bytes_received >= 0 && (t2-t1) < 0.25)
        {
            ps->bytes_received = read(ps->satellite_link, ps->receive_buffer + stop, RECEIVE_BUFFER_SIZE);
            if (ps->bytes_received > 0)
            {
                stop += ps->bytes_received;
            }
            gettimeofday(&tv, NULL);
            t2 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
        }
    }
    else 
    {
       stop = 0; 
    }
    if (stop > 0)
    {
        wclrtoeol(ps->main_window);
        if (stop >= RECEIVE_BUFFER_SIZE)
        {
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
    CGSE_command_queue_entry_t *e = NULL;
    // Run all commands that are due 
    // TODO maybe with a timeout as a safety...
    struct timeval tv = {0};
    gettimeofday(&tv, NULL);
    double t1 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
    double t2 = t1;
    memcpy(ps->editing_buffer, ps->command_buffer, COMMAND_BUFFER_SIZE);
    while ((e = CGSE_command_queue_next()) != NULL && t2 - t1 < 0.25) {
        snprintf(ps->command_buffer, COMMAND_BUFFER_SIZE, "%s", e->command_text);
        CGSE_execute_command(ps);
        wmove(ps->command_window, line, 0);
        wprintw(ps->command_window, "queue-> %s", e->command_text);
        wclrtoeol(ps->command_window);
        line++;
        wprintw(ps->command_window, "%s> %s", ps->command_prefix, ps->editing_buffer);
        wmove(ps->command_window, line, strlen(ps->command_prefix) + 2 + strlen(ps->editing_buffer));
        wrefresh(ps->command_window);
        CGSE_command_queue_remove_next();

        gettimeofday(&tv, NULL);
        t2 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
    }
    memcpy(ps->command_buffer, ps->editing_buffer, COMMAND_BUFFER_SIZE);

    return;
}


int CGSE_init(CGSE_program_state_t *ps)
{
    int status = 0;

    struct sigaction intact = {0};
    intact.sa_handler = interrupthandler;
    sigaction(SIGINT, &intact, NULL);

    status = init_terminal_screen(ps);
    if (status != 0)
    {
        endwin();
        fprintf(stderr, "Unable to initialize ncurses screen.\n");
        return EXIT_FAILURE;
    }
    
    if (ps->auto_connect)
    {
        status = CGSE_connect(ps, ".connect");
        if (status != 0)
        {
            wprintw(ps->command_window, "\n Unable to connect to satellite using \"%s\"\n", ps->satellite_link_path);
            // Do not quit - can try again later
        }
    }

    
    int read_history_res = CGSE_read_command_history(ps);
    if (read_history_res != 0) {
        CGSE_store_command("");
    }

    wprintw(ps->command_window, "%s> %s", ps->command_prefix, ps->command_buffer);
    wrefresh(ps->command_window);

    return 0;
}

