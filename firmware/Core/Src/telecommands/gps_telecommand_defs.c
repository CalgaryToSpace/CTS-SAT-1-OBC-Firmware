#include "telecommand_exec/telecommand_definitions.h"
#include "uart_handler/uart_handler.h"
#include "telecommands/eps_telecommands.h"
#include "gps/gps_internal_drivers.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "log/log.h"
#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/// @brief Telecommand: Transmit a log command to the GPS receiver through UART
/// @param args_str
/// - Arg 0: Log command to be sent to GPS eg "log bestxyza once" (string)
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success, > 0 error
uint8_t TCMDEXEC_gps_send_cmd_ascii(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    // Adding a new line character to the log command
    char gps_log_cmd[128];
    snprintf(gps_log_cmd, sizeof(gps_log_cmd), "%s\n", args_str);
    const uint16_t gps_log_cmd_len = strlen(gps_log_cmd);

    // Allocate space to receive incoming GPS response.
    const uint16_t GPS_rx_buffer_max_size = 512;
    uint16_t GPS_rx_buffer_len = 0;
    uint8_t GPS_rx_buffer[GPS_rx_buffer_max_size];
    memset(GPS_rx_buffer, 0, GPS_rx_buffer_max_size); // Initialize all elements to 0

    // Send log command to GPS and receive response
    const uint8_t gps_cmd_status = GPS_send_cmd_get_response(
        gps_log_cmd, gps_log_cmd_len, GPS_rx_buffer, GPS_rx_buffer_max_size,
        &GPS_rx_buffer_len // Will be mutated.
    );

    // Handle the gps_cmd_status: Perform the error checks
    // TODO: Potentially add GPS_validate_log_response function in here to validate response from the gps receiver

    if (gps_cmd_status != 0) {
        LOG_message(
            LOG_SYSTEM_GPS,
            LOG_SEVERITY_NORMAL,
            LOG_SINK_ALL,
            "GPS_send_cmd_get_response failed -> %d",
            gps_cmd_status
        );
    }

    snprintf(
        response_output_buf, response_output_buf_len,
        "GPS Response (%d bytes): %s",
        GPS_rx_buffer_len,
        GPS_rx_buffer
    );

    return 0;
}

/// @brief Telecommand: Enable/disable specific a function of the GPS receiver through UART
/// @param args_str separated by comma (eg: ANTENNAPOWER,0)
/// - Arg 0: Log command to be sent to GPS to enable/disable
/// - Arg 1: 1 to enable (power on), 0 to disable (power off)
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success, > 0 error
uint8_t TCMDEXEC_gps_enable_disable_command(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len)
{
    // Extract Arg 0: command to send
    char command_arg[128];
    memset(command_arg, 0, sizeof(command_arg));
    // will include file
    const uint8_t arg_0_result = TCMD_extract_string_arg(args_str, 0, command_arg, sizeof(command_arg));
    if (arg_0_result > 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Could not parse arg 0 for: %s", args_str);
        return 1;
    }

    if (strcmp(command_arg, "ANTENNAPOWER") != 0 && strcmp(command_arg, "TRACKSIGNAL") != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error: Invalid command (arg 0) from telecommand: '%s'", args_str);
        return 2;
    }

    // Extract Arg 1: 1 to enable (power on), 0 to disable (power off)
    uint64_t enable_disable_flag = 69;
    const uint8_t arg_1_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &enable_disable_flag);
    if (arg_1_result != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Could not parse arg 1 for: %s", args_str);
        return 1;
    }

    if (enable_disable_flag != 0 && enable_disable_flag != 1)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error: Invalid enable/disable flag (arg 1) from telecommand: %s", args_str);
        return 1;
    }

    // Allocate space to receive incoming GPS response.
    const uint16_t GPS_rx_buffer_max_size = 512;
    uint16_t GPS_rx_buffer_len = 0;
    uint8_t GPS_rx_buffer[GPS_rx_buffer_max_size];
    memset(GPS_rx_buffer, 0, GPS_rx_buffer_max_size); // Initialize all elements to 0

    // Send log command to GPS and receive response
    const uint8_t gps_response = GPS_enable_disable(command_arg, enable_disable_flag, GPS_rx_buffer, GPS_rx_buffer_len, GPS_rx_buffer_max_size);

    if (gps_response != 0)
    {
        // make more specific
        LOG_message(
            LOG_SYSTEM_GPS,
            LOG_SEVERITY_NORMAL,
            LOG_SINK_ALL,
            "Error: GPS enable/disable command failed with code: %d",
            gps_response
        );
        
        snprintf(response_output_buf, response_output_buf_len, "Error: GPS enable/disable command failed with code %d", gps_response);
        return 1;
    }
        
    // TODO: modify this returned snprintf string to be more specific
    snprintf(response_output_buf, response_output_buf_len, "GPS command: '%s' successfully transmitted", args_str);

    return 0;
}