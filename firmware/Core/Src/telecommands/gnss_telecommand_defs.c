#include "telecommand_exec/telecommand_definitions.h"
#include "uart_handler/uart_handler.h"
#include "telecommands/eps_telecommands.h"
#include "gnss_receiver/gnss_internal_drivers.h"
#include "log/log.h"
#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/// @brief Telecommand: Transmit a log command to the GNSS receiver through UART
/// @param args_str
/// - Arg 0: Log command to be sent to GNSS eg "log bestxyza once" (string)
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success, > 0 error
uint8_t TCMDEXEC_gnss_send_cmd_ascii(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
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
        &rx_buffer_len // Will be mutated.
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
        "GNSS Response (%d bytes): %s",
        rx_buffer_len,
        rx_buffer
    );

    return gnss_cmd_status;
}