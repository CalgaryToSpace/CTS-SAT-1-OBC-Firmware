#include "telecommands/telecommand_args_helpers.h"
#include "telecommands/mpi_telecommand_definitions.h"
#include "transforms/arrays.h"
#include "mpiCommandHandling.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/// @brief Send a command to the MPI
/// @param commandCode CommandCode to identify command being sent to the MPI
/// @param commandParam Parameter part of the command being sent to the MPI
/// @return 0 if successful, 1 if NULL input, 2 if invalid hex string, 3 if other
uint8_t TCMDEXEC_mpi_send_command_hex(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len)
{
    // Get the length of the input string
    size_t arg_str_length = strlen((const char *)args_str);

    // Invalid argument: NULL
    if (args_str == NULL)
    {
        snprintf(response_output_buf, response_output_buf_len, "No commands received. Sample MPI command: 0204, sets HV_INNER_DOME_SCAN_MODE(02) to sawtooth with beam tracking (04)");
        return 1;
    }

    // Invalid argument: Invalid hex length
    if (arg_str_length % 2 != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Invalid hex string length. Must be even.\n");
        return 2;
    }

    // Invalid argument: Invalid hex character
    char *endptr;
    for (size_t i = 0; i < arg_str_length; i += 2)
    {
        // Convert two characters at a time to ensure they form a valid hex byte
        char hex_byte[3] = {args_str[i], args_str[i + 1], '\0'};
        strtol(hex_byte, &endptr, 16);
        if (*endptr != '\0')
        {
            snprintf(response_output_buf, response_output_buf_len, "Invalid character in hex string: %s\n", hex_byte);
            return 3; // Error code for invalid character
        }
    }

    // Send command to MPI
    uint8_t commandResponse = sendTelecommandHex((uint8_t *)args_str);

    // Verify mpi response to command
    if (commandResponse == 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "MPI failed to execute command. MPI echoed: %u\n", commandResponse);
        return 3;
    }
}