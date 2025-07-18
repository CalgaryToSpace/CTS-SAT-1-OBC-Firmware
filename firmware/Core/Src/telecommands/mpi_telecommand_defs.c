#include "telecommand_exec/telecommand_args_helpers.h"
#include "telecommands/mpi_telecommand_defs.h"
#include "mpi/mpi_command_handling.h"
#include "transforms/arrays.h"
#include "mpi/mpi_transceiver.h"
#include "littlefs/littlefs_helper.h"
#include "log/log.h"
#include "uart_handler/uart_handler.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "main.h"


/// @brief Send a configuration command & params (IF ANY) to the MPI encoded in hex
/// @param args_str 
/// - Arg 0: Hex-encoded string representing the configuration command + arguments (IF ANY) to send to the MPI, INCLUDING 'TC' (0x54 0x43)
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0: Success, >0 error code
uint8_t TCMDEXEC_mpi_send_command_get_response_hex(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {    
    // Parse hex-encoded string to bytes
    const size_t args_str_len = strlen(args_str);       // Length of input string
    const uint16_t args_bytes_size = args_str_len/2;    // Expected size of input byte array
    uint16_t args_bytes_len;                            // Variable to store the length of the converted byte array
    uint8_t args_bytes[args_bytes_size];                // Byte array to store the values of converted hex string
    const uint8_t bytes_parse_result = TCMD_extract_hex_array_arg(
        args_str, 0, args_bytes, args_bytes_size, &args_bytes_len);

    // Check for invalid arguments
    if(bytes_parse_result != 0){
        snprintf(response_output_buf, response_output_buf_len, "Invalid hex argument received");
        return 1; // Error code: Invalid input
    }

    if (args_bytes_size < 3) {
        snprintf(response_output_buf, response_output_buf_len, "MPI command insufficent command length");
        return 7; // Error code: Invalid input
    }

    // Allocate space to receive incoming MPI response.
    // Max possible MPI response buffer size allocated to 256 bytes (Considering for the telecommand echo response,
    // NOT science data. MPI command + arguments can be 7 bytes + 2^N bytes of variable payload). 
    const size_t rx_buffer_max_size = 256;          // TODO: Verify once commands are finalized with payload limits
    uint16_t rx_buffer_len = 0;                     // Length of MPI response buffer
    uint8_t rx_buffer[rx_buffer_max_size];      // Buffer to store incoming response from the MPI
    memset(rx_buffer, 0, rx_buffer_max_size);   // Initialize all elements to 0

    // Send command to MPI and receive back the response
    uint8_t cmd_response = MPI_send_command_get_response(
        args_bytes, args_bytes_len, rx_buffer, rx_buffer_max_size, &rx_buffer_len
    );

    // If no errors are found during transmission and reception from the mpi, validate the response
    if (cmd_response == 0) {
        // Validate MPI response
        cmd_response = MPI_validate_command_response(args_bytes[2], rx_buffer, rx_buffer_len);
    }

    // Send back response from the MPI (if received), Log to console for now
    if (rx_buffer_len > 0) {                    
        snprintf(
            &response_output_buf[strlen(response_output_buf)],
            response_output_buf_len - strlen(response_output_buf) - 1,
            "MPI telecommand response (%u bytes): ",
            rx_buffer_len
        );
        for (size_t i = 0; i < rx_buffer_len; i++)
        {
            snprintf(
                &response_output_buf[strlen(response_output_buf)],
                response_output_buf_len - strlen(response_output_buf) - 1,
                "%02X ", rx_buffer[i]
            );
        }
    }

    // Return response code from the MPI
    return cmd_response;
}

/// @brief Enables systems to start receiving data actively from MPI and storing using LFS.
/// @param args_str
/// - Arg 0: File name as a string
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0: Success, >0: Failure
uint8_t TCMDEXEC_mpi_enable_active_mode(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len) {
    // Get the file name from the telecommand argument
    char arg_file_name[LFS_MAX_PATH_LENGTH];
    const uint8_t parse_file_name_result = TCMD_extract_string_arg(args_str, 0, arg_file_name, sizeof(arg_file_name));
    if (parse_file_name_result != 0) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing file name arg: Error %d", parse_file_name_result);
        return 1;
    }
    
    // Enable MPI Science Mode
    const uint8_t enable_result = MPI_enable_active_mode(arg_file_name);
    if (enable_result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "MPI enable active mode Failed! Error Code: %d", enable_result);
        return enable_result;
    }

    return 0;
}

/// @brief Sets the state to not send or receive data from MPI.
/// @param args_str No args.
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0: Success, >0: Failure
uint8_t TCMDEXEC_mpi_disable_active_mode(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len) {
    const uint8_t disable_result = MPI_disable_active_mode();

    if (disable_result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "MPI disable active mode Failed! Error Code: %d", disable_result);
        return disable_result;
    }

    return 0;
}



/// @brief Sends a message over UART to the MPI.
/// @param args_str No args.
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0: Success, >0: Failure
uint8_t TCMDEXEC_mpi_demo_tx_to_mpi(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    // First, set transceiver state.
    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_MOSI);
    HAL_Delay(100);

    // Note: 1000 sends takes 500ms at 230400 baud.
    // 20000 sends takes 10s at 230400 baud.
    const uint32_t transmit_count = 1;

    // const char transmit_message[] = "Hello, MPI!\n";

    // Transmit the sync bytes, plus a square wave.
    const char transmit_message[] = {0x0c, 0xff, 0xff, 0x0c, 0x00, 0x00, 'u', 'u', 'u', 'u', 0x00};

    for (uint32_t i = 0; i < transmit_count; i++) {
        // Transmit to the MPI.
        const HAL_StatusTypeDef result = HAL_UART_Transmit(
            UART_mpi_port_handle, (uint8_t*)transmit_message, sizeof(transmit_message), 1000
        );
        if (result != HAL_OK) {
            snprintf(
                response_output_buf, response_output_buf_len,
                "HAL error during UART transmit to MPI: %d.", result);
            return 1;
        }
    }

    // Reset transceiver state.
    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_INACTIVE);

    // Send message to MPI
    snprintf(response_output_buf, response_output_buf_len, "Sent message to MPI.");
    return 0;
}

/// @brief Sends a message over UART to the MPI.
/// @param args_str
/// - Arg 0: The target mode - "MISO" (from MPI), "MOSI" (to MPI), "DUPLEX", or anything else disables it
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0: Success, >0: Failure
uint8_t TCMDEXEC_mpi_demo_set_transceiver_mode(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    if (strcasecmp(args_str, "MISO") == 0) {
        MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_MISO);
        snprintf(response_output_buf, response_output_buf_len, "Did set MPI transceiver to MISO (OBC<--MPI).");
    } else if (strcasecmp(args_str, "MOSI") == 0) {
        MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_MOSI);
        snprintf(response_output_buf, response_output_buf_len, "Did set MPI transceiver to MOSI (OBC-->MPI).");
    } else if (strcasecmp(args_str, "DUPLEX") == 0) {
        MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_DUPLEX);
        snprintf(response_output_buf, response_output_buf_len, "Did set MPI transceiver to DUPLEX (OBC<->MPI).");
    } else {
        MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_INACTIVE);
        snprintf(response_output_buf, response_output_buf_len, "Disabled MPI transceiver.");
    }
    return 0;
}
