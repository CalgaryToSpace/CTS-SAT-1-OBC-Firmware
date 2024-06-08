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

#include <time.h>
#include <sys/time.h>

// TODO use curses to get an output window and an input window?


volatile sig_atomic_t running = 1;

static void interrupthandler(int sig)
{
    (void)sig;
    running = 0;
}

int main(int argc, char **argv)
{

    int status = 0;

    GSE_program_state_t program_state = {0};
    program_state.baud_rate = 115200;
    program_state.command_prefix = "CTS1";

    if (argc > 2)
    {
        fprintf(stderr, "usage: %s [<device-path>] \n", argv[0]);
        return EXIT_FAILURE;
    }

    struct sigaction intact = {0};
    intact.sa_handler = interrupthandler;
    sigaction(SIGINT, &intact, NULL);

    status = init_terminal_screen(&program_state);
    if (status != 0)
    {
        endwin();
        fprintf(stderr, "Unable to initialize ncurses screen.\n");
        return EXIT_FAILURE;
    }
    
    if (argc == 2)
    {
        program_state.device_path = argv[1];
        status = connect_to_satellite(&program_state);
        if (status != 0)
        {
            wprintw(program_state.command_window, "\n Unable to connect to satellite using \"%s\"\n", program_state.device_path);
            // Do not quit - can try again later
        }
        else
        {
            program_state.satellite_connected = true;
        }

    }

    // Set up a while loop, with a sleep to reduce processor overhead
    uint8_t receive_buffer[RECEIVE_BUFFER_SIZE] = {0};
    ssize_t bytes_received = 0;
    int y = 0;
    int x = 0;
    int key = 0;
    char command_buffer[COMMAND_BUFFER_SIZE] = {0};
    int command_index = 0;
    ssize_t bytes_sent = 0;

    char telecommand_buffer[TCMD_BUFFER_SIZE] = {0};
    wprintw(program_state.command_window, "%s> %s", program_state.command_prefix, command_buffer);
    wrefresh(program_state.command_window);
    int line = 0;
    int col = 0;
    // Current line being edited
    CGSE_store_command("");
    size_t command_history_index = 0;

    while(running)
    {
        // select() to see if data are ready?
        memset(receive_buffer, 0, RECEIVE_BUFFER_SIZE);
        if (program_state.satellite_connected)
        {
            bytes_received = read(program_state.satellite_link, receive_buffer, RECEIVE_BUFFER_SIZE);
        }
        else 
        {
            bytes_received = 0;
        }
        if (bytes_received > 0)
        {
            wprintw(program_state.main_window, "%s", receive_buffer);
        }
        wrefresh(program_state.main_window);
        wrefresh(program_state.command_window);

        struct timeval tv = {0};
        gettimeofday(&tv, NULL);
        double t1 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
        double t2 = t1;

        getyx(program_state.command_window, line, col);

        while ((key = wgetch(program_state.command_window)) != ERR && (t2 - t1) < 0.5)
        {
            if (key == '\b' || key == 127 || key == KEY_BACKSPACE)
            {
                if (command_index > 0)
                {
                    command_index--;
                    command_buffer[command_index] = '\0';
                    command_history_index = CGSE_number_of_stored_commands() - 1;
                    CGSE_remove_command(command_history_index);
                    CGSE_store_command(command_buffer);
                }
            }
            else if (key == KEY_UP )
            {
                if (command_history_index > 0)
                {
                    char *previous_command = CGSE_recall_command((size_t)command_history_index - 1);
                    if (previous_command != NULL)
                    {
                        snprintf(command_buffer, COMMAND_BUFFER_SIZE, "%s", previous_command);
                        command_history_index--;
                        command_index = (int)strlen(previous_command);
                    }
                }
            }
            else if (key == KEY_DOWN )
            {
                if (command_history_index < CGSE_number_of_stored_commands())
                {
                    char *next_command = CGSE_recall_command((size_t)(command_history_index+1));
                    if (next_command != NULL)
                    {
                        snprintf(command_buffer, COMMAND_BUFFER_SIZE, "%s", next_command);
                        command_history_index++;
                        command_index = (int)strlen(next_command);
                    }
                }
            }
            else if (command_index >= COMMAND_BUFFER_SIZE - 2)
            {
                command_index = COMMAND_BUFFER_SIZE - 1;
                command_buffer[command_index] = '\0';
            }
            else if (key != '\n') 
            {
                command_buffer[command_index++] = key;
                command_buffer[command_index] = '\0';
                command_history_index = CGSE_number_of_stored_commands() - 1;
                CGSE_remove_command(command_history_index);
                CGSE_store_command(command_buffer);
            }

            wmove(program_state.command_window, line, 0);
            wclrtoeol(program_state.command_window);
            wprintw(program_state.command_window, "%s> %s", program_state.command_prefix, command_buffer);

            if (key == '\n')
            {
                CGSE_remove_command(CGSE_number_of_stored_commands()-1);
                status = CGSE_store_command(command_buffer);
                command_history_index = (ssize_t)CGSE_number_of_stored_commands() - 1;
                if (strcmp(".exit", command_buffer) == 0 || strcmp(".quit", command_buffer) == 0)
                {
                    running = 0;
                }
                else if (strcmp("?", command_buffer) == 0 || strcmp(".help", command_buffer) == 0)
                {
                    wprintw(program_state.command_window, "\n Available commands:\n");
                    wprintw(program_state.command_window, "%30s - %s\n", "? or .help", "show available commands");
                    wprintw(program_state.command_window, "%30s - %s\n", ".quit or .exit", "quit terminal");
                    wprintw(program_state.command_window, "\n");
                    wprintw(program_state.command_window, "%30s - %s\n", ".connect <device-path>", "establish as serial link to the satellite <device-path>");

                    wprintw(program_state.command_window, "\n%s> ", program_state.command_prefix);
                    // Reset command 
                    command_index = 0;
                    command_buffer[0] = '\0';
                }
                else if (command_buffer[0] == '.')
                {
                    // TODO search terminal command list for this command 
                    char *cmd_to_check = ".connect";
                    if (strncmp(cmd_to_check, command_buffer, strlen(cmd_to_check)) == 0)
                    {
                        // Based on "man strsep" example
                        char **ap, *arg_vector[3];
                        char *input_string = command_buffer;
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
                            program_state.device_path = arg_vector[1];
                            status = connect_to_satellite(&program_state);
                            if (status != 0)
                            {
                                wprintw(program_state.command_window, "\n Unable to connect to satellite using \"%s\"", program_state.device_path);
                                // Do not quit - can try again later
                            }
                            else 
                            {
                                program_state.satellite_connected = true;
                            }
                        }
                        else 
                        {
                            wprintw(program_state.command_window, "\n Usage: .connect <device-path>");
                        }
                    }
                    else 
                    {
                        wprintw(program_state.command_window, "\n Unrecognized terminal command");
                        wrefresh(program_state.command_window);
                    }
                    wprintw(program_state.command_window, "\n%s> ", program_state.command_prefix);
                    // Reset command 
                    command_index = 0;
                    command_buffer[0] = '\0';
                }
                else 
                {
                    if (command_index > COMMAND_BUFFER_SIZE - 1)
                    {
                        command_index = COMMAND_BUFFER_SIZE - 1;
                    }
                    command_buffer[command_index] = '\0';
                    // write...
                    snprintf(telecommand_buffer, TCMD_BUFFER_SIZE, "%s+%s", program_state.command_prefix, command_buffer);
                    if (program_state.satellite_connected)
                    {
                        if (strlen(command_buffer) > 0)
                        {
                            bytes_sent = write(program_state.satellite_link, telecommand_buffer, strlen(telecommand_buffer));
                        }
                        else 
                        {
                            wprintw(program_state.command_window, "\n Empty command not sent");
                        }
                    }
                    else 
                    {
                        wprintw(program_state.command_window, "\n Not connected to satellite");
                    }
                    
                    wprintw(program_state.command_window, "\n%s> ", program_state.command_prefix);
                    // Reset command 
                    command_index = 0;
                    command_buffer[0] = '\0';
                }
                CGSE_store_command(command_buffer);
                command_history_index = CGSE_number_of_stored_commands() - 1;
            }
            gettimeofday(&tv, NULL);
            t2 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
        }

        wrefresh(program_state.command_window);
        usleep(IO_WAIT_USEC);
        
    }

cleanup:
    clear();
    refresh();
    if (program_state.satellite_connected && program_state.satellite_link > 0)
    {
        close(program_state.satellite_link);
    }

    endwin();


    fprintf(stdout, "Byte!\n");

    return EXIT_SUCCESS;
}

int init_terminal_screen(GSE_program_state_t *program_state)
{
    int status = 0;
    setlocale(LC_ALL, "");
    initscr();
    status |= noecho();
    status |= raw();
    status |= intrflush(NULL, false);

    program_state->main_window = newwin(15, 0, 0, 0);
    if (program_state->main_window == NULL)
    {
        status |= 1;
    }
    status |= keypad(program_state->main_window, TRUE);
    status |= nodelay(program_state->main_window, TRUE);
    status |= scrollok(program_state->main_window, TRUE);
    status |= idlok(program_state->main_window, TRUE);

    program_state->command_window = newwin(0, 0, 16, 0);
    if (program_state->command_window == NULL)
    {
        status |= 1;
    }
    status |= keypad(program_state->command_window, TRUE);
    status |= nodelay(program_state->command_window, TRUE);
    status |= scrollok(program_state->command_window, TRUE);
    status |= idlok(program_state->command_window, TRUE);

    return status;

}

int connect_to_satellite(GSE_program_state_t *program_state)
{

    wprintw(program_state->command_window, "\n Connecting to \"%s\" @ %lu...", program_state->device_path, program_state->baud_rate);
    wrefresh(program_state->command_window);

    // Connect and set link parameters
    int sat_link = open(program_state->device_path, O_RDWR | O_NONBLOCK | O_NOCTTY);
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
    cfsetspeed(&sat_link_params, program_state->baud_rate);
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
    
    wprintw(program_state->command_window, "connected.\n");
    wrefresh(program_state->command_window);

    program_state->satellite_link = sat_link;

    return 0;
}
