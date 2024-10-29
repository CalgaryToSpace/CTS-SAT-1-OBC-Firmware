#include "telecommands/telecommand_args_helpers.h"
#include "telecommands/mpi_telecommand_defs.h"
#include "mpi/mpi_command_handling.h"
#include "transforms/arrays.h"
#include "mpi/mpi_transceiver.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "main.h"


/// @brief Send a configuration command & params (IF ANY) to the MPI encoded in hex
/// @param args_str 
/// - Arg 0: Hex-encoded string representing the configuration command + arguments (IF ANY) to send to the MPI, INCLUDING 'TC' (0x54 0x43)
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0: Success, 1: Invalid Input, 2: Failed UART transmission, 3: Failed UART reception,
///         4: MPI timeout before sending 1 byte, 5: MPI failed to execute CMD, 6: Invalid response from the MPI
uint8_t TCMDEXEC_mpi_send_command_hex(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len) {
    
    // Parse hex-encoded string to bytes
    const size_t args_str_len = strlen(args_str);       // Length of input string
    const uint16_t args_bytes_size = args_str_len/2;    // Expected size of input byte array
    uint16_t args_bytes_len;                            // Variable to store the length of the converted byte array
    uint8_t args_bytes[args_bytes_size];                // Byte array to store the values of converted hex string
    const uint8_t bytes_parse_result = TCMD_extract_hex_array_arg(args_str, 0, args_bytes, args_bytes_size, &args_bytes_len);

    // Check for invalid arguments
    if(bytes_parse_result != 0){
        snprintf(response_output_buf, response_output_buf_len, "Invalid hex argument received");
        return 1; // Error code: Invalid input
    }

    // Allocate space to receive incoming MPI response.
    // Max possible MPI response buffer size allocated to 256 bytes (Considering for the telecommand echo response,
    // NOT science data. MPI command + arguments can be 7 bytes + 2^N bytes of variable payload). 
    const size_t MPI_rx_buffer_max_size = 256;          // TODO: Verify once commands are finalized with payload limits
    uint16_t MPI_rx_buffer_len = 0;                     // Length of MPI response buffer
    uint8_t MPI_rx_buffer[MPI_rx_buffer_max_size];      // Buffer to store incoming response from the MPI
    memset(MPI_rx_buffer, 0, MPI_rx_buffer_max_size);   // Initialize all elements to 0

    // Send command to MPI and receive back the response
    uint8_t cmd_response = MPI_send_telecommand_get_response(args_bytes, args_bytes_len, MPI_rx_buffer, MPI_rx_buffer_max_size, &MPI_rx_buffer_len);

    // If no errors are found during transmission and reception from the mpi, validate the response
    if(cmd_response == 0) {
        // Validate MPI response
        cmd_response = MPI_validate_telecommand_response(args_bytes, MPI_rx_buffer, MPI_rx_buffer_len-1);
    }

    // Send back MPI response log detail
    switch(cmd_response) {
        case 0: 
            snprintf(response_output_buf, response_output_buf_len, "MPI successfully executed the command. MPI echoed response code: %u\n", MPI_rx_buffer[args_bytes_len]);
            break;
        case 2: 
            snprintf(response_output_buf, response_output_buf_len, "Failed UART transmission.\n");
            break;
        case 3: 
            snprintf(response_output_buf, response_output_buf_len, "Failed UART reception.\n");
            break;
        case 4: 
            snprintf(response_output_buf, response_output_buf_len, "Timeout waiting for 1st byte from MPI.\n");
            break;
        case 5: 
            snprintf(response_output_buf, response_output_buf_len, "Timeout after receiving bytes from MPI.\n");
            break;
        case 6:
            snprintf(response_output_buf, response_output_buf_len, "MPI failed to execute telecommand.  MPI echoed response code: %u\n", cmd_response);
            break;
        case 7:
            snprintf(response_output_buf, response_output_buf_len, "Invalid response from the MPI.  MPI echoed response code: %u\n", cmd_response);
            break;
        default:
            snprintf(response_output_buf, response_output_buf_len, "Unknown MPI_send_telecommand_get_response() return: %u\n", cmd_response);
            break;
    }

    // Send back response from the MPI (if received), Log to console for now
    // TODO: Change after testing to meet new requirements
    if(MPI_rx_buffer_len > 0) {                    
        snprintf(
            &response_output_buf[strlen(response_output_buf)],
            response_output_buf_len - strlen(response_output_buf) - 1,
            "MPI telecommand response (%u bytes): ",
            MPI_rx_buffer_len
        );
        for (size_t i = 0; i < MPI_rx_buffer_len; i++)
        {
            snprintf(
                &response_output_buf[strlen(response_output_buf)],
                response_output_buf_len - strlen(response_output_buf) - 1,
                "%02X ", MPI_rx_buffer[i]
            );
        }
    }

    // Return response code from the MPI
    return cmd_response;
}


/// @brief Sends a message over UART to the MPI.
/// @param args_str No args.
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0: Success, >0: Failure
uint8_t TCMDEXEC_mpi_demo_tx_to_mpi(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    // First, set transceiver state.
    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_MOSI);
    HAL_Delay(100); // TODO: Confirm amount of delay required.

    const uint16_t transmit_count = 1;

    // Note: 1000 sends takes 500ms at 230400 baud.
    for (uint16_t i = 0; i < transmit_count; i++) {
        // Transmit to the MPI.
        const HAL_StatusTypeDef result = HAL_UART_Transmit(&huart1, (uint8_t*)"Hello, MPI!\n", strlen("Hello, MPI!\n"), HAL_MAX_DELAY);
        if (result != HAL_OK) {
            snprintf(response_output_buf, response_output_buf_len, "HAL error during UART transmit to MPI: %d.", result);
            return 1;
        }
    }

    // Reset transceiver state.
    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_INACTIVE);

    // Send message to MPI
    snprintf(response_output_buf, response_output_buf_len, "Sent message to MPI.");
    return 0;
}
