#define _GNU_SOURCE
#include "gnss_receiver/gnss_internal_drivers.h"
#include "timekeeping/timekeeping.h"
#include "log/log.h"

#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief Portable version of timegm(). Converts a struct tm (UTC) to Unix epoch time.
 *
 * This function temporarily sets the environment timezone to UTC and uses mktime(),
 * which normally interprets the struct tm as local time.
 *
 * @param tm Pointer to a struct tm that represents UTC time.
 * @return time_t The Unix epoch time, or (time_t)-1 on error.
 */
time_t portable_timegm(struct tm *tm) {
    char *tz = getenv("TZ");
    setenv("TZ", "", 1);  // Temporarily set timezone to UTC
    tzset();

    time_t t = mktime(tm);  // mktime now interprets as UTC

    if (tz)
        setenv("TZ", tz, 1); // Restore original timezone
    else
        unsetenv("TZ");
    tzset();

    return t;
}

/**
 * @brief Parses a GNSS receiver TIMEA message and converts it into Unix epoch time in milliseconds.
 *
 * The function expects a full TIMEA log string as input (e.g., starting with "#TIMEA,...").
 * It tokenizes the string, extracts UTC date and time fields, validates their integrity,
 * and converts them to a Unix timestamp in milliseconds.
 *
 * @param input_str The GNSS response string (e.g., from a TIMEA log).
 * @return uint64_t Unix timestamp in milliseconds, or >0 on failure.
 */
uint64_t GNSS_format_and_convert_to_unix_epoch(char* input_str) {
    
    if (!input_str) return 1;

    // Create a safe, modifiable copy of the input
    char copy_of_input_str[512];
    strncpy(copy_of_input_str, input_str, sizeof(copy_of_input_str));
    copy_of_input_str[sizeof(copy_of_input_str) - 1] = '\0';

    // Tokenize input using both comma and semicolon as delimiters
    char* tokens[25];  
    int count = 0;
    char* token = strtok(copy_of_input_str, ",;");
    while (token != NULL && count < 25) {
        tokens[count++] = token;
        token = strtok(NULL, ",;");
    }

    // Extract UTC date/time components
    // TODO: Might be better to use tokens[x] instead of tokens[count - y], Note: check if 8-13 works
    int year       = atoi(tokens[count - 7]);
    int month      = atoi(tokens[count - 6]);
    int day        = atoi(tokens[count - 5]);
    int hour       = atoi(tokens[count - 4]);
    int minute     = atoi(tokens[count - 3]);
    int millisecond= atoi(tokens[count - 2]);
    char* utc_status = tokens[count - 1];

    // Clean up any trailing newlines in the status field
    char* newline = strchr(utc_status, '\n');
    if (newline) *newline = '\0';

    // Reject invalid UTC status
    if (strcmp(utc_status, "VALID") != 0) {
        return 1;  // UTC time is not valid
    }

    int second = millisecond / 1000;
    millisecond %= 1000;

    struct tm t = {0};
    t.tm_year = year - 1900;
    t.tm_mon  = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min  = minute;
    t.tm_sec  = second;

    // Convert to Unix epoch time (UTC)
    time_t epoch_seconds = portable_timegm(&t);
    if (epoch_seconds == -1) return 1;

    // Return Unix timestamp in milliseconds
    return (uint64_t)epoch_seconds * 1000 + millisecond;
}

/**
 * @brief Sets the OBC's time, based on the GNSS receiver's current time.
 *
 * This function sends a command to the GNSS receiver to fetch the current time,
 * parses the TIMEA response to extract UTC time, converts it to Unix epoch time,
 * and sets the system clock accordingly.
 *
 * @return 0 on success, >0 on failure.
 */
uint8_t GNSS_set_obc_time_based_on_gnss_time() {

    // Initialize GNSS command "log timea once"
    const char *full_command = "log timea once";
    const uint8_t full_command_len = strlen(full_command);

    // The following buffer will have data written into from the response from GNSS Transmitter
    const size_t rx_buffer_max_size = 512;
    uint16_t rx_buffer_len = 0;
    uint8_t rx_buffer[rx_buffer_max_size];
    memset(rx_buffer, 0, rx_buffer_max_size);

    // Send the command to the GNSS receiver to get UTC time (and other data)
    const uint8_t gnss_cmd_response = GNSS_send_cmd_get_response(
        full_command, full_command_len, rx_buffer, rx_buffer_max_size, &rx_buffer_len);

    // Error check to make sure we've even received a response from the GNSS receiver
    if (gnss_cmd_response != 0 ) {
        LOG_message(
            LOG_SYSTEM_GPS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
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
    const uint64_t formatted_time = GNSS_format_and_convert_to_unix_epoch(response_str);

    // Error check to make sure GNSS_format_and_convert_to_unix_epoch executed successfully
    if (formatted_time == 1) {
        LOG_message(
            LOG_SYSTEM_GPS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Failed to parse GNSS TIMEA response: %s", response_str
        );
        return 1;
    }

    // Set the system time based on GNSS time
    TIM_set_current_unix_epoch_time_ms(
        formatted_time,
        TIM_SOURCE_GNSS
    );
    return 0;
}
