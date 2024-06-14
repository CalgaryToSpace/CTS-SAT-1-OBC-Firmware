#include "main.h"

#include "command_history.h"

#include "telecommand_definitions.h"

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

#include <dirent.h>

#include <time.h>
#include <sys/time.h>


volatile sig_atomic_t running = 1;
volatile uint8_t TASK_heartbeat_is_on = 0;
extern const int16_t TCMD_NUM_TELECOMMANDS;
extern const TCMD_TelecommandDefinition_t TCMD_telecommand_definitions[];

static void interrupthandler(int sig)
{
    (void)sig;
    running = 0;
}

int main(int argc, char **argv)
{

    int status = 0;

    CGSE_program_state_t ps = {0};
    ps.argc = argc;
    ps.argv = argv;
    status = parse_args(&ps);
    if (status != 0)
    {
        return EXIT_FAILURE;
    }

    status = CGSE_init(&ps);
    if (status != 0)
    {
        return EXIT_FAILURE;
    }

    int key = 0;
    while(running)
    {
        update_link_status(&ps);
        wrefresh(ps.status_window);

        parse_telemetry(&ps);
        wrefresh(ps.main_window);
        wrefresh(ps.command_window);

        parse_input(&ps, key);
        wrefresh(ps.command_window);

        usleep(IO_WAIT_USEC);
    }

    clear();
    refresh();
    if (ps.satellite_connected && ps.satellite_link > 0)
    {
        close(ps.satellite_link);
    }

    endwin();


    fprintf(stdout, "Byte!\n");

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

    ps->main_window = newwin(15, 0, 1, 0);
    if (ps->main_window == NULL)
    {
        status |= 1;
    }
    status |= keypad(ps->main_window, TRUE);
    status |= nodelay(ps->main_window, TRUE);
    status |= scrollok(ps->main_window, TRUE);
    status |= idlok(ps->main_window, TRUE);

    ps->command_window = newwin(0, 0, 16, 0);
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

int CGSE_connect(CGSE_program_state_t *ps)
{
    // Connect and set link parameters
    int sat_link = open(ps->satellite_link_path, O_RDWR | O_NONBLOCK | O_NOCTTY);
    if (sat_link == -1)
    {
        return -1;
    }

    struct termios sat_link_params;
    int result = tcgetattr(sat_link, &sat_link_params);
    if (result != 0)
    {
        return -1;
    }
    cfsetspeed(&sat_link_params, ps->baud_rate);
    sat_link_params.c_cflag &= ~PARENB;
    sat_link_params.c_cflag &= ~CSTOPB;
    sat_link_params.c_cflag &= ~CSIZE;
    sat_link_params.c_cflag &= ~CS8;
    sat_link_params.c_cflag &= ~CRTSCTS;
    sat_link_params.c_cc[VMIN] = 1;
    sat_link_params.c_cc[VTIME] = 1;
    sat_link_params.c_cflag |= (CREAD | CLOCAL);
    cfmakeraw(&sat_link_params);
    
    result = tcsetattr(sat_link, TCSANOW, &sat_link_params);
    if (result != 0)
    {
        return -1;
    }
    tcflush(sat_link, TCIOFLUSH);
    
    update_link_status(ps);
    ps->satellite_link = sat_link;
    ps->satellite_connected = true;

    return 0;
}

int parse_input(CGSE_program_state_t *ps, int key)
{
    int status = 0;
    int line = 0;
    int col = 0;
    ssize_t bytes_sent = 0;
    size_t buffer_len = 0;

    struct timeval tv = {0};
    gettimeofday(&tv, NULL);
    double t1 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
    double t2 = t1;

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
            ps->command_index = strlen(ps->command_buffer);
            ps->cursor_position = ps->command_index;
            col = strlen(ps->command_prefix) + 2 + ps->cursor_position;
            wmove(ps->command_window, line, col);
            if (strlen(ps->command_buffer) > 0)
            {
                if (ps->command_history_index < CGSE_number_of_stored_commands() - 1)
                {
                    if (strlen(CGSE_recall_command(CGSE_number_of_stored_commands() - 1)) == 0)
                    {
                        CGSE_remove_command(CGSE_number_of_stored_commands() - 1);
                    }
                    CGSE_store_command(ps->command_buffer);
                }
                CGSE_store_command("");
                ps->command_history_index = CGSE_number_of_stored_commands() - 1;
            }
            if (strcmp(".exit", ps->command_buffer) == 0 || strcmp(".quit", ps->command_buffer) == 0)
            {
                running = 0;
            }
            else if (strcmp("?", ps->command_buffer) == 0 || strcmp(".help", ps->command_buffer) == 0)
            {
                wprintw(ps->command_window, "\n Available commands:\n");
                wprintw(ps->command_window, "%30s - %s\n", "? or .help", "show available commands");
                wprintw(ps->command_window, "%30s - %s\n", ".quit or .exit", "quit terminal");
                wprintw(ps->command_window, "\n");
                wprintw(ps->command_window, "%30s - %s\n", ".connect [<device-path>]", "connect to the satellite, optionally using <device-path>");
                wprintw(ps->command_window, "%30s - %s\n", ".disconnect", "disconnect from the satellite");
                wprintw(ps->command_window, "%30s - %s\n", ".telecommands", "list telecommands");
                wprintw(ps->command_window, "%30s - %s\n", ".upload_mpi_firwmare <filename>", "upload MPI firmware from <filename> relative to the current directory. ");

                wprintw(ps->command_window, "\n%s> ", ps->command_prefix);
                // Reset command 
                ps->command_index = 0;
                ps->cursor_position = 0;
                ps->command_buffer[0] = '\0';
            }
            else if (ps->command_buffer[0] == '.')
            {
                // TODO search terminal command list for this command 
                if (strncmp(".connect", ps->command_buffer, strlen(".connect")) == 0)
                {
                    if (ps->satellite_connected)
                    {
                        CGSE_disconnect(ps);
                        
                    }
                    if (strlen(ps->command_buffer) > strlen(".connect"))
                    {
                        char *arg_vector[2];
                        int n_connect_args = 2;
                        char *buf = CGSE_parse_command_args(ps, &n_connect_args, arg_vector);
                        if (n_connect_args == 2)
                        {
                            snprintf(ps->satellite_link_path, FILENAME_MAX, "%s", arg_vector[1]);
                        }
                        free(buf);
                    }
                    CGSE_connect(ps);
                }
                else if (strcmp(".disconnect", ps->command_buffer) == 0)
                {
                    CGSE_disconnect(ps);
                }
                else if (strcmp(".telecommands", ps->command_buffer) == 0)
                {
                    wprintw(ps->command_window, "\n");
                    CGSE_list_telecommands(ps);
                }
                else if (strncmp(".ls", ps->command_buffer, strlen(".ls")) == 0)
                {
                    if (strlen(ps->command_buffer) > strlen(".ls"))
                    {
                        char *arg_vector[2];
                        int n_ls_args = 2;
                        char *buf = CGSE_parse_command_args(ps, &n_ls_args, arg_vector);
                        if (n_ls_args == 2)
                        {
                            snprintf(ps->current_directory, FILENAME_MAX, "%s", arg_vector[1]);
                        }
                        free(buf);
                    }
                    wclrtoeol(ps->command_window);
                    wprintw(ps->command_window, "\n");
                    CGSE_ls_dir(ps);
                }
                else if (strncmp(".upload_mpi_firmware", ps->command_buffer, strlen(".upload_mpi_firmware")) == 0)
                {
                    if (!ps->satellite_connected)
                    {
                        wprintw(ps->command_window, "\nNot connected to satellite.");
                    }
                    else if (strlen(ps->command_buffer) > strlen(".upload_mpi_firmware"))
                    {
                        char *arg_vector[2];
                        int n_args = 2;
                        char *buf = CGSE_parse_command_args(ps, &n_args, arg_vector);
                        if (n_args == 2)
                        {
                            char path[FILENAME_MAX];
                            snprintf(path, FILENAME_MAX, "%s/%s", ps->current_directory, arg_vector[1]);
                            char *CGSE_base64_full = CGSE_base64_encode_from_file(ps, path);
                            if (CGSE_base64_full == NULL)
                            {
                                wprintw(ps->command_window, "\nUnable to load firmware bytes as base64 from %s", path);
                            }
                            // Send bytes in groups of 64 to the satellite
                            free(CGSE_base64_full);
                        }
                        else 
                        {
                            wprintw(ps->command_window, "\nUnable to parse command.");
                        }
                        free(buf);
                    }
                    wclrtoeol(ps->command_window);
                }
                else 
                {
                    wprintw(ps->command_window, "\n Unrecognized terminal command");
                }
                wprintw(ps->command_window, "\n%s> ", ps->command_prefix);
                wrefresh(ps->command_window);
                // Reset command 
                ps->command_index = 0;
                ps->cursor_position = 0;
                ps->command_buffer[0] = '\0';
            }
            else 
            {
                if (ps->command_index > COMMAND_BUFFER_SIZE - 1)
                {
                    ps->command_index = COMMAND_BUFFER_SIZE - 1;
                    if (ps->cursor_position > 0)
                    {
                        ps->cursor_position = ps->command_index;
                    }
                }
                ps->command_buffer[ps->command_index] = '\0';
                col = strlen(ps->command_prefix) + 2 + strlen(ps->command_buffer);
                wmove(ps->command_window, line, col);
                // write...
                snprintf(ps->telecommand_buffer, TCMD_BUFFER_SIZE, "%s+%s", ps->command_prefix, ps->command_buffer);
                if (strlen(ps->command_buffer) > 0)
                {
                    if (ps->satellite_connected)
                    {
                        bytes_sent = write(ps->satellite_link, ps->telecommand_buffer, strlen(ps->telecommand_buffer));
                    }
                    else 
                    {
                        wprintw(ps->command_window, "\n Not connected to satellite");
                    }
                }
                wprintw(ps->command_window, "\n%s> ", ps->command_prefix);
                // Reset command 
                ps->command_index = 0;
                ps->cursor_position = 0;
                ps->command_buffer[0] = '\0';
            }
            ps->command_history_index = CGSE_number_of_stored_commands() - 1;
        }

        gettimeofday(&tv, NULL);
        t2 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
    }

    return 0;
}

int parse_args(CGSE_program_state_t *ps)
{
    int status = 0;

    ps->baud_rate = CGSE_DEFAULT_BAUD_RATE;
    ps->command_prefix = CGSE_DEFAULT_TELECOMMAND_PREFIX;
    ps->auto_connect = true;
    snprintf(ps->current_directory, FILENAME_MAX, "%s", ".");

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
        if (strncmp("--baud-rate=", arg, 12) == 0)
        {
            if (strlen(arg) < 13)
            {
                fprintf(stderr, "Unable to interpret %s\n", arg);
                return EXIT_FAILURE;
            }
            ps->baud_rate = atoi(arg + 12);
            ps->nOptions++;
        }
        if (strncmp("--command-prefix=", arg, 17) == 0)
        {
            if (strlen(arg) < 18)
            {
                fprintf(stderr, "Unable to interpret %s\n", arg);
                return EXIT_FAILURE;
            }
            ps->command_prefix = arg + 17;
            ps->nOptions++;
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
            CGSE_help(ps->argv[0]);
            return 1;
        }
        else 
        {
            fprintf(stderr, "Unrecognized option %s\n", arg);
            return EXIT_FAILURE;
        }

    }

    if (strlen(ps->satellite_link_path) == 0)
    {
        snprintf(ps->satellite_link_path, FILENAME_MAX, "/dev");
        find_link_path(ps->satellite_link_path);
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

void CGSE_help(char *name)
{
    fprintf(stdout, "usage: %s [option] ...\n", name);

    fprintf(stdout, "Options:\n");
    fprintf(stdout, "%30s -- %s\n", "--link=<path>", "Set the satellite link path. Chosen automatically by default.");
    fprintf(stdout, "%30s -- %s%d.\n", "--baud-rate=<rate>", "Set the satellite link baud rate. Default: ", CGSE_DEFAULT_BAUD_RATE);
    fprintf(stdout, "%30s -- %s\n", "--no-auto-connect", "Start program without connecting to satellite.");
    fprintf(stdout, "%30s -- %s\n", "--command-prefix=<prefix>", "Telecommand prefix. Default: " CGSE_DEFAULT_TELECOMMAND_PREFIX );

    /// more help
    return;
}

/// linkpath must be allocated to FILENAME_MAX and must be initialized with the path of the system device files.
int find_link_path(char *linkpath)
{
    struct dirent *dp = NULL;
    char *dirpath = strdup(linkpath);             

    // From man 3 directory example
    DIR *directory = opendir(dirpath);
    if (directory == NULL)
        return -1;

    char *pattern = "tty.usbmodem";
    int len = strlen(pattern);
    while ((dp = readdir(directory)) != NULL) 
    {
        if (strncmp(dp->d_name, pattern, len) == 0) 
        {
            snprintf(linkpath, FILENAME_MAX, "%s/%s", dirpath, dp->d_name);
            break;
        }
    }
    (void)closedir(directory);
    free(dirpath);

    return 0;
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


void CGSE_list_telecommands(CGSE_program_state_t *ps)
{

    const TCMD_TelecommandDefinition_t *cmd = NULL;
    int nArgs = 0;
    for (int i = 0; i < TCMD_NUM_TELECOMMANDS; i++)
    {
        cmd = &TCMD_telecommand_definitions[i];
        nArgs = cmd->number_of_args;
        wprintw(ps->command_window, "%3d) %s(", i+1, cmd->tcmd_name);
        for (int a = 0; a < nArgs; a++)
        {
            wprintw(ps->command_window, "a%03d", a+1);
            if (a < nArgs - 1)
            {
                wprintw(ps->command_window, ",");
            }
        }
        wprintw(ps->command_window, ")\n");
    }

    return;
}

void update_link_status(CGSE_program_state_t *ps)
{
    if (ps->satellite_connected)
    {
        mvwprintw(ps->status_window, 0, 0, "Connected on %s @ %lu", ps->satellite_link_path, ps->baud_rate);
        wclrtoeol(ps->status_window);
        wrefresh(ps->status_window);
    }
    else
    {
        mvwprintw(ps->status_window, 0, 0, "");
        wclrtoeol(ps->status_window);
        wrefresh(ps->status_window);
    }

}

void parse_telemetry(CGSE_program_state_t *ps)
{
    // select() to see if data are ready?
    memset(ps->receive_buffer, 0, RECEIVE_BUFFER_SIZE);
    size_t start = 0;
    size_t stop = 0;

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
            start = stop;
            ps->bytes_received = read(ps->satellite_link, ps->receive_buffer + start, RECEIVE_BUFFER_SIZE);
            if (ps->bytes_received > 0)
            {
                stop = start + ps->bytes_received;
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
        wprintw(ps->main_window, "%s: %s", ps->time_buffer, ps->receive_buffer);
    }

    return;
}

void CGSE_disconnect(CGSE_program_state_t *ps)
{
    if (ps->satellite_link > 0)
    {
        close(ps->satellite_link);
        ps->satellite_link = 0;
        ps->satellite_connected = false;
    }

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
        status = CGSE_connect(ps);
        if (status != 0)
        {
            wprintw(ps->command_window, "\n Unable to connect to satellite using \"%s\"\n", ps->satellite_link_path);
            // Do not quit - can try again later
        }
    }


    wprintw(ps->command_window, "%s> %s", ps->command_prefix, ps->command_buffer);
    wrefresh(ps->command_window);
    // Current line being edited
    CGSE_store_command("");

    return 0;
}

char * CGSE_parse_command_args(CGSE_program_state_t *ps, int *nargs, char **arg_vector)
{
    if (ps == NULL || nargs == NULL || arg_vector == NULL)
    {
        return NULL;
    }

    // Based on "man strsep" example
    char *input_string_caller_must_free = strdup(ps->command_buffer);
    if (input_string_caller_must_free == NULL)
    {
        return NULL;
    }

    int max_arg = *nargs;
    int args_found = 0;
    for (char **ap = arg_vector; (*ap = strsep(&input_string_caller_must_free, " ")) != NULL;)
    {
        if (**ap != '\0')
        {
            args_found++;
            if (++ap >= &arg_vector[max_arg])
            {
                break;
            }
        }
    }
    *nargs = args_found;

    return input_string_caller_must_free;
}

int CGSE_ls_dir(CGSE_program_state_t *ps)
{
    struct dirent *dp = NULL;

    // From man 3 directory example
    DIR *directory = opendir(ps->current_directory);
    if (directory == NULL)
        return -1;

    int n_files = 0;
    while ((dp = readdir(directory)) != NULL) 
    {
        if (strlen(dp->d_name) > 0 && dp->d_name[0] != '.')
        {
            n_files++;
            wprintw(ps->command_window, "%3d) %s\n", n_files, dp->d_name);
        }
    }
    (void)closedir(directory);

    return 0;

}

char * CGSE_base64_encode_from_file(CGSE_program_state_t *ps, char *filename)
{
    if (filename == NULL || strlen(filename) == 0)
    {
        return NULL;
    }

    char *base64 = NULL;
    
    FILE *f = fopen(filename, "r");
    if (f == NULL)
    {
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    uint8_t *buffer = malloc(sizeof *buffer * file_size + 1);
    if (buffer == NULL)
    {
        fclose(f);
        return NULL;
    }

    fseek(f, 0, SEEK_SET);
    unsigned long bytes_read = fread(buffer, 1, file_size, f);
    if (bytes_read != file_size)
    {
        fclose(f);
        free(buffer);
        return NULL;
    }
   
    fclose(f);

    char *base64_caller_frees = CGSE_base64_encode_bytes(ps, buffer, file_size);

    free(buffer);

    return base64_caller_frees;
}

char * CGSE_base64_encode_bytes(CGSE_program_state_t *ps, uint8_t *byte_array, int len)
{
    int base64_size = ((len + 2) / 3) * 4;

    char *base64 = malloc(sizeof *base64 * (base64_size + 1));
    if (base64 == NULL)
    {
        return NULL;
    }

    uint8_t bits = 0;
    int offset = 0;
    int bytes_encoded = 0;
    int i = 0;
    for (; i < base64_size; i++)
    {
        offset = i % 4;
        if (bytes_encoded >= len)
        {
            bits = 255; // trigger '='
        }
        else
        {
            switch (offset)
            {
                case 0:
                    bits = byte_array[bytes_encoded] >> 2;
                    break;
                case 1:
                    bits = ((byte_array[bytes_encoded] & 0b11) << 4) | (byte_array[bytes_encoded + 1] >> 4);
                    bytes_encoded++;
                    break;
                case 2:
                    bits = ((byte_array[bytes_encoded] & 0b1111) << 2) | (byte_array[bytes_encoded + 1] >> 6);
                    bytes_encoded++;
                    break;
                case 3:
                    bits = byte_array[bytes_encoded] & 0b111111;
                    bytes_encoded++;
                    break;
            }
        }
        base64[i] = CGSE_base64_encode_character(bits);
    }
    base64[i] = '\0';

    return base64;
}

char CGSE_base64_encode_character(uint8_t bits) {
    uint8_t result = 255;
    if (bits == 63)
    {
        result ='/';
    }
    else if (bits == 62)
    {
        result = '+';
    }
    else if (bits > 51 && bits < 62)
    {
        result = bits - 4;
    }
    else if (bits > 25 && bits < 52)
    {
        result = bits + 71;
    }
    else if  (bits < 26)
    {
        result = bits + 65;
    }
    else 
    {
        result = (uint8_t)'='; 
    }

    return (char) result;

}

