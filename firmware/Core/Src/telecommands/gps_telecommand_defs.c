#include "gps/gps_executor.h"
#include "telecommands/telecommand_definitions.h"
#include "uart_handler/uart_handler.h"
#include "telecommands/eps_telecommands.h"
#include "log/log.h"
#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


/// @brief Telecommand: Enable the interrupt mode for the GPS UART line
/// @param args_str
/// - Arg 0: Toggle status for the GPS_set_uart_interrupt_state function (integer): Should be either 0 or 1
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success, > 0 error
uint8_t TCMDEXEC_gps_set_enabled(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    // Turn on power
    // TODO: Determine how to set the gps into a low power state or toggle on and off

    //Wait for a second
    HAL_Delay(1000);

    // Converting the string response to an integer
    char *endptr;
    const uint8_t toggle_status = (uint8_t) strtoul(args_str, &endptr, 10);

    // Error checking for the string conversion
    if (*endptr != '\0') {
        // Invalid input
        snprintf(response_output_buf, response_output_buf_len, "Error: Invalid argument '%s'", args_str);
        return 1;
    }

    if (toggle_status != 0){
        snprintf(response_output_buf, response_output_buf_len, "Error: Invalid value, Expected Value = 0");
        return 2;
    }


    // Transmit setup commands for the GPS
    // TODO: Verify the set up commands and add the,
    const char *gps_setup_cmds[] = {
        "unlogall thisport_30 true\n",
        "log versionb once\n"
    };


    for (size_t i = 0; i < sizeof(gps_setup_cmds) / sizeof(gps_setup_cmds[0]); i++) {

        uint8_t gps_setup_transmission_result = GPS_TCMD_transmit_log_command(gps_setup_cmds[i]);
        if (gps_setup_transmission_result != 0) {
            // Transmission error handling
            snprintf(response_output_buf, response_output_buf_len, "Error: GPS UART transmission failed");
            return 3;
        }

        // Wait for half a second
        HAL_Delay(500);
    }

    // Call GPS Uart Toggle Function
    GPS_set_uart_interrupt_state(toggle_status);
    snprintf(response_output_buf, response_output_buf_len, "GPS interrupt enable and setup completed successfully");

    return 0;
}

/// @brief Telecommand: Transmit the bestxyza log command through the GPS receiver UART line
/// @param args_str
/// - Arg 0: Time interval for bestxyza log command in seconds (integer)
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success, > 0 error
uint8_t TCMDEXEC_gps_log_bestxyza(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    // Unlogall to stop receiving other commands
    // TODO: Create and use the generic gps log command function to transmit to the GPS


    // TODO: Verify that unlogall was successful
    // Investigate the gps response and also check that the HAL transmit was successful
    // Checks for successful transmission and valid response from response ( 2 things )
    // The gps response should be "OK" - Will verify this in the lab
    
    // Extract the time interval from the args_str and append to the bestxyza log
    char *endptr;
    const uint32_t time_interval_seconds = (uint32_t) strtoul(args_str, &endptr, 10);

    // Error checking for the string conversion
    if (*endptr != '\0') {
        // Invalid input
        snprintf(response_output_buf, response_output_buf_len, "Error: Invalid time interval argument '%s'. Provide an integer", args_str);
        return 1;
    }

    // Check if the value is within bounds
    if (time_interval_seconds <= 0 && time_interval_seconds > UINT32_MAX){
        snprintf(response_output_buf, response_output_buf_len, "Error: Time interval provided out of bounds");
        return 2;
    }

    char gps_log_command [50];
    snprintf(gps_log_command,sizeof(gps_log_command), "LOG BESTXYZA ONTIME %lu", time_interval_seconds);

    LOG_message(
            LOG_SYSTEM_GPS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Log Command: %s",
            gps_log_command
        );


    // Transmit the bestxyza log command
    const uint8_t gps_transmit_result = GPS_TCMD_transmit_log_command(gps_log_command);

    if (gps_transmit_result != 0) {
        // Transmission error handling
        snprintf(response_output_buf, response_output_buf_len, "Error: GPS UART transmission failed");
        return 3;
    }
    
    // Verify that the transmission and received the "OK" response
    // - This could be done in the gps log command function.

    return 0;
}

/// @brief Telecommand: Transmit the timea log command through the GPS receiver UART line
/// @param args_str
/// - Arg 0: Time interval for timea log command in seconds (integer)
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success, > 0 error
uint8_t TCMDEXEC_gps_log_timea(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    // Unlogall to stop receiving other commands
    // TODO: Create and use the generic gps log command function to transmit to the GPS


    // TODO: Verify that unlogall was successful
    // Investigate the gps response and also check that the HAL transmit was successful
    // Checks for successful transmission and valid response from response ( 2 things )
    // The gps response should be "OK" - Will verify this in the lab
    
    // Extract the time interval from the args_str and append to the bestxyza log
    char *endptr;
    const uint32_t time_interval_seconds = (uint32_t) strtoul(args_str, &endptr, 10);

    // Error checking for the string conversion
    if (*endptr != '\0') {
        // Invalid input
        snprintf(response_output_buf, response_output_buf_len, "Error: Invalid time interval argument '%s'. Provide an integer", args_str);
        return 1;
    }

    // Check if the value is within bounds
    if (time_interval_seconds <= 0 && time_interval_seconds > UINT32_MAX){
        snprintf(response_output_buf, response_output_buf_len, "Error: Time interval provided out of bounds");
        return 2;
    }

    char gps_log_command [50];
    snprintf(gps_log_command,sizeof(gps_log_command), "LOG TIMEA ONTIME %lu", time_interval_seconds);

    LOG_message(
            LOG_SYSTEM_GPS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Log Command: %s",
            gps_log_command
        );


    // Transmit the timea log command
    const uint8_t gps_transmit_result = GPS_TCMD_transmit_log_command(gps_log_command);

    if (gps_transmit_result != 0) {
        // Transmission error handling
        snprintf(response_output_buf, response_output_buf_len, "Error: GPS UART transmission failed");
        return 3;
    }

    // Verify that the transmission and received the "OK" response
    // - This could be done in the gps log command function.

    return 0;
}