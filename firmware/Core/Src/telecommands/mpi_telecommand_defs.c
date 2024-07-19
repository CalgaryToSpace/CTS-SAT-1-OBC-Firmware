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
 * @return 0: if successful, 1: if invalid input, 2: error transmitting, 3: MPI failed to execute cmd
 */
uint8_t TCMDEXEC_mpi_send_command_hex(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len)
{
    // Get the length of the input string
    size_t args_str_len = strlen(args_str);

    // Validate command argument
    if (args_str == NULL)
    {
        snprintf(response_output_buf, response_output_buf_len, "No commands received. Sample MPI command: 0204, sets HV_INNER_DOME_SCAN_MODE(02) to sawtooth with beam tracking (04)");
        return 1; // Error code: NULL input
    }

    if (args_str_len % 2 != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Invalid hex-encoded string length. Must be even.\n");
        return 2; // Error code: Invalid hex input
    }

    // Parse hex-encoded string to bytes
    uint16_t args_bytes_size = args_str_len/2;
    uint16_t args_bytes_len;
    uint8_t args_bytes[args_bytes_size];
    uint8_t bytes_parse_result = TCMD_extract_hex_array_arg(args_str, 0, args_bytes, args_bytes_size, &args_bytes_len);

    // Check for hex parsing errors
    if(bytes_parse_result != 0){
        snprintf(response_output_buf, response_output_buf_len, "Invalid character found in hex-encoded string");
        return 2; // Error code: Invalid hex input
    }

    // Allocate space to receive incoming MPI response
    const size_t mpi_response_size = 50;
    uint8_t mpi_response[mpi_response_size];     // Max possible size for an MPI command+parameters can be 7 bytes + 2^N bytes of variable payload. To account for a buffer zone, 50 bytes will be allocated
    memset(mpi_response, 0, mpi_response_size);  // Initialize all elements to 0

    // Send command to MPI and receive back the response
    uint8_t cmd_response = MPI_send_telecommand_hex(args_bytes, args_bytes_len, mpi_response, mpi_response_size);

    // Verify successful echo response from the mpi
    if (cmd_response != 0)
    {
        // Send back complete response from the MPI incase of an error
        snprintf(response_output_buf, response_output_buf_len, "MPI failed to execute command. MPI echoed response code: %u\n", cmd_response);        
        snprintf(&response_output_buf[strlen(response_output_buf)], response_output_buf_len - strlen(response_output_buf) - 1, "MPI response buffer: ");
        for (size_t i = 0; i < mpi_response_size; i++)
        {
            snprintf(&response_output_buf[strlen(response_output_buf)], response_output_buf_len - strlen(response_output_buf) - 1, "%02X", mpi_response[i]);
        }

        return 3; // Error code: MPI failed to execute cmd
    }

    // Successful MPI response
    snprintf(response_output_buf, response_output_buf_len, "MPI successfully executed the command. MPI echoed response code: %u\n", cmd_response);
    return 0;
}