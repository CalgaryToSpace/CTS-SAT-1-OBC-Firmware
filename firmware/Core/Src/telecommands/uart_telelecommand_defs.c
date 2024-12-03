#include "telecommands/uart_telecommand_defs.h"
#include "telecommands/telecommand_args_helpers.h"

#include "uart_handler/uart_handler.h"
#include "mpi/mpi_command_handling.h"
#include "log/log.h"
#include "debug_tools/debug_uart.h"

#include <string.h>
#include <stdio.h>

/// @brief Timeout duration for transmit HAL call, in milliseconds.
static const uint16_t UART_TX_TIMEOUT_DURATION_MS = 200;
/// @brief Timeout duration for receive in milliseconds. Same between bytes and at the start.
static const uint16_t UART_RX_TIMEOUT_DURATION_MS = 300;

// Allocate 5KiB of space for send and receive arrays
const uint16_t tx_buffer_max_size = 5120;
const uint16_t rx_buffer_max_size = 5120;
static uint8_t tx_buffer[5120];
static uint8_t rx_buffer[5120];

/// @brief Send artibrary configuration commands to a UART peripheral
/// @param args_str 
/// - Arg 0: UART port name to send data to: MPI, LORA, GPS, CAMERA, EPS (case insensitive)
/// - Arg 1: Data to be sent (bytes specified as hex - Max 5KiB buffer)
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0: Success, 1: Error parsing args, 2: Invalid uart port requested, 3: Error transmitting data, 
///         4: Error receiving data, 5: Timeout waiting for response / No response, 6: Peripheral specific error, 
///         7: Unhandled error
/// @note This function doesn't toggle the EPS power lines for peripherals. Ensure they are powered on before 
///       using this function.
uint8_t TCMDEXEC_uart_send_bytes_hex(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    // Parse UART port argument
    char arg_uart_port_name[10] = "";
    const uint8_t uart_port_name_parse_result = TCMD_extract_string_arg(args_str, 0, arg_uart_port_name, 10);

    // Parse hex-encoded config command (string to bytes)
    uint16_t tx_buffer_len = 0;                             // Variable to store the length of the converted byte array
    const uint8_t bytes_to_send_parse_result = TCMD_extract_hex_array_arg(args_str, 1, tx_buffer, tx_buffer_max_size, &tx_buffer_len);

    // Check for argument parsing errors
    if(uart_port_name_parse_result != 0 || bytes_to_send_parse_result !=0) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error parsing uart port/data to send arg: Arg 0 Err=%d, Arg 1 Err=%d", 
            uart_port_name_parse_result, 
            bytes_to_send_parse_result
        );
        return 1;    // Error code: Error parsing args
    }
    
    // Allocate space to receive incoming response      
    uint16_t rx_buffer_len = 0;                                
    memset(rx_buffer, 0, rx_buffer_max_size);                   

    // Assign pointers to UART reception variables to unify the interface (Interrupt case only in this function)
    UART_HandleTypeDef *UART_handle_ptr;                        
    volatile uint16_t *UART_rx_buffer_write_idx_ptr;            
    volatile uint8_t *UART_rx_buffer;
    const uint16_t *UART_rx_buffer_size_ptr;
    volatile uint32_t *UART_last_write_time_ms_ptr;

    LOG_system_enum_t LOG_source = LOG_SYSTEM_TELECOMMAND;
    
    // UART 1 selected (MPI)
    if(strcasecmp(arg_uart_port_name, "MPI") == 0) {

        // Set log source
        LOG_source = LOG_SYSTEM_MPI;
        UART_rx_buffer_size_ptr = &UART_mpi_buffer_len;

        // Transmit bytes and receive response (DMA enabled reception)
        const uint8_t MPI_tx_rx_status = MPI_send_telecommand_get_response(tx_buffer, tx_buffer_len, rx_buffer, *UART_rx_buffer_size_ptr, &rx_buffer_len);

        // Log successful transmission and print rx_buffer response in hex (uint8_t)
        char rx_buffer_str[rx_buffer_len*3];

        for(uint16_t i = 0; i < rx_buffer_len; i++) {
            // Copy received bytes into a string to log it
            sprintf(&rx_buffer_str[i*3], "%02X ", rx_buffer[i]);
        }

        // Log mpi response
        switch(MPI_tx_rx_status) {
            case 0:
                LOG_message(
                    LOG_source, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
                    "Transmitted %u byte(s) to %s.", 
                    tx_buffer_len, 
                    arg_uart_port_name
                );
                LOG_message(
                    LOG_source, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
                    "Received %u byte(s) from %s: %s",
                    rx_buffer_len,
                    arg_uart_port_name,
                    rx_buffer_str
                );
                return 0;   // Success
            case 2:
                LOG_message(
                    LOG_source, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "Failed UART transmission."
                );
                return 3;   // Error code: Error transmitting data
            case 3:
                LOG_message(
                    LOG_source, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "Failed UART reception."
                );
                return 4;   // Error code: Error receiving data
            case 4:
                LOG_message(
                    LOG_source, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "No response received. Timeout waiting for 1st byte."
                ); 
                return 5;   // Error code: Error receiving data
            case 8:
                LOG_message(
                    LOG_source, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "Not enough space in the MPI response buffer"
                ); 
                return 6;   // Error code: Error receiving data
            default:
                LOG_message(
                    LOG_source, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "Unknown response return: %u", 
                    MPI_tx_rx_status
                );
                return 7;   // Error code: Unhandled error
        }
    }

    // UART 4 Selected (CAMERA)
    else if(strcasecmp(arg_uart_port_name, "CAMERA") == 0) {        
        UART_handle_ptr = UART_camera_port_handle;
        UART_rx_buffer_write_idx_ptr = &UART_camera_buffer_write_idx;
        UART_rx_buffer = &UART_camera_buffer[0];
        UART_rx_buffer_size_ptr = &UART_camera_buffer_len;
        UART_camera_is_expecting_data = 1;
        UART_last_write_time_ms_ptr = &UART_camera_last_write_time_ms;
        LOG_source = LOG_SYSTEM_BOOM; // Camera is related to the Boom experiment and should hence share the log source

        // TODO: DMA handling function for camera is still under development.
        // Return error code for now to indicate that the UART handling is not yet implemented. Remove this 
        // once the UART handling is implemented for the camera. 
        // Transmit the MPI command for now to test UART line
        const HAL_StatusTypeDef transmit_status = HAL_UART_Transmit(UART_camera_port_handle, tx_buffer, tx_buffer_len, UART_TX_TIMEOUT_DURATION_MS);

        // Check UART transmission status
        if (transmit_status != HAL_OK) {
            MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;
            return 2; // Error code: Failed UART transmission
        }

        // Log successful transmission
        LOG_message(
            LOG_source, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Transmitted %u byte(s) to %s.", 
            tx_buffer_len, 
            arg_uart_port_name
        );
        LOG_message(
            LOG_source, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Transmitted bytes to camera. The Camera UART reception is not yet implemented."
        );
        
        return 2;
    }

    // UART 2,3 & 5 use interrupt based reception, set UART handle and buffers according to selection
    else {

        // UART 2 Selected (LORA)
        if(strcasecmp(arg_uart_port_name, "LORA") == 0) {
            UART_handle_ptr = UART_lora_port_handle;
            UART_rx_buffer_write_idx_ptr = &UART_lora_buffer_write_idx;
            UART_rx_buffer = UART_lora_buffer;
            UART_rx_buffer_size_ptr = &UART_lora_buffer_len;
            UART_lora_is_expecting_data = 1;
            UART_last_write_time_ms_ptr = &UART_lora_last_write_time_ms;
            // TODO: LORA is not a system log source, Currently set to TELECOMMAND for the time being
            LOG_source = LOG_SYSTEM_TELECOMMAND; 
        }

        // UART 3 Selected (GPS)
        else if(strcasecmp(arg_uart_port_name, "GPS") == 0) {
            UART_handle_ptr = UART_gps_port_handle;
            UART_rx_buffer_write_idx_ptr = &UART_gps_buffer_write_idx;
            UART_rx_buffer = UART_gps_buffer;
            UART_rx_buffer_size_ptr = &UART_gps_buffer_len;
            UART_last_write_time_ms_ptr = &UART_gps_last_write_time_ms;
            LOG_source = LOG_SYSTEM_GPS;
        }

        // UART 5 Selected (EPS)
        else if(strcasecmp(arg_uart_port_name, "EPS") == 0) {
            UART_handle_ptr = UART_eps_port_handle;
            UART_rx_buffer_write_idx_ptr = &UART_eps_buffer_write_idx;
            UART_rx_buffer = UART_eps_buffer;
            UART_rx_buffer_size_ptr = &UART_eps_buffer_len;
            UART_eps_is_expecting_data = 1;
            UART_last_write_time_ms_ptr = &UART_eps_last_write_time_ms;
            LOG_source = LOG_SYSTEM_EPS;
        }

        // Invalid UART selected
        else {
            LOG_message(
                LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
                "Invalid UART peripheral port requested: %s", 
                arg_uart_port_name
            );
            return 2;   // Error code: Invalid UART port requested
        }

        // Reset UART buffer write index
        *UART_rx_buffer_write_idx_ptr = 0;

        // Clear the response buffer (Note: Can't use memset because UART_rx_buffer is Volatile)
        for (uint16_t i = 0; i < *UART_rx_buffer_size_ptr; i++) {
            UART_rx_buffer[i] = 0;
        }

        // Record start time for response reception
        const uint32_t UART_rx_start_time_ms = HAL_GetTick();

        // Transmit config command to requested peripheral
        const HAL_StatusTypeDef transmit_status = HAL_UART_Transmit(UART_handle_ptr, tx_buffer, tx_buffer_len, UART_TX_TIMEOUT_DURATION_MS);

        // Check UART transmission
        if (transmit_status != HAL_OK) {
            LOG_message(
                LOG_source, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
                "Failed to transmit %u bytes to %s UART port. HAL status: %d", 
                tx_buffer_len, 
                arg_uart_port_name,
                transmit_status
            );
            return 3; // Error code: Error transmitting data
        }

        // Log successful transmission
        LOG_message(
            LOG_source, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Transmitted %u byte(s) to %s.",
            tx_buffer_len, 
            arg_uart_port_name
        );

        // Receive from peripheral until a timeout event
        while (1) {
            
            // Check if we have received upto max UART rx buffer size
            if (*UART_rx_buffer_write_idx_ptr >= *UART_rx_buffer_size_ptr) {
                break;
            }

            // Check if we have timed out (Before receiving the first byte)
            if (*UART_rx_buffer_write_idx_ptr == 0) {
                if((HAL_GetTick() - UART_rx_start_time_ms) > UART_RX_TIMEOUT_DURATION_MS) {
                    LOG_message(
                        LOG_source, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                        "No response received from %s. Timeout waiting for 1st byte.",
                        arg_uart_port_name
                    );

                    return 4; // Error code: Timeout waiting for 1st byte
                }            
            }

            // We have received some data but timed out (in between bytes / end of reception)
            else {
                const uint32_t current_time = HAL_GetTick(); // Get current time

                // Get last write time (Required to dereference volatile pointer here to prevent race 
                // condition in the folllowing if statement)
                const uint32_t last_write_time = *UART_last_write_time_ms_ptr;  

                // Check if we have timed out while receiving bytes
                if (
                    (current_time > last_write_time) // Important seemingly-obvious safety check.
                    && ((current_time - last_write_time) > UART_RX_TIMEOUT_DURATION_MS)
                ) {
                    rx_buffer_len = *UART_rx_buffer_write_idx_ptr; // Set the length of the response buffer
                    break;
                }
            }
        }

        // Log the response received from peripheral
        // Copy the buffer to the last received byte index & clear the UART buffer
        for (uint16_t i = 0; i < rx_buffer_len; i++) {
            rx_buffer[i] = UART_rx_buffer[i];
            UART_rx_buffer[i] = 0;
        }

        // Reset UART buffer write index                
        *UART_rx_buffer_write_idx_ptr = 0;

        // Reset listening flags if used by peripheral (interrupt reception mode case is only handled here)
        if(UART_handle_ptr == UART_lora_port_handle){
            UART_lora_is_expecting_data = 0;
        }
        else if(UART_handle_ptr == UART_eps_port_handle){
            UART_eps_is_expecting_data = 0;
        }
    }

    // Send back complete response if any
    if(rx_buffer_len > 0) { 

        // Convert rx_buffer to a string for logging
        // 2 hex chars, 1 space, +1 for null terminator
        size_t rx_buffer_str_len = rx_buffer_len*3+1;
        char rx_buffer_str[rx_buffer_str_len];

        for(uint16_t i = 0; i < rx_buffer_len; i++) {
            // Format each byte as hex with a trailing space
            sprintf(&rx_buffer_str[i*3], "%02X ", rx_buffer[i]); 
        }

        // Null terminator at the end of the string
        rx_buffer_str[rx_buffer_len*3] = '\0';

        // Format log header and log response in chunks
        size_t log_buffer_offset = 0;
        size_t log_message_max_length = LOG_FORMATTED_MESSAGE_MAX_LENGTH-1; // -1 for null terminator

        // Log header at the start of the log message
        LOG_message(
            LOG_source, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Received %u byte(s) from %s. Response in Hex:",
            rx_buffer_len,
            arg_uart_port_name
        ); 

        while(log_buffer_offset < rx_buffer_str_len-1) {

            // Calculate the remaining space in the buffer for the log message
            size_t log_buffer_remaining_space = rx_buffer_str_len - 1 - log_buffer_offset;
            // The length of the log chunk is the smaller of the remaining space or the maximum log message length
            size_t chunk_len = (rx_buffer_str_len-1 - log_buffer_offset) < log_message_max_length ? log_buffer_remaining_space : log_message_max_length;
            
            // Extract the response chunk to log
            char log_message[chunk_len+1];
            strncpy(log_message, rx_buffer_str + log_buffer_offset, chunk_len);
            log_message[chunk_len] = '\0';

            // Log the log message chunk
            LOG_message(
                LOG_source, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
                "%s",
                log_message
            );

            // Move the offset forward by the chunk size
            log_buffer_offset += chunk_len;
        }  
    }

    return 0;   // Success
}
