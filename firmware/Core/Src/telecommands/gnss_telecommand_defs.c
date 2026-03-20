#include "telecommand_exec/telecommand_definitions.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "uart_handler/uart_handler.h"
#include "telecommands/eps_telecommands.h"
#include "gnss_receiver/gnss_internal_drivers.h"
#include "gnss_receiver/gnss_firehose_storage.h"
#include "log/log.h"
#include "littlefs/littlefs_helper.h"
#include "main.h"
#include "transforms/arrays.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/// @brief Send a command to the GNSS and receive the response as ASCII.
/// @param args_str
/// - Arg 0: Log command to be sent to GNSS eg "log bestxyza once" (string)
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success, > 0 error
uint8_t TCMDEXEC_gnss_send_cmd_ascii(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    // Adding a new line character to the log command
    char gnss_log_cmd[128];
    snprintf(gnss_log_cmd, sizeof(gnss_log_cmd), "%s\n", args_str);
    const uint16_t gnss_log_cmd_len = strlen(gnss_log_cmd);

    // Allocate space to receive incoming GNSS response.
    const uint16_t rx_buffer_max_size = 512;
    uint16_t rx_buffer_len = 0;
    uint8_t rx_buffer[rx_buffer_max_size];
    memset(rx_buffer, 0, rx_buffer_max_size); // Initialize all elements to 0

    // Send log command to GNSS and receive response
    const uint8_t gnss_cmd_status = GNSS_send_cmd_get_response(
        gnss_log_cmd, gnss_log_cmd_len, rx_buffer, rx_buffer_max_size,
        &rx_buffer_len, // Will be mutated.
        1 // Remove null bytes in the middle.
    );

    // Handle the gnss_cmd_status: Perform the error checks
    // TODO: Potentially add GNSS_validate_log_response function in here to validate response from the gnss receiver

    if (gnss_cmd_status != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "GNSS_send_cmd_get_response failed -> %d",
            gnss_cmd_status
        );
    }

    snprintf(
        response_output_buf, response_output_buf_len,
        "GNSS Response (%d chars): %s",
        rx_buffer_len,
        rx_buffer
    );

    return gnss_cmd_status;
}


/// @brief Send a command to the GNSS (text) and receive the response as hex.
/// @param args_str
/// - Arg 0: Log command to be sent to GNSS eg "log bestxyzb once" (string)
/// @param response_output_buf The buffer to write the response to (output as hex)
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success, > 0 error
uint8_t TCMDEXEC_gnss_send_cmd_ascii_get_response_hex(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    // Adding a new line character to the log command
    char gnss_log_cmd[128];
    snprintf(gnss_log_cmd, sizeof(gnss_log_cmd), "%s\n", args_str);
    const uint16_t gnss_log_cmd_len = strlen(gnss_log_cmd);

    // Allocate space to receive incoming GNSS response.
    const uint16_t rx_buffer_max_size = 512;
    uint16_t rx_buffer_len = 0;
    uint8_t rx_buffer[rx_buffer_max_size];
    memset(rx_buffer, 0, rx_buffer_max_size); // Initialize all elements to 0

    // Send log command to GNSS and receive response
    const uint8_t gnss_cmd_status = GNSS_send_cmd_get_response(
        gnss_log_cmd, gnss_log_cmd_len, rx_buffer, rx_buffer_max_size,
        &rx_buffer_len, // Will be mutated.
        0 // KEEP null bytes in the response.
    );

    // Handle the gnss_cmd_status: Perform the error checks
    // TODO: Potentially add GNSS_validate_log_response function in here to validate response from the gnss receiver

    if (gnss_cmd_status != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "GNSS_send_cmd_get_response failed -> %d",
            gnss_cmd_status
        );
    }

    GEN_byte_array_to_hex_str(
        rx_buffer, rx_buffer_len,
        response_output_buf, response_output_buf_len
    );

    return 0; // Success.
}


/// @brief Enables storing data from GNSS into a file.
/// @param args_str
/// - Arg 0: File name as a string
/// @return 0: Success, >0: Failure
/// @details This feature is meant to be used in combination with the "log bestxyza ontime <seconds>" GNSS commands.
/// @example Enable power, enable "ontime" logs in the GNSS (using gnss_send_cmd_ascii), then enable this mode.
/// @note This telecommand does not control EPS power. You must enable the GNSS power first.
uint8_t TCMDEXEC_gnss_enable_firehose_storage_mode(
    const char *args_str, char *response_output_buf, uint16_t response_output_buf_len
) {
    // Get the file name from the telecommand argument.
    char arg_file_name[LFS_MAX_PATH_LENGTH];
    const uint8_t parse_file_name_result = TCMD_extract_string_arg(args_str, 0, arg_file_name, sizeof(arg_file_name));
    if (parse_file_name_result != 0) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing file name arg: Error %d", parse_file_name_result);
        return 1;
    }
    
    // Enable GNSS firehose mode (store to file).
    const uint8_t enable_result = GNSS_enable_firehose_storage_mode(arg_file_name);
    if (enable_result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "GNSS firehose storage enable failed! Error Code: %d", enable_result
        );
        return enable_result;
    }

    return 0;
}

/// @brief Disables the GNSS firehose storage mode (closes the firehose file).
/// @param args_str No args.
/// @return 0: Success, >0: Failure
/// @note This telecommand does not control EPS power. You must disable the GNSS power afterwards.
uint8_t TCMDEXEC_gnss_disable_firehose_storage_mode(
    const char *args_str, char *response_output_buf, uint16_t response_output_buf_len
) {
    const uint8_t disable_result = GNSS_disable_firehose_storage_mode("TCMD");

    if (disable_result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "GNSS firehose storage disable failed! Error Code: %d", disable_result
        );
        return disable_result;
    }

    return 0;
}
