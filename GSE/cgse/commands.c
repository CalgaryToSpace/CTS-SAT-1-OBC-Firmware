#include "commands.h"
#include "command_history.h"
#include "command_queue.h"
#include "main.h"
#include "base64.h"

#include "telecommands/telecommand_definitions.h"

#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include <ncurses.h>

extern volatile sig_atomic_t running;
extern int line;
extern int col;

const CGSE_command_t CGSE_terminal_commands[] = {
    {".quit", ".quit", "quit the terminal", CGSE_quit, false},
    {".help", ".help", "show available terminal commands", CGSE_help, false},
    {".connect", ".connect [<device-name>]", "connect to the satellite, optionally using <device-path>", CGSE_connect, false},
    {".disconnect", ".disconnect", "disconnect from from satellite", CGSE_disconnect, false},
    {".show_timestamp", ".show_timestamp", "show GSE computer timestamp on received messages", CGSE_show_timestamp, false},
    {".hide_timestamp", ".hide_timestamp", "show GSE computer timestamp on received messages", CGSE_hide_timestamp, false},
    {".sync_time", ".sync_time", "synchronize satellite time with whit computer's time", CGSE_sync_timestamp, false},
    {".telecommands", ".telecommands", "list telecommands", CGSE_list_telecommands, false},
    {".list_queued_commands", ".list_queued_commands", "list queued telecommands and terminal commands", CGSE_list_queued_commands, false},
    {".ls", ".ls", "list current directory", CGSE_list_current_directory, false},
    {".upload_mpi_firmware", ".upload_mpi_firmware <file_name>", "upload MPI firmware from <file_name> relative to the current directory", CGSE_upload_mpi_firmware, false},
};
const int CGSE_NUM_TERMINAL_COMMANDS = sizeof(CGSE_terminal_commands) / sizeof(CGSE_command_t);

int CGSE_quit(CGSE_program_state_t *ps, char *cmd_string) 
{
    running = 0;

    return 0;
};

int CGSE_help(CGSE_program_state_t *ps, char *cmd_string) 
{
    wprintw(ps->command_window, "\n Available commands:\n");
    for (int c = 0; c < CGSE_NUM_TERMINAL_COMMANDS; c++) {
        const CGSE_command_t *cmd = &CGSE_terminal_commands[c];
        wprintw(ps->command_window, "%30s - %s\n", cmd->help_template, cmd->description);
    }
    // TODO show help for telecommands

    return 0;
}

int CGSE_connect(CGSE_program_state_t *ps, char *cmd_string)
{
    if (ps->satellite_connected) {
        CGSE_disconnect(ps, cmd_string);
    }

    char *arg_vector[2];
    int n_connect_args = 2;
    char *buf = CGSE_get_args_from_str(cmd_string, &n_connect_args, arg_vector);
    if (n_connect_args == 2) {
        snprintf(ps->satellite_link_path, FILENAME_MAX, "%s", arg_vector[1]);
    }
    else {
        CGSE_find_link_path(ps->satellite_link_path);
    }
    free(buf);

    // Connect and set link parameters
    int sat_link = open(ps->satellite_link_path, O_RDWR | O_NONBLOCK | O_NOCTTY);
    if (sat_link == -1) {
        wprintw(ps->command_window, "\n Error opening satellite link");
        return -1;
    }

    struct termios sat_link_params;
    int result = tcgetattr(sat_link, &sat_link_params);
    if (result != 0) {
        wprintw(ps->command_window, "\n Error setting satellite link parameters");
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
    if (result != 0) {
        wprintw(ps->command_window, "\n Error setting satellite link parameters");
        return -1;
    }
    tcflush(sat_link, TCIOFLUSH);
    
    update_link_status(ps);
    ps->satellite_link = sat_link;
    ps->satellite_connected = true;

    return 0;
}

int CGSE_disconnect(CGSE_program_state_t *ps, char *cmd_string) {
    if (ps->satellite_link > 0) {
        close(ps->satellite_link);
        ps->satellite_link = 0;
        ps->satellite_connected = false;
    }

    return 0;
}

int CGSE_show_timestamp(CGSE_program_state_t *ps, char *cmd_string) {
    ps->prepend_timestamp = true;
    return 0;
}

int CGSE_hide_timestamp(CGSE_program_state_t *ps, char *cmd_string) {
    ps->prepend_timestamp = false;
    return 0;
}

int CGSE_sync_timestamp(CGSE_program_state_t *ps, char *cmd_string) {
    char tcmd[256];
    struct timeval epoch = {0};
    int gtod_result = gettimeofday(&epoch, NULL);
    if (gtod_result != 0) {
        wprintw(ps->command_window, "\n Unable to get the time");
        return -1;
    }
    uint64_t epoch_ms = (uint64_t)epoch.tv_sec * 1000 + epoch.tv_usec/1000;
    // TODO account for a calibrated delay in
    // communicating with the satellite
    snprintf(tcmd, 256, "%s+set_system_time(%llu)!", ps->command_prefix, epoch_ms);
    if (ps->satellite_connected) {
        int bytes_sent = write(ps->satellite_link, tcmd, strlen(tcmd));
        if (bytes_sent <= 0) {
            wprintw(ps->command_window, "\n Error sending telecommand");
        }
    }
    else {
        wprintw(ps->command_window, "\n Not connected to satellite");
    }

    return 0;
}

int CGSE_list_telecommands(CGSE_program_state_t *ps, char *cmd_string) {
    wprintw(ps->command_window, "\n");
    const TCMD_TelecommandDefinition_t *cmd = NULL;
    int nArgs = 0;
    for (int i = 0; i < TCMD_NUM_TELECOMMANDS; i++) {
        cmd = &TCMD_telecommand_definitions[i];
        nArgs = cmd->number_of_args;
        wprintw(ps->command_window, "%3d) %s(", i+1, cmd->tcmd_name);
        for (int a = 0; a < nArgs; a++) {
            wprintw(ps->command_window, "a%03d", a+1);
            if (a < nArgs - 1) {
                wprintw(ps->command_window, ",");
            }
        }
        wprintw(ps->command_window, ")\n");
    }

    return 0;
}

int CGSE_list_current_directory(CGSE_program_state_t *ps, char *cmd_string) {
    char *arg_vector[2];
    int n_ls_args = 2;
    char *buf = CGSE_get_args_from_str(cmd_string, &n_ls_args, arg_vector);
    if (n_ls_args == 2) {
        snprintf(ps->current_directory, FILENAME_MAX, "%s", arg_vector[1]);
    }
    free(buf);
    wclrtoeol(ps->command_window);
    wprintw(ps->command_window, "\n");
    CGSE_ls_dir(ps);

    return 0;
}

int CGSE_list_queued_commands(CGSE_program_state_t *ps, char *cmd_string) {
    CGSE_command_queue_list_commands(ps);
    return 0;
}

int CGSE_upload_mpi_firmware(CGSE_program_state_t *ps, char *cmd_string) {
    if (!ps->satellite_connected) {
        wprintw(ps->command_window, "\nNot connected to satellite.");
        return -1;
    }

    char *arg_vector[2];
    int n_args = 2;
    char *buf = CGSE_get_args_from_str(cmd_string, &n_args, arg_vector);
    if (n_args == 2) {
        char mpi_firmware_path[FILENAME_MAX];
        snprintf(mpi_firmware_path, FILENAME_MAX, "%s/%s", ps->current_directory, arg_vector[1]);
        size_t mpi_firmware_length = 0;
        char *mpi_firmware = CGSE_base64_encode_from_file(mpi_firmware_path, &mpi_firmware_length);
        if (mpi_firmware == NULL) {
            wprintw(ps->command_window, "\nUnable to load firmware from %s", mpi_firmware_path);
        }
        else {
            // Send bytes as pages to the satellite
            // TODO get number of bytes, not number of base64
            // characters
            size_t mpi_firmware_size_base64 = strlen(mpi_firmware);
            size_t remaining_chars = mpi_firmware_size_base64;
            size_t chars_to_send = 0;
            size_t chars_sent = 0;
            char *p = mpi_firmware;
            char telemetry_buffer[COMMAND_BUFFER_SIZE] = {0};
            size_t tm_offset = 0;
            size_t tm_bytes_sent = 0;
            int mpi_firmware_page = 0;
            int mpi_firmware_bytes_sent = 0;
            int mpi_firmware_bytes_offset = 0;
            while (remaining_chars > 0) {
                chars_to_send = FIRMWARE_CHUNK_SIZE;    
                if (chars_to_send > remaining_chars) {
                    chars_to_send = remaining_chars;
                }
                mpi_firmware_bytes_offset = mpi_firmware_bytes_sent;
                tm_offset = snprintf(telemetry_buffer, COMMAND_BUFFER_SIZE, "%s+upload_mpi_firmware_page(%d,", ps->command_prefix, mpi_firmware_bytes_offset);
                memcpy(telemetry_buffer + tm_offset, p, chars_to_send);
                tm_offset += chars_to_send;
                //tm_offset += snprintf(telemetry_buffer + tm_offset, chars_to_send, "%s", p);
                snprintf(telemetry_buffer + tm_offset, COMMAND_BUFFER_SIZE - tm_offset, ",%s,%lu)!", arg_vector[1], mpi_firmware_length);

                tm_bytes_sent = write(ps->satellite_link, telemetry_buffer, strlen(telemetry_buffer));
                if (tm_bytes_sent == strlen(telemetry_buffer)) {
                    chars_sent += chars_to_send;
                    mpi_firmware_bytes_sent = (chars_sent * 3) / 4;
                    remaining_chars -= chars_to_send;
                    p += chars_to_send;
                    mpi_firmware_page++;
                    wprintw(ps->command_window, "\nSent MPI firmware page %d (total %d of %lu bytes)", mpi_firmware_page, mpi_firmware_bytes_sent, mpi_firmware_length);
                    wrefresh(ps->command_window);
                    // Return to main while loop, re-entering
                    // here?
                    // TODO check CRC from response, etc.
                    struct timeval tv = {0};
                    gettimeofday(&tv, NULL);
                    double t1 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
                    double t2 = t1;
                    int bytes_received = 0;
                    bool got_page = false;
                    char expected_response[255] = {0};
                    while ((t2-t1) < MPI_FIRMWARE_PAGE_TIMEOUT && !got_page) {
                        usleep(MPI_FIRMWARE_PAGE_TIMEOUT_STEP_US);
                        int new_bytes = read(ps->satellite_link, ps->receive_buffer, RECEIVE_BUFFER_SIZE);
                        bytes_received += new_bytes;
                        if (new_bytes > 0) {
                            snprintf(expected_response, 255, "Received MPI firmware page. Wrote %d bytes to \"%s\" at address %d", mpi_firmware_bytes_sent, arg_vector[1], mpi_firmware_bytes_offset);
                            //if (strncmp(expected_response, (char*)ps->receive_buffer, strlen(expected_response)) == 0)
                            if (strncmp(expected_response, (char*)ps->receive_buffer, 12) == 0) {
                                CGSE_time_string(ps->time_buffer);
                                got_page = true;
                            }
                        }
                        if (got_page) {
                            wprintw(ps->main_window, "%s: %s", ps->time_buffer, ps->receive_buffer);
                            wrefresh(ps->main_window); 
                            wrefresh(ps->command_window);
                        }
                        gettimeofday(&tv, NULL);
                        t2 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
                    }
                    if (!got_page) {
                        wprintw(ps->command_window, "\nDid not receive reply. Aborting firmware upload...");
                        break;
                    }
                }
                else {
                    // otherwise abort (or add this page to the
                    // failed-list and try those pages later?)
                    wprintw(ps->command_window, "\nFailed to send firmware page %d. Aborting...", mpi_firmware_page);
                    break;
                }

                // Check for interrupt...
                int mpi_key = 0;
                mpi_key = wgetch(ps->command_window);
                if (mpi_key == 'q') {
                    wprintw(ps->command_window, "\nUpload interrupted by user.\n");
                    wrefresh(ps->command_window);
                    break;
                }
            }
        }

        // All done
        free(mpi_firmware);
    }
    else {
        wprintw(ps->command_window, "\nUnable to parse command.");
    }
    free(buf);

    wclrtoeol(ps->command_window);
    return 0;
}

char * CGSE_get_args_from_str(char* args, int *nargs, char **arg_vector)
{
    if (args == NULL || nargs == NULL || arg_vector == NULL) {
        return NULL;
    }

    // Based on "man strsep" example
    char *input_string_caller_must_free = strdup(args);
    if (input_string_caller_must_free == NULL) {
        return NULL;
    }

    int max_arg = *nargs;
    int args_found = 0;
    for (char **ap = arg_vector; (*ap = strsep(&input_string_caller_must_free, " ")) != NULL;) {
        if (**ap != '\0') {
            args_found++;
            if (++ap >= &arg_vector[max_arg]) {
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
    if (directory == NULL) {
        return -1;
    }

    int n_files = 0;

    while ((dp = readdir(directory)) != NULL) {
        if (strlen(dp->d_name) > 0 && dp->d_name[0] != '.') {
            n_files++;
            wprintw(ps->command_window, "%3d) %s\n", n_files, dp->d_name);
        }
    }
    (void)closedir(directory);

    return 0;

}

/// link_path must be allocated to FILENAME_MAX and must be initialized with the path of the system device files.
int CGSE_find_link_path(char *link_path)
{
    struct dirent *dp = NULL;
    char *dirpath = strdup(link_path);             

    // From man 3 directory example
    DIR *directory = opendir(dirpath);
    if (directory == NULL)
        return -1;

    char *pattern = "tty.usbmodem";
    int len = strlen(pattern);

    while ((dp = readdir(directory)) != NULL) {
        if (strncmp(dp->d_name, pattern, len) == 0) {
            snprintf(link_path, FILENAME_MAX, "%s/%s", dirpath, dp->d_name);
            break;
        }
    }
    (void)closedir(directory);
    free(dirpath);

    return 0;
}

int CGSE_execute_command(CGSE_program_state_t *ps)
{
    ps->command_index = strlen(ps->command_buffer);
    ps->cursor_position = ps->command_index;
    col = strlen(ps->command_prefix) + 2 + ps->cursor_position;
    wmove(ps->command_window, line, col);
    if (strlen(ps->command_buffer) > 0) {
        if (ps->command_history_index < CGSE_number_of_stored_commands() - 1) {
            if (strlen(CGSE_recall_command(CGSE_number_of_stored_commands() - 1)) == 0) {
                CGSE_remove_command(CGSE_number_of_stored_commands() - 1);
            }
            CGSE_store_command(ps->command_buffer);
        }
        CGSE_store_command("");
        ps->command_history_index = CGSE_number_of_stored_commands() - 1;
    }
    if (ps->command_buffer[0] == '.') {
        bool got_command = false;
        for (int c = 0; c < CGSE_NUM_TERMINAL_COMMANDS; c++) {
            const CGSE_command_t *cmd = &CGSE_terminal_commands[c];
            if (strncmp(ps->command_buffer, cmd->name, strlen(cmd->name)) == 0) {
                int cmd_status = cmd->function(ps, ps->command_buffer);
                got_command = true;
                break;
            }
        }
        if (!got_command) {
            wprintw(ps->command_window, "\nUnrecognized terminal command");
        }
    }
    else {
        // A possible telecommand
        if (ps->command_index > COMMAND_BUFFER_SIZE - 1) {
            ps->command_index = COMMAND_BUFFER_SIZE - 1;
            if (ps->cursor_position > 0) {
                ps->cursor_position = ps->command_index;
            }
        }
        ps->command_buffer[ps->command_index] = '\0';
        col = strlen(ps->command_prefix) + 2 + strlen(ps->command_buffer);
        wmove(ps->command_window, line, col);
        // write...
        snprintf(ps->telecommand_buffer, TCMD_BUFFER_SIZE, "%s+%s!", ps->command_prefix, ps->command_buffer);
        if (strlen(ps->command_buffer) > 0) {
            if (ps->satellite_connected) {
                write(ps->satellite_link, ps->telecommand_buffer, strlen(ps->telecommand_buffer));
            }
            else {
                wprintw(ps->command_window, "\n Not connected to satellite");
            }
        }
    }
    wprintw(ps->command_window, "\n%s> ", ps->command_prefix);
    wrefresh(ps->command_window);
    // Reset command 
    ps->command_index = 0;
    ps->cursor_position = 0;
    ps->command_buffer[0] = '\0';
    ps->command_history_index = CGSE_number_of_stored_commands() - 1;

    return 0;
}
