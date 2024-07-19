#include "telecommands/telecommand_args_helpers.h"
#include "telecommands/mpi_telecommand_defs.h"
#include "mpi/mpi_command_handling.h"
#include "transforms/arrays.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>


///@brief Send a configuration command & params (IF ANY) to the MPI encoded in hex
/// @param args_str 
/// - Arg 0: Hex-encoded string representing the configuration command + params (IF ANY) being sent to the MPI INCLUDING 'TC' (0x54 0x43)
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0: Success, 1: Invalid Input, 2: Failed UART transmission, 3: Failed UART reception, 4: MPI unresponsive, 5: MPI failed to execute CMD
uint8_t TCMDEXEC_mpi_send_command_hex(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len)
{
    // Parse hex-encoded string to bytes
    const size_t args_str_len = strlen(args_str);       // Length of input string
    const uint16_t args_bytes_size = args_str_len/2;    // Expected size of input byte array
    uint16_t args_bytes_len;                            // Variable to store the length of the converted byte array
    uint8_t args_bytes[args_bytes_size];                // Byte array to store the values of converted hex string
    const uint8_t bytes_parse_result = TCMD_extract_hex_array_arg(args_str, 0, args_bytes, args_bytes_size, &args_bytes_len);

    // Check invalid arguments
    if(bytes_parse_result != 0){
        snprintf(response_output_buf, response_output_buf_len, "Invalid hex argument received");
        return 1; // Error code: Invalid input
    }

    // Allocate space to receive incoming MPI response
    const size_t MPI_rx_buffer_max_size = 50;
    uint16_t MPI_rx_buffer_len;
    uint8_t MPI_rx_buffer[MPI_rx_buffer_max_size];     // Max possible size for an MPI command+parameters can be 7 bytes + 2^N bytes of variable payload. To account for a buffer zone, 50 bytes will be allocated
    memset(MPI_rx_buffer, 0, MPI_rx_buffer_max_size);  // Initialize all elements to 0

    // Send command to MPI and receive back the response
    uint8_t cmd_response = MPI_send_telecommand_get_response(args_bytes, args_bytes_len, MPI_rx_buffer, MPI_rx_buffer_max_size, &MPI_rx_buffer_len);

    // Verify successful echo response from the mpi
    if (cmd_response == 0){
        snprintf(response_output_buf, response_output_buf_len, "MPI successfully executed the command. MPI echoed response code: %u\n", MPI_rx_buffer[args_bytes_len]);
    }
    else{
        snprintf(response_output_buf, response_output_buf_len, "MPI failed to execute command. MPI echoed response code: %u\n", cmd_response);
    }

    // Send back complete response from the MPI               
    snprintf(&response_output_buf[strlen(response_output_buf)], response_output_buf_len - strlen(response_output_buf) - 1, "MPI telecommand response: ");
    for (size_t i = 0; i < MPI_rx_buffer_len; i++)
    {
        snprintf(&response_output_buf[strlen(response_output_buf)], response_output_buf_len - strlen(response_output_buf) - 1, "%02X ", MPI_rx_buffer[i]);
    }

    return cmd_response;
}