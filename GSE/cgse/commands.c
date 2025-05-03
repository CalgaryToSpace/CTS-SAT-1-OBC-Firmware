#include "commands.h"
#include "command_history.h"
#include "command_queue.h"
#include "main.h"
#include "base64.h"
#include "terminal.h"

#include "telecommands/telecommand_definitions.h"

#include <stdio.h>
#include <inttypes.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_ssize_t.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <errno.h>

#include <ncurses.h>

extern volatile sig_atomic_t running;

const CGSE_command_t CGSE_terminal_commands[] = {
    {".quit", ".quit", "quit the terminal", CGSE_terminal_quit, false},
    {".help", ".help", "show available terminal commands", CGSE_terminal_help, false},
    {".connect", ".connect [<device-name>]", "connect to the satellite, optionally using <device-path>", CGSE_terminal_connect, false},
    {".disconnect", ".disconnect", "disconnect from from satellite", CGSE_terminal_disconnect, false},
    {".show_timestamp", ".show_timestamp", "show GSE computer timestamp on received messages", CGSE_terminal_show_timestamp, false},
    {".hide_timestamp", ".hide_timestamp", "show GSE computer timestamp on received messages", CGSE_terminal_hide_timestamp, false},
    {".sync_time", ".sync_time", "synchronize satellite time with whit computer's time", CGSE_terminal_sync_timestamp, false},
    {".telecommands", ".telecommands", "list telecommands", CGSE_terminal_list_telecommands, false},
    {".list_queued_commands", ".list_queued_commands", "list queued telecommands and terminal commands", CGSE_terminal_list_queued_commands, false},
    {".reload_command_queue", ".reload_command_queue", "reload command queue from default and optional files", CGSE_terminal_reload_command_queue, false},
    {".ls", ".ls", "list current directory", CGSE_terminal_list_current_directory, false},
    {".upload_mpi_firmware", ".upload_mpi_firmware <file_name>", "upload MPI firmware from <file_name> relative to the current directory", CGSE_terminal_upload_mpi_firmware, false},
};
const int CGSE_NUM_TERMINAL_COMMANDS = sizeof(CGSE_terminal_commands) / sizeof(CGSE_command_t);

int CGSE_terminal_quit(CGSE_program_state_t *ps, const char *cmd_string) 
{
    running = 0;

    return 0;
};

int CGSE_terminal_help(CGSE_program_state_t *ps, const char *cmd_string) 
{
    command_window_print(ps, "Available commands:");
    for (int c = 0; c < CGSE_NUM_TERMINAL_COMMANDS; c++) {
        const CGSE_command_t *cmd = &CGSE_terminal_commands[c];
        command_window_print(ps, "%35s - %s", cmd->help_template, cmd->description);
    }
    // TODO show help for telecommands

    return 0;
}

int CGSE_terminal_connect(CGSE_program_state_t *ps, const char *cmd_string)
{
    if (ps->satellite_connected) {
        CGSE_terminal_disconnect(ps, cmd_string);
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
        command_window_print(ps, "Error opening satellite link. Check umbilical or radio is plugged into the ground station computer");
        return -1;
    }

    struct termios sat_link_params;
    int result = tcgetattr(sat_link, &sat_link_params);
    if (result != 0) {
        command_window_print(ps, "Error setting satellite link parameters");
        return -2;
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
        command_window_print(ps, "Error setting satellite link parameters");
        ps->line += 1;
        return -3;
    }
    tcflush(sat_link, TCIOFLUSH);
    
    update_link_status(ps);
    ps->satellite_link = sat_link;
    ps->satellite_connected = true;

    return 0;
}

int CGSE_terminal_disconnect(CGSE_program_state_t *ps, const char *cmd_string) 
{
    if (ps->satellite_link > 0) {
        close(ps->satellite_link);
        ps->satellite_link = 0;
        ps->satellite_connected = false;
    }

    return 0;
}

int CGSE_terminal_show_timestamp(CGSE_program_state_t *ps, const char *cmd_string) 
{
    ps->prepend_timestamp = true;
    return 0;
}

int CGSE_terminal_hide_timestamp(CGSE_program_state_t *ps, const char *cmd_string) 
{
    ps->prepend_timestamp = false;
    return 0;
}

int CGSE_terminal_sync_timestamp(CGSE_program_state_t *ps, const char *cmd_string) 
{
    char tcmd[256];
    struct timeval epoch = {0};
    int gtod_result = gettimeofday(&epoch, NULL);
    if (gtod_result != 0) {
        command_window_print(ps, "Unable to get the time");
        return -1;
    }
    uint64_t epoch_ms = (uint64_t)epoch.tv_sec * 1000 + epoch.tv_usec/1000;
    // TODO account for a calibrated delay in
    // communicating with the satellite
    snprintf(tcmd, 256, "%s+set_system_time(%"PRIu64")!", ps->command_prefix, epoch_ms);
    if (ps->satellite_connected) {
        int bytes_sent = write(ps->satellite_link, tcmd, strlen(tcmd));
        if (bytes_sent <= 0) {
            command_window_print(ps, "Error sending telecommand");
        }
    }
    else {
        command_window_print(ps, "Not connected to satellite");
    }

    return 0;
}

int CGSE_terminal_list_telecommands(CGSE_program_state_t *ps, const char *cmd_string) 
{
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
        wprintw(ps->command_window, ")");
        // Trigger newline and scroll
        command_window_print(ps, "\n"); 
    }

    return 0;
}

int CGSE_terminal_list_current_directory(CGSE_program_state_t *ps, const char *cmd_string) 
{
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

int CGSE_terminal_list_queued_commands(CGSE_program_state_t *ps, const char *cmd_string) 
{
    CGSE_command_queue_list_commands(ps);
    return 0;
}

int CGSE_terminal_reload_command_queue(CGSE_program_state_t *ps, const char *cmd_string) 
{
    CGSE_load_command_queue(ps);
    return 0;
}

int CGSE_terminal_upload_mpi_firmware(CGSE_program_state_t *ps, const char *cmd_string) 
{
    if (!ps->satellite_connected) {
        command_window_print(ps, "Not connected to satellite");
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
            command_window_print(ps, "Unable to load firmware from %s", mpi_firmware_path);
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
                    command_window_print(ps, "Sent MPI firmware page %d (total %d of %lu bytes)", mpi_firmware_page, mpi_firmware_bytes_sent, mpi_firmware_length);
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
                        command_window_print(ps, "Did not receive reply. Aborting firmware upload...");
                        break;
                    }
                }
                else {
                    // otherwise abort (or add this page to the
                    // failed-list and try those pages later?)
                    command_window_print(ps, "Failed to send firmware page %d. Aborting...", mpi_firmware_page);
                    break;
                }

                // Check for interrupt...
                int mpi_key = 0;
                mpi_key = wgetch(ps->command_window);
                if (mpi_key == 'q') {
                    command_window_print(ps, "Upload interrupted by user.");
                    wrefresh(ps->command_window);
                    break;
                }
            }
        }

        // All done
        free(mpi_firmware);
    }
    else {
        command_window_print(ps, "Unable to parse command.");
    }
    free(buf);

    wclrtoeol(ps->command_window);
    return 0;
}

char * CGSE_get_args_from_str(const char* args, int *nargs, char **arg_vector)
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

int CGSE_execute_command(CGSE_program_state_t *ps, const char *command)
{

    size_t buffer_len = strlen(command);
    if (buffer_len == 0) {
        return 0;
    }

    // Store the command 
    CGSE_store_command(command);
    ps->command_history_index = CGSE_number_of_stored_commands() - 1;

    // check for known terminal command
    if (command[0] == '.') {
        bool known_terminal_command = false;
        for (int c = 0; c < CGSE_NUM_TERMINAL_COMMANDS; c++) {
            const CGSE_command_t *cmd = &CGSE_terminal_commands[c];
            if (strncmp(command, cmd->name, strlen(cmd->name)) == 0) {
                // Execute the terminal command
                int cmd_status = cmd->function(ps, command);
                known_terminal_command = true;
                break;
            }
        }
        if (!known_terminal_command) {
            command_window_print(ps, "Unrecognized terminal command");
        }
    }
    else if (CGSE_is_valid_telecommand(command)) {
        snprintf(ps->telecommand_buffer, TCMD_BUFFER_SIZE, "%s+%s!", ps->command_prefix, command);
        if (buffer_len > 0) {
            if (ps->satellite_connected) {
                ssize_t write_result = write(ps->satellite_link, ps->telecommand_buffer, strlen(ps->telecommand_buffer));
                if (write_result < 0) {
                    command_window_print(ps, "Error sending command: %s", strerror(errno));
                }
            }
            else {
                command_window_print(ps, "Not connected to satellite");
            }
        }
    }
    else {
        command_window_print(ps, "Unrecognized telecommand");
    }

    return 0;
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
        command_window_print(ps, "queue-> %s", e->command_text);
        CGSE_execute_command(ps, e->command_text);
        commandline_redraw(ps);
        CGSE_command_queue_remove_next();

        gettimeofday(&tv, NULL);
        t2 = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
    }

    return;
}

bool CGSE_is_valid_telecommand(const char *command)
{
    if (strlen(command) == 0) {
        return false;
    }

    // TODO other checks for telecommand validity

    return true;
}

