#include "telecommands/uart_telecommand_defs.h"
#include "telecommands/telecommand_args_helpers.h"

#include "uart_handler/uart_handler.h"
#include "mpi/mpi_command_handling.h"
#include "log/log.h"

#include <string.h>
#include <stdio.h>

#define TX_TIMEOUT_DURATION_MS 100	// Timeout duration for transmit in milliseconds
#define RX_TIMEOUT_DURATION_MS 200  // Timeout duration for reception in milliseconds

// Allocate 5KiB of space for send and receive arrays
const uint16_t tx_buffer_max_size = 5120;
const uint16_t rx_buffer_max_size = 5120;
static uint8_t tx_buffer[5120];
static uint8_t rx_buffer[5120];

/// @brief Send artibrary configuration commands to a peripheral
/// @param args_str 
/// - Arg 0: UART port to send data to: MPI, LORA, GPS, CAMERA, EPS (case insensitive)
/// - Arg 1: Data to be sent (bytes specified as hex - Max 5KiB buffer)
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0: Success, 1: Error parsing args, 2: Invalid uart port requested, 3: Error transmitting data, 
///         4: Error receiving data, 5: Timeout waiting for response / No response, 6: Unhandled error
/// @note This function doesn't support toggling the EPS to send power to the peripherals
/// @todo Add support to send data to the camera once it's supporting functions are implemented
uint8_t TCMDEXEC_uart_send_bytes(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    // Parse UART port argument
    char arg_uart_port_name[10] = "";
    const uint8_t uart_port_name_parse_result = TCMD_extract_string_arg(args_str, 0, arg_uart_port_name, 10);

    // Parse hex-encoded config command (string to bytes)
    uint16_t tx_buffer_len = 0;                             // Store the length of the converted byte array
    const uint8_t bytes_to_send_parse_result = TCMD_extract_hex_array_arg(args_str, 1, tx_buffer, tx_buffer_max_size, &tx_buffer_len);

    // Check for parsing errors
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
    uint16_t rx_buffer_len = 0;                             // Length of rx buffer
    memset(rx_buffer, 0, rx_buffer_max_size);               // Initialize all elements to 0

    // Assign UART Handler variables to handle reception according to selected peripheral (Interrupt case only in this function)
    UART_HandleTypeDef *UART_handle;                        // Selected UART handle
    volatile uint16_t *UART_rx_buffer_write_idx_ptr;        // Pointer to write index for selected UART rx buffer
    // const uint16_t *UART_rx_buffer_len_ptr;                 // Pointer to the length of the selected UART rx buffer //TODO: Need to implement a check to make sure the UART handler buffers have the capacity needed
    volatile uint8_t *UART_rx_buffer;                       // Pointer to selected UART rx buffer
    LOG_system_enum_t LOG_source = LOG_SYSTEM_TELECOMMAND;  // Slected system log source

    // UART 1 selected (MPI)
    if(strcmp(arg_uart_port_name, "MPI") == 0) {

        // UART1 is used by the MPI. We need to store the original MPI mode, then set MPI to command mode
        MPI_current_uart_rx_mode = MPI_RX_MODE_COMMAND_MODE;

        // Set log source
        LOG_source = LOG_SYSTEM_MPI;

        // Transmit bytes and receive response (DMA enabled reception)
        const uint8_t MPI_tx_rx_status = MPI_send_telecommand_get_response(tx_buffer, tx_buffer_len, rx_buffer, rx_buffer_max_size, &rx_buffer_len);

        // Log successful transmission and print rx_buffer response in hex (uint8_t)
        char rx_buffer_str[rx_buffer_len*3];

        for(uint16_t i = 0; i < rx_buffer_len; i++) {
            sprintf(&rx_buffer_str[i*3], "%02X ", rx_buffer[i]);
        }

        // Send response log detail
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
            default:
                LOG_message(
                    LOG_source, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "Unknown response return: %u", 
                    MPI_tx_rx_status
                );
                return 6;   // Error code: Unhandled error
        }
    }

    // UART 4 Selected (CAMERA)
    else if(strcasecmp(arg_uart_port_name, "CAMERA") == 0) {
        // TODO: Implement once, DMA handling function for camera is added
        UART_handle = UART_camera_port_handle;
        UART_rx_buffer_write_idx_ptr = &UART_camera_buffer_write_idx;
        // UART_rx_buffer_len_ptr = &UART_camera_buffer_len;    // TODO: Need to implement a check to make sure the UART handler buffers have the capacity needed
        UART_rx_buffer = &UART_camera_buffer[0];
        UART_camera_is_expecting_data = 1;
        LOG_source = LOG_SYSTEM_TELECOMMAND;                    // TODO: CAMERA is not a system log source, Currently set to TELECOMMAND for the time being
    }

    // UART 2,3 & 5 use interrupt based reception, set UART handle and buffers according to selection
    else {

        // UART 2 Selected (LORA)
        if(strcasecmp(arg_uart_port_name, "LORA") == 0) {
            UART_handle = UART_lora_port_handle;
            UART_rx_buffer_write_idx_ptr = &UART_lora_buffer_write_idx;
            // UART_rx_buffer_len_ptr = &UART_lora_buffer_len;  // TODO: Need to implement a check to make sure the UART handler buffers have the capacity needed
            UART_rx_buffer = UART_lora_buffer;
            UART_lora_is_expecting_data = 1;
            LOG_source = LOG_SYSTEM_TELECOMMAND;                // TODO: LORA is not a system log source, Currently set to TELECOMMAND for the time being
        }

        // UART 3 Selected (GPS)
        else if(strcasecmp(arg_uart_port_name, "GPS") == 0) {
            UART_handle = UART_gps_port_handle;
            UART_rx_buffer_write_idx_ptr = &UART_gps_buffer_write_idx;
            // UART_rx_buffer_len_ptr = &UART_gps_buffer_len;   // TODO: Need to implement a check to make sure the UART handler buffers have the capacity needed
            UART_rx_buffer = UART_gps_buffer;
            LOG_source = LOG_SYSTEM_GPS;
        }

        // UART 5 Selected (EPS)
        else if(strcasecmp(arg_uart_port_name, "EPS") == 0) {
            UART_handle = UART_eps_port_handle;
            UART_rx_buffer_write_idx_ptr = &UART_eps_buffer_write_idx;
            // UART_rx_buffer_len_ptr = &UART_eps_buffer_len;   // TODO: Need to implement a check to make sure the UART handler buffers have the capacity needed
            UART_rx_buffer = UART_eps_buffer;
            UART_eps_is_expecting_data = 1;
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

        // Transmit config command to requested peripheral
        HAL_StatusTypeDef transmit_status = HAL_UART_Transmit(UART_handle, tx_buffer, tx_buffer_len, TX_TIMEOUT_DURATION_MS);

        // Record start time for response reception
        const uint32_t UART_rx_last_write_time_ms = HAL_GetTick();

        // Check UART transmission
        if (transmit_status != HAL_OK) {
            LOG_message(
                LOG_source, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
                "Failed to transmit %u bytes to %s UART port.", 
                tx_buffer_len, 
                arg_uart_port_name
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
        while(1) {

            // Check for response at least until a timout event
            if ((HAL_GetTick() - UART_rx_last_write_time_ms) < RX_TIMEOUT_DURATION_MS) {
                continue;
            }

            // Timeout without receiving any data
            if (*UART_rx_buffer_write_idx_ptr == 0) { 
                LOG_message(
                    LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "No response received. Timeout waiting for 1st byte."
                );

                return 5;   // Error code: Timout waiting for first byte
            }

            // Timeout after receiving some data
            else if ((HAL_GetTick() - UART_rx_last_write_time_ms > RX_TIMEOUT_DURATION_MS) && (*UART_rx_buffer_write_idx_ptr > 0)) {
                
                rx_buffer_len = *UART_rx_buffer_write_idx_ptr;

                // Copy the UART buffer to the last received byte index & clear the UART buffer
                for (uint16_t i = 0; i < rx_buffer_len; i++) {
                    // Copy only until the max size of rx buffer
                    if(i == rx_buffer_max_size){
                        break;
                    }

                    rx_buffer[i] = UART_rx_buffer[i];
                }

                // Reset UART buffer write index                
                *UART_rx_buffer_write_idx_ptr = 0;
                
                // Reset the UART buffer (memset to 0, but volatile-compatible)
                for (uint16_t i = 0; i < rx_buffer_len; i++) {
                    UART_rx_buffer[i] = 0;
                }

                // Reset listening flags if used by peripheral (interrupt reception mode case is only handled here)
                if(UART_handle == UART_lora_port_handle){
                    UART_lora_is_expecting_data = 0;
                }
                else if(UART_handle == UART_eps_port_handle){
                    UART_eps_is_expecting_data = 0;
                }

                // Reception complete break out of loop
                break; 
            }
        }
    }

    // Send back complete response if received
    if(rx_buffer_len > 0) { 

        // Convert rx_buffer to a string for logging
        char rx_buffer_str[rx_buffer_len*3];                        

        for(uint16_t i = 0; i < rx_buffer_len; i++) {
            sprintf(&rx_buffer_str[i*3], "%02X ", rx_buffer[i]);
        }

        // Log received response in Hex formatting
        LOG_message(
            LOG_source, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Received %u byte(s) from %s: %s",
            rx_buffer_len,
            arg_uart_port_name,
            rx_buffer_str
        );        
    }

    return 0;   // Success
}
