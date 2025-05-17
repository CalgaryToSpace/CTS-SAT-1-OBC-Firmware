#define _GNU_SOURCE
#include "gps/gps_internal_drivers.h"
#include "timekeeping/timekeeping.h"

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
 * @brief Parses a GPS TIMEA message and converts it into Unix epoch time in milliseconds.
 *
 * The function expects a full TIMEA log string as input (e.g., starting with "#TIMEA,...").
 * It tokenizes the string, extracts UTC date and time fields, validates their integrity,
 * and converts them to a Unix timestamp in milliseconds.
 *
 * @param input_str The GPS response string (e.g., from a TIMEA log).
 * @return uint64_t Unix timestamp in milliseconds, or 0 on failure.
 */
uint64_t GPS_format_and_convert_to_unix_epoch(char* input_str) {
    
    if (!input_str) return 0;

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
        token = strtok(NULL, ",");
    }

    // Extract UTC date/time components
    // TODO: Might be better to use tokens[x] instead of tokens[count - y]
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
        return 0;  // UTC time is not valid
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
    if (epoch_seconds == -1) return 0;

    // Return Unix timestamp in milliseconds
    return (uint64_t)epoch_seconds * 1000 + millisecond;
}

/**
 * @brief Sets the OBC's time, based on the GPS's current time.
 *
 * This function sends a command to the GPS receiver to fetch the current time,
 * parses the TIMEA response to extract UTC time, converts it to Unix epoch time,
 * and sets the system clock accordingly.
 *
 * @return 0 on success, >0 on failure.
 */
uint8_t GPS_set_obc_time_based_on_gps_time() {

    // TODO: Does this need to be a global variable? Or is it okay being locally scoped.
    const char *full_command = "log timea once";
    const uint8_t full_command_len = strlen(full_command);

    // The following buffer will have data written into from the response from GNSS Transmitter
    const uint16_t GPS_rx_buffer_max_size = 512;
    uint16_t GPS_rx_buffer_len = 0;
    uint8_t GPS_rx_buffer[GPS_rx_buffer_max_size];
    memset(GPS_rx_buffer, 0, GPS_rx_buffer_max_size);

    // Send the command to the GPS receiver to get UTC time (and other data)
    const uint8_t gps_cmd_response = GPS_send_cmd_get_response(
        full_command, full_command_len, GPS_rx_buffer, GPS_rx_buffer_max_size, &GPS_rx_buffer_len);

    // Error check to make sure we've even received a response from the GPS receiver
    if (gps_cmd_response != 0 ) {
        // TODO: Do we need to log this error? If so, we need to pass in buffers to this function
        return 1;
    }

    // Null termination of the received buffer
    if (GPS_rx_buffer_len < GPS_rx_buffer_max_size) {
        GPS_rx_buffer[GPS_rx_buffer_len] = '\0';  // Ensure null-termination
    } else {
        GPS_rx_buffer[GPS_rx_buffer_max_size - 1] = '\0';  // Prevent overflow
    }

    // Make response buffer a string
    char* response_str = (char*)GPS_rx_buffer;

    // Function to parse AND FORMAT the GPS response (add unit test) -> will return timestamp value
    const uint64_t formatted_time = GPS_format_and_convert_to_unix_epoch(response_str);

    // TODO: Uncomment the following call `TIM_set_current_unix_epoch_time_ms` after GPS_format_and_convert_to_unix_epoch is finished
    TIM_set_current_unix_epoch_time_ms(
        formatted_time,
        TIM_SOURCE_GNSS
    );

    return 0;
}
