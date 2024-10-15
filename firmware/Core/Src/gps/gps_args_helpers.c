#include "gps/gps_args_helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/// @brief Checks if the passed string starts with the valid log prefix (LOG, UNLOG and UNLOGALL)
/// @param str The gps log command string to check.
/// @return 0 if str starts with valid log prefix, 1 otherwise.
uint8_t GPS_check_starts_with_log_prefix(const char *str) {

    // Copying the passed gps log command into a buffer for parsing
    char buffer[100];
    strncpy(buffer,str,sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';

    // Extracting the log prefix from the gps log command
    char* token = strtok(buffer, " \t\n");

    if (token == NULL) {
        return 1;
    }

    if (strncmp(token, "log", 3) == 0 || strncmp(token, "unlog", 5) == 0 || strncmp(token, "unlogall", 8) == 0) {
        return 0;  
    }

    // Error: Token does not start with the log prefix
    return 1; 
}
