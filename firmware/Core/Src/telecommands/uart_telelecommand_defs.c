#include "telecommands/uart_telecommand_defs.h"
#include "telecommand_exec/telecommand_args_helpers.h"

#include "uart_handler/uart_handler.h"
#include "mpi/mpi_command_handling.h"
#include "log/log.h"
#include "debug_tools/debug_uart.h"
#include "uart_handler/uart_control.h"

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

/// @brief Send arbitrary commands to a UART peripheral, and receive the response.
/// @param args_str 
/// - Arg 0: UART port name to send data to: MPI, GNSS, CAMERA, EPS (case insensitive)
/// - Arg 1: Data to be sent (bytes specified as hex - Max 5KiB buffer)
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0: Success, 1: Error parsing args, 2: Invalid uart port requested, 3: Error transmitting data, 
///         4: Error receiving data, 5: Timeout waiting for response / No response, 6: Peripheral specific error, 
///         7: Unhandled error
/// @note This function doesn't toggle the EPS power lines for peripherals. Ensure they are powered on before 
///       using this function.
/// @note The camera UART port does not support receiving data in the function. TX only.
uint8_t TCMDEXEC_uart_send_hex_get_response_hex(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    // Parse UART port argument
    char arg_uart_port_name[10] = "";
    const uint8_t uart_port_name_parse_result = TCMD_extract_string_arg(args_str, 0, arg_uart_port_name, 10);

    // Parse hex-encoded config command (string to bytes)
    uint16_t tx_buffer_len = 0; // Variable to store the length of the converted byte array
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
    // TODO: Use `UART_get_port_handle_by_name` to get the port handle by name instead of this if-statement.
    if(strcasecmp(arg_uart_port_name, "MPI") == 0) {

        // Set log source
        LOG_source = LOG_SYSTEM_MPI;
        UART_rx_buffer_size_ptr = &UART_mpi_buffer_len;

        // Transmit bytes and receive response (DMA enabled reception)
        const uint8_t MPI_tx_rx_status = MPI_send_command_get_response(
            tx_buffer, tx_buffer_len, rx_buffer, *UART_rx_buffer_size_ptr, &rx_buffer_len
        );

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
    // Note: Camera does not support receiving byte-by-byte.
    else if(strcasecmp(arg_uart_port_name, "CAMERA") == 0) {        
        // UART_handle_ptr = UART_camera_port_handle;
        // UART_rx_buffer_write_idx_ptr = &UART_camera_buffer_write_idx;
        // UART_rx_buffer = &UART_camera_dma_buffer[0];
        // UART_rx_buffer_size_ptr = &UART_camera_dma_buffer_len;
        // UART_last_write_time_ms_ptr = &UART_camera_last_write_time_ms;
        // LOG_source = LOG_SYSTEM_BOOM; // Camera is related to the Boom experiment and should hence share the log source

        // FIXME: Fix this to use blocking receive probably (see CAM_test for details)

        const HAL_StatusTypeDef transmit_status = HAL_UART_Transmit(
            UART_camera_port_handle, tx_buffer, tx_buffer_len, UART_TX_TIMEOUT_DURATION_MS
        );

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
        
        return 0;
    }

    // UART 2 (disabled), 3 & 5 use interrupt based reception. Set UART handle and buffers accordingly.
    else {
        // UART 3 Selected (GNSS)
        if(strcasecmp(arg_uart_port_name, "GNSS") == 0) {
            UART_handle_ptr = UART_gnss_port_handle;
            UART_rx_buffer_write_idx_ptr = &UART_gnss_buffer_write_idx;
            UART_rx_buffer = UART_gnss_buffer;
            UART_rx_buffer_size_ptr = &UART_gnss_buffer_len;
            UART_last_write_time_ms_ptr = &UART_gnss_last_write_time_ms;
            LOG_source = LOG_SYSTEM_GNSS;

            // Enable GNSS UART interrupt. We are now listening.
            GNSS_set_uart_interrupt_state(1);
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

        // Transmit config command to requested peripheral
        const HAL_StatusTypeDef transmit_status = HAL_UART_Transmit(
            UART_handle_ptr, tx_buffer, tx_buffer_len, UART_TX_TIMEOUT_DURATION_MS
        );

        // Record start time for response reception
        const uint32_t UART_rx_start_time_ms = HAL_GetTick();

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
            // Check if we have received up to max UART rx buffer size.
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
                // condition in the following if statement)
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
        if (UART_handle_ptr == UART_eps_port_handle) {
            UART_eps_is_expecting_data = 0;
        }
    }

    // Send back complete response (as hex), if any.
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

/// @brief Get a JSON dict of how long, in ms, SINCE each UART ISR handler was last triggered.
/// @note Uses placeholder "-99" to mean "never triggered". A value of 50 means that handler was last
///       triggered 50ms ago.
/// @note Intended for debugging and testing purposes, but safe to use in flight also.
uint8_t TCMDEXEC_uart_get_last_rx_times_json(
    const char *args_str,
    char *response_output_buf,
    uint16_t response_output_buf_len
) {
    const uint32_t now = HAL_GetTick();

    const int32_t debug_delta  = UART_telecommand_last_write_time_ms == 0 ? -99 : (int32_t)(now - UART_telecommand_last_write_time_ms);
    const int32_t mpi_delta    = UART_mpi_last_write_time_ms         == 0 ? -99 : (int32_t)(now - UART_mpi_last_write_time_ms);
    const int32_t ax100_delta  = UART_ax100_last_write_time_ms       == 0 ? -99 : (int32_t)(now - UART_ax100_last_write_time_ms);
    const int32_t camera_delta = UART_camera_last_write_time_ms      == 0 ? -99 : (int32_t)(now - UART_camera_last_write_time_ms);
    const int32_t eps_delta    = UART_eps_last_write_time_ms         == 0 ? -99 : (int32_t)(now - UART_eps_last_write_time_ms);
    const int32_t gnss_delta   = UART_gnss_last_write_time_ms        == 0 ? -99 : (int32_t)(now - UART_gnss_last_write_time_ms);

    int used = snprintf(response_output_buf, response_output_buf_len,
        "{"
        "\"debug\":%ld,"
        "\"mpi\":%ld,"
        "\"ax100\":%ld,"
        "\"camera\":%ld,"
        "\"eps\":%ld,"
        "\"gnss\":%ld"
        "}",
        debug_delta,
        mpi_delta,
        ax100_delta,
        camera_delta,
        eps_delta,
        gnss_delta
    );

    if (used >= response_output_buf_len) {
        response_output_buf[response_output_buf_len - 1] = '\0';
        return 1; // truncated
    }

    return 0; // success
}


/// @brief Set the STM32 UART peripheral's baud rate to a different value.
/// @param args_str
/// - Arg 0: UART port name to set the baud rate for: MPI, GNSS, CAMERA, EPS, AX100, DEBUG (case insensitive)
/// - Arg 1: Baud rate to set the STM32 UART peripheral to (in bits per second). Common values are 9600, 115200, 230400, etc.
/// @note This does not command the subsystem device to change its baud rate. This only updates the STM32.
/// @example If the GNSS receiver does a factory reset, its baud rate is set to a different value. This command
///        can be used to change the STM32's UART baud rate to match the GNSS receiver's default baud rate to recover it.
uint8_t TCMDEXEC_uart_set_baud_rate(
    const char *args_str,
    char *response_output_buf,
    uint16_t response_output_buf_len
) {
    // Parse UART port argument
    char arg_uart_port_name[50];
    const uint8_t uart_port_name_parse_result = TCMD_extract_string_arg(args_str, 0, arg_uart_port_name, 10);

    // Parse baud rate argument
    uint64_t new_baud_rate_u64 = 0;
    const uint8_t baud_rate_parse_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &new_baud_rate_u64);

    // Check max bound to convert to uint32_t.
    if (new_baud_rate_u64 > 921600) {
        return 99; // Error code: Invalid baud rate (too high)
    }

    const uint32_t new_baud_rate = (uint32_t)new_baud_rate_u64;

    // Check for argument parsing errors
    if(uart_port_name_parse_result != 0 || baud_rate_parse_result != 0) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error parsing uart port/baud rate arg: Arg 0 Err=%d, Arg 1 Err=%d", 
            uart_port_name_parse_result, 
            baud_rate_parse_result
        );
        return 100; // Error code: Error parsing args
    }

    // Get the UART port handle by name
    UART_HandleTypeDef *UART_handle_ptr = UART_get_port_handle_by_name(arg_uart_port_name);
    
    // Check if the UART port is valid
    if (UART_handle_ptr == NULL) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Invalid UART port requested: %s", 
            arg_uart_port_name
        );
        return 101; // Error code: Invalid UART port requested
    }

    // Set the new baud rate
    const uint8_t set_baud_rate_status = UART_set_baud_rate(UART_handle_ptr, new_baud_rate);
    
    // Check if setting the baud rate was successful
    if (set_baud_rate_status != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Failed to set baud rate for %s to %lu. UART_set_baud_rate() -> %u", 
            arg_uart_port_name,
            new_baud_rate,
            set_baud_rate_status
        );
        return set_baud_rate_status;
    }

    // Log success and return
    snprintf(
        response_output_buf, response_output_buf_len,
        "Successfully set %s baud rate to %lu.",
        arg_uart_port_name,
        new_baud_rate
    );
    return 0;
}
