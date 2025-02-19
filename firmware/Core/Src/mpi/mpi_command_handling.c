#include "mpi/mpi_command_handling.h"
#include "mpi/mpi_types.h"
#include "mpi/mpi_transceiver.h"
#include "uart_handler/uart_handler.h"

#include "main.h"
#include "stm32l4xx_hal.h"

#include <string.h>
#include <stdio.h>

/// @brief Timeout duration for transmit HAL call, in milliseconds.
static const uint16_t MPI_TX_TIMEOUT_DURATION_MS = 100;
/// @brief Timeout duration for receive in milliseconds. Same between bytes and at the start.
static const uint16_t MPI_RX_TIMEOUT_DURATION_MS = 200;

MPI_rx_mode_t MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;

/// @brief Sends commandcode+params to the MPI as bytes
/// @param bytes_to_send Buffer containing the telecommand + params (IF ANY) as hex bytes
/// @param bytes_to_send_len Size of telecommand buffer
/// @param MPI_rx_buffer Buffer to store incoming response from the MPI
/// @param MPI_rx_buffer_max_size The maximum size of the MPI response buffer
/// @param MPI_rx_buffer_len Pointer to variable that will contain the length of the populated MPI response buffer 
/// @return 0: Success, 2: Failed UART transmission, 3: Failed UART reception, 
///         4: Timeout waiting for 1st byte from MPI, 8: Not enough space in the MPI response buffer
/// @note If the MPI is in "science data" mode, it will be disabled after the command is executed.
uint8_t MPI_send_command_get_response(
    const uint8_t *bytes_to_send, const size_t bytes_to_send_len, uint8_t *MPI_rx_buffer, 
    const size_t MPI_rx_buffer_max_size, uint16_t *MPI_rx_buffer_len
) {
    // Assert: MPI_rx_buffer_max_size is >= the length of the bytes_to_send_len + 1 to receive the command echo
    if (MPI_rx_buffer_max_size < (bytes_to_send_len + 1)) return 8; // Error code: Not enough space in the MPI response buffer
    
    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_MOSI); // Set the MPI transceiver to MOSI mode
    MPI_current_uart_rx_mode = MPI_RX_MODE_COMMAND_MODE; // Set MPI to command mode.

    // Transmit the MPI command
    const HAL_StatusTypeDef transmit_status = HAL_UART_Transmit(
        UART_mpi_port_handle, bytes_to_send, bytes_to_send_len, MPI_TX_TIMEOUT_DURATION_MS);

    // Check UART transmission status
    if (transmit_status != HAL_OK) {
        MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;
        return 2; // Error code: Failed UART transmission
    }

    // Set the MPI transceiver to MISO mode
    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_MISO);

    // Clear the MPI response buffer (Note: Can't use memset because UART_mpi_buffer is Volatile)
    for (uint16_t i = 0; i < UART_mpi_buffer_len; i++) {
		UART_mpi_buffer[i] = 0;
	}

    // Reset UART interrupt buffer write index & record start time for mpi response reception
    UART_mpi_buffer_write_idx = 0;                                      
    const uint32_t UART_mpi_rx_start_time_ms = HAL_GetTick();

    // Receive MPI response byte by byte.
    // Note: This is done to account for potential errors from the MPI where it doesn't send back
    // an expected-length response.
    const HAL_StatusTypeDef receive_status = HAL_UART_Receive_DMA(
        UART_mpi_port_handle, (uint8_t*) &UART_mpi_last_rx_byte, 1);
    
    // Check for UART reception errors
    if (receive_status != HAL_OK) {
        MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_INACTIVE);
        HAL_UART_DMAStop(&huart1);
        MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;
        return 3; // Error code: Failed UART reception
    }

    // Receive until MPI response timed out
    while (1) {
        // MPI response has been received upto uart rx buffer capacity
        if (UART_mpi_buffer_write_idx >= MPI_rx_buffer_max_size) {
            break;
        }

        // Timeout before receiving the first byte from the MPI
        if (UART_mpi_buffer_write_idx == 0) {
            if((HAL_GetTick() - UART_mpi_rx_start_time_ms) > MPI_RX_TIMEOUT_DURATION_MS) {
                // Stop reception from the MPI & Reset mpi UART & transceiver mode states
                MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_INACTIVE);
                HAL_UART_DMAStop(UART_mpi_port_handle);
                MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;
                return 4; // Error code: Timeout waiting for 1st byte
            }            
        }

        // Timeout in between (or end of) receiving bytes from the MPI
        else {
            const uint32_t current_time = HAL_GetTick(); // Get current time
             if (
                (current_time > UART_mpi_last_write_time_ms) // Important seemingly-obvious safety check.
                && ((current_time - UART_mpi_last_write_time_ms) > MPI_RX_TIMEOUT_DURATION_MS)
            ) {
                *MPI_rx_buffer_len = UART_mpi_buffer_write_idx; // Set the length of the MPI response buffer
                break;
            }
        }
    }

    // Stop reception & reset MPI transceiver mode state, Set MPI UART mode state to previous state
    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_INACTIVE);
    HAL_UART_DMAStop(UART_mpi_port_handle);
    MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;

    // Log the response from the MPI
    // Copy the buffer to the last received byte index & clear the UART buffer
    for (uint16_t i = 0; i < *MPI_rx_buffer_len; i++) {
        MPI_rx_buffer[i] = UART_mpi_buffer[i];
    }

    // Reset UART buffer write index
    UART_mpi_buffer_write_idx = 0;

    // Return success
    return 0;
}

/// @brief The MPI responds to each telecommand with a response code consisting of an echo of the 
///        telecommand and a success byte (either 1 for success or 0 for fail).
/// @param MPI_tx_buffer MPI telecommand buffer containing bytes sent
/// @param MPI_rx_buffer MPI response buffer containing bytes received
/// @param MPI_tx_buffer_size Size of the MPI response buffer
/// @return 0: MPI successfully executed telecommand, 5: MPI failed to execute telecommand, 
///         6: Invalid response from the MPI
uint8_t MPI_validate_command_response(
    const uint8_t *MPI_tx_buffer, uint8_t *MPI_rx_buffer, 
    const uint16_t MPI_tx_buffer_size
) {    
    // Verify if the MPI response echos the cmd sent
    if (memcmp(MPI_tx_buffer, MPI_rx_buffer, MPI_tx_buffer_size) != 0) {
        return 6; // Error code: Invalid response from the MPI
    }

    // Verify if the MPI response responds with a success byte
    if (MPI_rx_buffer[MPI_tx_buffer_size] != 0x01) {
        return 5; // Error code: MPI failed to execute command
    }

    return 0; //  MPI executed the cmd successfully
}
