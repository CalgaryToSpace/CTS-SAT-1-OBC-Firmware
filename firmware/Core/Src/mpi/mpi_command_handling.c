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
/// @return 0: Success, 1: No bytes to send, 2: Failed UART transmission, 3: Failed UART reception, 
///			4: Timeout waiting for 1st byte from MPI
/// @note If the MPI is in "science data" mode, it will be disabled after the command is executed.
uint8_t MPI_send_telecommand_get_response(const uint8_t *bytes_to_send, const size_t bytes_to_send_len, uint8_t *MPI_rx_buffer, 
                                          const size_t MPI_rx_buffer_max_size, uint16_t *MPI_rx_buffer_len) {
    // Set the MPI transceiver to MOSI mode
    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_MOSI);
    
    // Store the original MPI mode, then set MPI to command mode.
    MPI_current_uart_rx_mode = MPI_RX_MODE_COMMAND_MODE;

    // Transmit the MPI command
    HAL_StatusTypeDef transmit_status = HAL_UART_Transmit(&huart1, bytes_to_send, bytes_to_send_len, MPI_TX_TIMEOUT_DURATION_MS);

    // Check UART transmission
    if (transmit_status != HAL_OK) {
        MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;
        return 2; // Error code: Failed UART transmission
    }

    // Set the MPI transceiver to MISO mode
    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_MISO);

    // Receive MPI response byte by byte (Note: This is done to account for potential errors from the mpi where it doesnt send back an expected response)
    HAL_StatusTypeDef receive_status = HAL_UART_Receive_DMA(&huart1, (uint8_t*) &UART_mpi_last_rx_byte, 1);
    
    // Check for UART reception errors
    if (receive_status != HAL_OK) {
        MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_INACTIVE);
        HAL_UART_DMAStop(&huart1);
        MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;
        return 3; // Error code: Failed UART reception
    }

    // Reset UART buffer write index & record start time for mpi response reception
    UART_mpi_buffer_write_idx = 0;
    const uint32_t UART_mpi_rx_start_time_ms = HAL_GetTick();

    // Receive until MPI response timesout and verify it
    while (1) {
        // Check for MPI response at least until a timout event
        if ((HAL_GetTick() - UART_mpi_rx_start_time_ms) < MPI_RX_TIMEOUT_DURATION_MS) {
            continue;
        }

        // Passing the length of the response buffer back
        *MPI_rx_buffer_len = UART_mpi_buffer_write_idx;

        // MPI hasn't sent any data and has timed out
        if ((*MPI_rx_buffer_len == 0)) {
            // Stop reception from the MPI & Reset mpi UART mode state
            MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_INACTIVE);
            HAL_UART_DMAStop(&huart1);
            MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;
            return 4; // Error code: Timeout waiting for 1st byte
        }

        // MPI has sent some data and has timed out
        else if (
            (*MPI_rx_buffer_len > 0)
            && ((HAL_GetTick() - UART_mpi_rx_last_byte_write_time_ms) > MPI_RX_TIMEOUT_DURATION_MS)
        ) {
            // Stop reception from the MPI & Reset MPI UART mode state
            MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_INACTIVE);
            HAL_UART_DMAStop(&huart1);
            MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;

            // Copy the buffer to the last received byte index & clear the UART buffer
            for (uint16_t i = 0; i < *MPI_rx_buffer_len; i++) {
                MPI_rx_buffer[i] = UART_mpi_buffer[i];
            }
            
            // Clear the buffer (memset to 0, but volatile-compatible)
            for (uint16_t i = 0; i < UART_mpi_buffer_len; i++) {
                UART_mpi_buffer[i] = 0;
            }

            // Reset UART buffer write index
            UART_mpi_buffer_write_idx = 0;

            return 0; // MPI successfully executed the telecommand
        }
    }
}

/// @brief The MPI responds to each telecommand with a response code consisting of an echo of the telecommand and a success byte (either 1 for success or 0 for fail).
/// @param MPI_tx_buffer MPI telecommand buffer containing bytes sent
/// @param MPI_rx_buffer MPI response buffer containing bytes received
/// @param MPI_tx_buffer_size Size of the MPI response buffer
/// @return 0: MPI successfully executed telecommand, 5: MPI failed to execute telecommand, 6: Invalid response from the MPI
uint8_t MPI_validate_telecommand_response(const uint8_t *MPI_tx_buffer, uint8_t *MPI_rx_buffer, const uint16_t MPI_tx_buffer_size) {
    
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
