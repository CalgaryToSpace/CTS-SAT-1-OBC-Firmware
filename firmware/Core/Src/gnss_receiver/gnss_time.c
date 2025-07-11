#include "gnss_receiver/gnss_internal_drivers.h"
#include "timekeeping/timekeeping.h"
#include "log/log.h"

#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

/// @brief Parses a GNSS receiver TIMEA message and converts it into Unix epoch time in milliseconds.
/// The function expects a full TIMEA log string as input (e.g., starting with "#TIMEA,...").
/// It tokenizes the string, extracts UTC date and time fields, validates their integrity,
/// and converts them to a Unix timestamp in milliseconds.
/// @param input_str The GNSS response string (e.g., from a TIMEA log).
/// @return uint8_t returning 0 for success and 1 for error.
uint8_t GNSS_parse_timea_response_and_convert_to_unix_time_ms(char* input_str, uint64_t* unix_time_ms) {
    
    // Null pointer check
    if (!input_str) {
        return 1;
    }

    // Create a safe, modifiable copy of the input
    char copy_of_input_str[512];
    strncpy(copy_of_input_str, input_str, sizeof(copy_of_input_str));
    copy_of_input_str[sizeof(copy_of_input_str) - 1] = '\0';

    // Tokenize input using both comma and semicolon as delimiters
    char* tokens[25];  
    uint8_t count = 0;
    char* token = strtok(copy_of_input_str, ",;*");
    while (token != NULL && count < 25) {
        tokens[count++] = token;
        token = strtok(NULL, ",;*");
    }

    // Ensure at least 21 tokens to safely access tokens[20]
    if (count < 21) {
        return 1;
    }

    // Extract UTC date/time components
    const int32_t year         = atoi(tokens[14]);
    const int32_t month        = atoi(tokens[15]);
    const int32_t day          = atoi(tokens[16]);
    const int32_t hour         = atoi(tokens[17]);
    const int32_t minute       = atoi(tokens[18]);
    const int32_t milliseconds = atoi(tokens[19]);
    const char* utc_status = tokens[20];
    
    // Reject invalid UTC status
    if (strcmp(utc_status, "VALID") != 0) {
        return 1;  // UTC time is not valid
    }

    struct tm t = {0};
    t.tm_year = year - 1900;
    t.tm_mon  = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min  = minute;
    t.tm_sec  = 0;

    // Convert to Unix epoch time (UTC)
    const time_t epoch_seconds = mktime(&t);
    if (epoch_seconds == -1) {
        return 1;
    }
    
    // Combine seconds and milliseconds
    *unix_time_ms = (uint64_t)epoch_seconds * 1000 + milliseconds;

    return 0;
}

/// @brief Sets the OBC's time, based on the GNSS receiver's current time.
/// This function sends a command to the GNSS receiver to fetch the current time,
/// parses the TIMEA response to extract UTC time, converts it to Unix epoch time,
/// and sets the system clock accordingly.
/// @return 0 on success, >0 on failure.
uint8_t GNSS_set_obc_time_based_on_gnss_time() {

    // Initialize GNSS command "log timea once"
    const char *full_command = "log timea once";
    const uint8_t full_command_len = strlen(full_command);

    // The following buffer will have data written into from the response from GNSS Transmitter
    const uint16_t rx_buffer_max_size = 230; // The timea response is ~169 characters such that added a bit extra to the unit test string case
    uint16_t rx_buffer_len = 0;
    uint8_t rx_buffer[rx_buffer_max_size];
    memset(rx_buffer, 0, rx_buffer_max_size);

    // Send the command to the GNSS receiver to get UTC time (and other data)
    const uint8_t gnss_cmd_response = GNSS_send_cmd_get_response(
        full_command, full_command_len, rx_buffer, rx_buffer_max_size, &rx_buffer_len);

    // Error check to make sure we've even received a response from the GNSS receiver
    if (gnss_cmd_response != 0 ) {
        LOG_message(
            LOG_SYSTEM_GNSS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "GNSS TIMEA request failed (cmd_response=%u)", gnss_cmd_response
        );
        return 1;
    }

    // Null termination of the received buffer
    if (rx_buffer_len < rx_buffer_max_size) {
        rx_buffer[rx_buffer_len] = '\0';  // Ensure null-termination
    } else {
        rx_buffer[rx_buffer_max_size - 1] = '\0';  // Prevent overflow
    }

    // Parse and convert GNSS time string to epoch
    char* response_str = (char*)rx_buffer;
    uint64_t formatted_time = 0;
    const uint8_t parse_status = GNSS_parse_timea_response_and_convert_to_unix_time_ms(response_str, &formatted_time);

    // Error check to make sure GNSS_parse_timea_response_and_convert_to_unix_time_ms executed successfully
    if (parse_status != 0) {
        LOG_message(
            LOG_SYSTEM_GNSS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Failed to parse GNSS TIMEA response: %s", response_str
        );
        return 2;
    }

    // Set the system time based on GNSS time
    TIME_set_current_unix_epoch_time_ms(
        formatted_time,
        TIME_SYNC_SOURCE_GNSS
    );
    
    return 0;
}
