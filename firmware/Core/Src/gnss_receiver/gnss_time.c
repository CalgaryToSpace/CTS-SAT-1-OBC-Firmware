#include "gnss_receiver/gnss_internal_drivers.h"
#include "timekeeping/timekeeping.h"
#include "log/log.h"
#include "uart_handler/uart_handler.h"

#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

#include "main.h"

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
uint8_t GNSS_set_obc_time_based_on_gnss_time_uart() {
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
            LOG_SYSTEM_GNSS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
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
            LOG_SYSTEM_GNSS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Failed to parse GNSS TIMEA response: %s", response_str
        );
        return 2;
    }

    // Set the system time based on GNSS time
    TIME_set_current_unix_epoch_time_ms(
        formatted_time,
        TIME_SYNC_SOURCE_GNSS_UART
    );
    
    return 0;
}


/// @brief Get the state of the GNSS PPS pin.
/// @return 0 if low (nominal state), 1 if high (very short pulse).
static inline uint8_t GNSS_get_pps_pin_state() {
    return HAL_GPIO_ReadPin(PIN_GNSS_PPS_IN_GPIO_Port, PIN_GNSS_PPS_IN_Pin);
}


/// @brief Modify the already-synced OBC time to move the time by up to 499ms to align with the GNSS PPS.
/// @return 0 on success. 1 on initial validation failure (bad PPS enabling).
///         5 on adjustment timeout. 10 on PPS command failure.
/// @note Related Spec: https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/503
uint8_t GNSS_set_obc_time_based_on_gnss_pps() {
    // Run "PPSCONTROL ENABLE NEGATIVE" to get a normally-high-active-low pulse signal.
    // Default behaviour of OEM7 is a 1-second period with 1000us (1ms) pulse width.
    // Source: Page 305-306 of https://docs.novatel.com/OEM7/Content/PDFs/OEM7_Commands_Logs_Manual.pdf.
    const uint32_t max_polling_time_ms = 5000;

    // Enable PPS mode.
    const char *pps_command = "PPSCONTROL ENABLE NEGATIVE\r\n";
    const HAL_StatusTypeDef tx_status = HAL_UART_Transmit(
        UART_gnss_port_handle,
        (uint8_t *)pps_command, strlen(pps_command),
        100
    );
    if (tx_status != HAL_OK) {
        LOG_message(
            LOG_SYSTEM_GNSS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "GNSS PPS command failed (UART tx_status=%d)", tx_status
        );
        // Streamroll. If GNSS control is having issues (very unlikely), we could try to enable PPS
        // via a different command.
    }

    HAL_Delay(250); // Arbitrary delay, in case it takes the OEM7 a sec.

    // Validate that the PPS pin is high (indicating it's enabled correctly).
    const uint32_t start_time_validation_phase = HAL_GetTick();
    while ((HAL_GetTick() - start_time_validation_phase) < 250) {
        if (GNSS_get_pps_pin_state() == 0) {
            // If it's low, then wait a sec, and check again. Ensures we don't confuse a legit PPS
            // signal for a validation failure.
            HAL_Delay(10);
            if (GNSS_get_pps_pin_state() == 0) {
                LOG_message(
                    LOG_SYSTEM_GNSS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "PPS pin validation failed"
                );
                return 1; // Error: PPS pin validation failed.
            }
        }
        
        // Delay here to significantly decrease the likelihood of the GPIO validation check
        // coinciding with a legit PPS pulse. We check several times over 250ms.
        HAL_Delay(10);
    }

    // Begin the adjustment.
    const uint32_t start_time_adjustment_phase = HAL_GetTick();
    while (HAL_GetTick() - start_time_adjustment_phase < max_polling_time_ms) {
        if (GNSS_get_pps_pin_state() == 0) {
            // Nominal case. It is at this point precisely that we must
            // round the current OBC time to the nearest 1000ms (nearest top of a second).
            const uint64_t current_time_ms = TIME_get_current_unix_epoch_time_ms();
            const uint64_t rounded_time_ms = (current_time_ms + 500) / 1000 * 1000;
            TIME_set_current_unix_epoch_time_ms(rounded_time_ms, TIME_SYNC_SOURCE_GNSS_PPS);

            return 0; // Nominal success exit.
        }
    }

    LOG_message(
        LOG_SYSTEM_GNSS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
        "GNSS PPS adjustment timeout"
    );
    return 5; // Error: Adjustment timeout.
}
