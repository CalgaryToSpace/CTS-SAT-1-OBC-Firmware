#include "main.h"

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

WINDOW *main_window = NULL;
WINDOW *command_window = NULL;

volatile sig_atomic_t running = 1;

static void interrupthandler(int sig)
{
    (void)sig;
    running = 0;
}

int main(int argc, char **argv)
{

    GSE_program_state_t program_state = {0};
    program_state.baud_rate = 115200;
    program_state.command_prefix = "CTS1";

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <devicepath> \n", argv[0]);
        return EXIT_FAILURE;
    }

    program_state.device_path = argv[1];

    struct sigaction intact = {0};
    intact.sa_handler = interrupthandler;
    sigaction(SIGINT, &intact, NULL);

    // window setup
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    raw();
    intrflush(NULL, false);

    main_window = newwin(15, 0, 0, 0);
    keypad(main_window, TRUE);
    nodelay(main_window, TRUE);
    scrollok(main_window, TRUE);
    idlok(main_window, TRUE);

    command_window = newwin(0, 0, 16, 0);
    keypad(command_window, TRUE);
    nodelay(command_window, TRUE);
    scrollok(command_window, TRUE);
    idlok(command_window, TRUE);

    wprintw(main_window, "Connecting to \"%s\" @ %lu...", program_state.device_path, program_state.baud_rate);
    wrefresh(main_window);

    // Connect and set link parameters
    int sat_link = open(program_state.device_path, O_RDWR | O_NONBLOCK | O_NOCTTY);
    if (sat_link == -1)
    {
        endwin();
        fprintf(stderr, "Unable to open satellite link.\n");
        return EXIT_FAILURE;
    }

    struct termios sat_link_params;
    int result = tcgetattr(sat_link, &sat_link_params);
    if (result != 0)
    {
        fprintf(stderr, "Unable to get satellite link information.\n");
        goto cleanup;
    }
    cfsetspeed(&sat_link_params, program_state.baud_rate);
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
        fprintf(stderr, "Unable to set satellite link baud rate.\n");
        goto cleanup;
    }
    tcflush(sat_link, TCIOFLUSH);
    
    wprintw(main_window, "connected.\n");
    wrefresh(main_window);


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
    mvwprintw(command_window, 0, 0, "%s> %s", program_state.command_prefix, command_buffer);
    wrefresh(command_window);
    int line = 0;
    int col = 0;

    while(running)
    {
        // select() to see if data are ready?
        memset(receive_buffer, 0, RECEIVE_BUFFER_SIZE);
        bytes_received = read(sat_link, receive_buffer, RECEIVE_BUFFER_SIZE);
        if (bytes_received > 0)
        {
            wprintw(main_window, "%s", receive_buffer);
            wrefresh(main_window);
            wrefresh(command_window);
        }

        struct timeval tv = {0};
        gettimeofday(&tv, NULL);
        double t1 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
        double t2 = t1;

        getyx(command_window, line, col);

        while ((key = wgetch(command_window)) != ERR && (t2 - t1) < 0.5)
        {
            if (key == '\b' || key == 127 || key == KEY_BACKSPACE)
            {
                if (command_index > 0)
                {
                    command_index--;
                    command_buffer[command_index] = '\0';
                }
            }
            else if (command_index >= COMMAND_BUFFER_SIZE - 2)
            {
                command_index = COMMAND_BUFFER_SIZE - 1;
            }
            else if (key != '\n') 
            {
                command_buffer[command_index++] = key;
            }
            command_buffer[command_index] = '\0';
            wmove(command_window, line, 0);
            wclrtoeol(command_window);
            wprintw(command_window, "%s> %s", program_state.command_prefix, command_buffer);
            if (key == '\n')
            {
                wprintw(command_window, "\n%s> ", program_state.command_prefix);
                if (strcmp(".exit", command_buffer) == 0 || strcmp(".quit", command_buffer) == 0)
                {
                    running = 0;
                }
                else if (strcmp("?", command_buffer) == 0 || strcmp(".help", command_buffer) == 0)
                {
                    wprintw(command_window, "Available commands:\n");
                    wprintw(command_window, "%30s - %s\n", "? or .help", "show available commands");
                    wprintw(command_window, "%30s - %s\n", ".quit or .exit", "quit terminal");

                    wprintw(command_window, "\n%s> ", program_state.command_prefix);
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
                    if (strlen(command_buffer) > 0 && command_buffer[0] != '.')
                    {
                        bytes_sent = write(sat_link, telecommand_buffer, strlen(telecommand_buffer));
                    }
                    
                    // Reset command 
                    command_index = 0;
                    command_buffer[0] = '\0';
                }
            }
            gettimeofday(&tv, NULL);
            t2 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
        }

        wrefresh(command_window);
        usleep(IO_WAIT_USEC);
        
    }

cleanup:
    clear();
    refresh();
    if (sat_link > 0)
    {
        close(sat_link);
    }

    endwin();


    fprintf(stdout, "Byte!\n");

    return EXIT_SUCCESS;
}
