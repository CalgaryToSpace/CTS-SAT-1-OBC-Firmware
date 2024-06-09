#include "main.h"

#include "command_history.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/fcntl.h>
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

static void interrupthandler(int sig)
{
    (void)sig;
    running = 0;
}

int main(int argc, char **argv)
{

    int status = 0;

    GSE_program_state_t ps = {0};
    ps.argc = argc;
    ps.argv = argv;
    status = parse_args(&ps);
    if (status != 0)
    {
        return EXIT_FAILURE;
    }

    struct sigaction intact = {0};
    intact.sa_handler = interrupthandler;
    sigaction(SIGINT, &intact, NULL);

    status = init_terminal_screen(&ps);
    if (status != 0)
    {
        endwin();
        fprintf(stderr, "Unable to initialize ncurses screen.\n");
        return EXIT_FAILURE;
    }
    
    if (ps.auto_connect)
    {
        status = connect_to_satellite(&ps);
        if (status != 0)
        {
            wprintw(ps.command_window, "\n Unable to connect to satellite using \"%s\"\n", ps.satellite_link_path);
            // Do not quit - can try again later
        }
    }

    // Set up a while loop, with a sleep to reduce processor overhead
    uint8_t receive_buffer[RECEIVE_BUFFER_SIZE] = {0};
    ssize_t bytes_received = 0;
    int y = 0;
    int x = 0;
    int key = 0;

    wprintw(ps.command_window, "%s> %s", ps.command_prefix, ps.command_buffer);
    wrefresh(ps.command_window);
    // Current line being edited
    CGSE_store_command("");

    while(running)
    {
        // select() to see if data are ready?
        memset(receive_buffer, 0, RECEIVE_BUFFER_SIZE);
        if (ps.satellite_connected)
        {
            bytes_received = read(ps.satellite_link, receive_buffer, RECEIVE_BUFFER_SIZE);
        }
        else 
        {
            bytes_received = 0;
        }
        if (bytes_received > 0)
        {
            wprintw(ps.main_window, "%s", receive_buffer);
        }
        wrefresh(ps.main_window);
        wrefresh(ps.command_window);

        struct timeval tv = {0};
        gettimeofday(&tv, NULL);
        double t1 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
        double t2 = t1;


        
        while ((key = wgetch(ps.command_window)) != ERR && (t2 - t1) < 0.5)
        {
            parse_input(&ps, key);
            gettimeofday(&tv, NULL);
            t2 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
        }

        wrefresh(ps.command_window);
        usleep(IO_WAIT_USEC);
        
    }

cleanup:
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

int init_terminal_screen(GSE_program_state_t *ps)
{
    int status = 0;
    setlocale(LC_ALL, "");
    initscr();
    status |= noecho();
    status |= raw();
    status |= intrflush(NULL, false);

    ps->main_window = newwin(15, 0, 0, 0);
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

int connect_to_satellite(GSE_program_state_t *ps)
{

    wprintw(ps->command_window, "\n Connecting to \"%s\" @ %lu...", ps->satellite_link_path, ps->baud_rate);
    wrefresh(ps->command_window);

    // Connect and set link parameters
    int sat_link = open(ps->satellite_link_path, O_RDWR | O_NONBLOCK | O_NOCTTY);
    if (sat_link == -1)
    {
        endwin();
        fprintf(stderr, "Unable to open satellite link.\n");
        return -1;
    }

    struct termios sat_link_params;
    int result = tcgetattr(sat_link, &sat_link_params);
    if (result != 0)
    {
        endwin();
        fprintf(stderr, "Unable to get satellite link information.\n");
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
        endwin();
        fprintf(stderr, "Unable to set satellite link baud rate.\n");
        return -1;
    }
    tcflush(sat_link, TCIOFLUSH);
    
    wprintw(ps->command_window, "connected.\n");
    wrefresh(ps->command_window);

    ps->satellite_link = sat_link;
    ps->satellite_connected = true;

    return 0;
}

int parse_input(GSE_program_state_t *ps, int key)
{
    int status = 0;
    int line = 0;
    int col = 0;
    ssize_t bytes_sent = 0;

    getyx(ps->command_window, line, col);
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
            for (int c = ps->cursor_position; c < strlen(ps->command_buffer) && c < COMMAND_BUFFER_SIZE - 2; c++)
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
                ps->cursor_position = strlen(ps->command_buffer);
                col = strlen(ps->command_prefix) + 2 + ps->cursor_position;
                ps->command_history_index--;
                ps->command_index = strlen(ps->command_buffer);
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
                ps->cursor_position = strlen(ps->command_buffer);
                col = strlen(ps->command_prefix) + 2 + ps->cursor_position;
                ps->command_history_index++;
                ps->command_index = strlen(ps->command_buffer);
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
        if (ps->cursor_position < strlen(ps->command_buffer) && ps->cursor_position < COMMAND_BUFFER_SIZE - 1)
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
    else if (key != '\n') 
    {
        ps->command_buffer[ps->command_index++] = key;
        ps->command_buffer[ps->command_index] = '\0';
        ps->command_history_index = CGSE_number_of_stored_commands() - 1;
        CGSE_remove_command(ps->command_history_index);
        CGSE_store_command(ps->command_buffer);
        ps->cursor_position++;
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
            wprintw(ps->command_window, "%30s - %s\n", ".connect <device-path>", "establish as serial link to the satellite <device-path>");

            wprintw(ps->command_window, "\n%s> ", ps->command_prefix);
            // Reset command 
            ps->command_index = 0;
            ps->command_buffer[0] = '\0';
        }
        else if (ps->command_buffer[0] == '.')
        {
            // TODO search terminal command list for this command 
            char *cmd_to_check = ".connect";
            if (strncmp(cmd_to_check, ps->command_buffer, strlen(cmd_to_check)) == 0)
            {
                // Based on "man strsep" example
                char **ap, *arg_vector[3];
                char *input_string = ps->command_buffer;
                int n_connect_args = 0;
                for (ap = arg_vector; (*ap = strsep(&input_string, " ")) != NULL;)
                {
                    if (**ap != '\0')
                    {
                        n_connect_args++;
                        if (++ap >= &arg_vector[3])
                        {
                            break;
                        }
                    }
                }
                if (n_connect_args == 2)
                {
                    snprintf(ps->satellite_link_path, FILENAME_MAX, "%s", arg_vector[1]);
                    status = connect_to_satellite(ps);
                    if (status != 0)
                    {
                        wprintw(ps->command_window, "\n Unable to connect to satellite using \"%s\"", ps->satellite_link_path);
                        // Do not quit - can try again later
                    }
                    else 
                    {
                        ps->satellite_connected = true;
                    }
                }
                else 
                {
                    wprintw(ps->command_window, "\n Usage: .connect <device-path>");
                }
            }
            else 
            {
                wprintw(ps->command_window, "\n Unrecognized terminal command");
            }
            wprintw(ps->command_window, "\n%s> ", ps->command_prefix);
            wrefresh(ps->command_window);
            // Reset command 
            ps->command_index = 0;
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
            ps->command_buffer[0] = '\0';
        }
        ps->command_history_index = CGSE_number_of_stored_commands() - 1;
    }

    return 0;
}

int parse_args(GSE_program_state_t *ps)
{
    int status = 0;

    ps->baud_rate = CGSE_DEFAULT_BAUD_RATE;
    ps->command_prefix = CGSE_DEFAULT_TELECOMMAND_PREFIX;
    ps->auto_connect = true;

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

