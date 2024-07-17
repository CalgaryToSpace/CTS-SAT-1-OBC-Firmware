#include "telecommands/telecommand_args_helpers.h"
#include "telecommands/mpi_telecommand_defs.h"
#include "mpi/mpi_command_handling.h"
#include "transforms/arrays.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/**
 * @brief Send a configuration command & params (IF ANY) to the MPI encoded in hex
 *
 * @param args_str Hex-encoded string representing the configuration command + params (IF ANY) being sent to the MPI
 * @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
 * @param response_output_buf The buffer to write the response to
 * @param response_output_buf_len The maximum length of the response_output_buf (its size)
 * @return 0: if successful, 1: if invalid input, 2: error transmitting, 3: MPI responded with an error
 */
uint8_t TCMDEXEC_mpi_send_command_hex(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len)
{
    // Get the length of the input string
    size_t arg_str_len = strlen((const char *)args_str);

    // Invalid argument: NULL
    if (args_str == NULL)
    {
        snprintf(response_output_buf, response_output_buf_len, "No commands received. Sample MPI command: 0204, sets HV_INNER_DOME_SCAN_MODE(02) to sawtooth with beam tracking (04)");
        return 1; // Error code for null arg
    }

    // Invalid argument: Invalid hex length
    if (arg_str_len % 2 != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Invalid hex string length. Must be even.\n");
        return 2; // Error code for invalid hex input
    }

    // Invalid argument: Invalid hex character
    char *endptr;
    for (size_t i = 0; i < arg_str_len; i += 2)
    {
        // Convert two characters at a time to ensure they form a valid hex byte
        char hex_byte[3] = {args_str[i], args_str[i + 1], '\0'};
        strtol(hex_byte, &endptr, 16);
        if (*endptr != '\0')
        {
            snprintf(response_output_buf, response_output_buf_len, "Invalid character in hex string: %s\n", hex_byte);
            return 2; // Error code for invalid hex input
        }
    }

    // Allocate space to receive incoming MPI response
    const size_t mpi_cmd_response_len = 50;
    uint8_t mpi_cmd_response[mpi_cmd_response_len];        // Max possible size for an MPI command+parameters can be 7 bytes + 2^N bytes of variable payload. To account for a buffer zone, 50 bytes will be allocated
    memset(mpi_cmd_response, 0, sizeof(mpi_cmd_response)); // Initialize all elements to 0

    // Send command to MPI
    uint8_t cmd_response = MPI_send_telecommand_hex(args_str, arg_str_len, mpi_cmd_response, mpi_cmd_response_len);

    // Verify successful echo response from the mpi
    if (cmd_response != 0)
    {
        snprintf(&response_output_buf[strlen(response_output_buf)], response_output_buf_len - strlen(response_output_buf) - 1, "MPI failed to execute command. MPI echoed response code: %u\n", cmd_response);

        // Send back complete response from the MPI
        snprintf(&response_output_buf[strlen(response_output_buf)], response_output_buf_len - strlen(response_output_buf) - 1, "MPI response buffer: ");
        for (size_t i = 0; i < mpi_cmd_response_len; i++)
        {
            snprintf(&response_output_buf[strlen(response_output_buf)], response_output_buf_len - strlen(response_output_buf) - 1, "%02X", mpi_cmd_response[i]);
        }

        return 3; // Error code: MPI responded with an error
    }

    // Successful MPI response
    snprintf(response_output_buf, response_output_buf_len, "MPI successfully executed the command. MPI echoed response code: %u\n", cmd_response);
    return 0;
}