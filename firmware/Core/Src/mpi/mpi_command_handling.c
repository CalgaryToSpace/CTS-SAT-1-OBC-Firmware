#include "mpi/mpi_command_handling.h"
#include "mpi/mpi_types.h"
#include "mpi/mpi_transceiver.h"
#include "uart_handler/uart_handler.h"
#include "log/log.h"
#include "debug_tools/debug_uart.h"

#include "main.h"
#include "stm32l4xx_hal.h"

#include <string.h>
#include <stdio.h>

/// @brief Timeout duration for transmit HAL call, in milliseconds.
static const uint16_t MPI_TX_TIMEOUT_DURATION_MS = 100;

/// @brief Timeout duration for receive in milliseconds. Same between bytes and at the start.
static const uint16_t MPI_RX_TIMEOUT_DURATION_MS = 2000;

volatile MPI_rx_mode_t MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;


uint8_t MPI_send_command_get_response(
    const uint8_t *bytes_to_send, const size_t bytes_to_send_len, 
    uint8_t *rx_buffer, const size_t rx_buffer_max_size, 
    uint16_t *rx_buffer_len
) {
    if (rx_buffer_max_size < (bytes_to_send_len + 1)) {
        return 8; // Not enough space in the MPI response buffer
    }

    // Stop any ongoing DMA reception
    HAL_UART_DMAStop(UART_mpi_port_handle);
    HAL_UART_AbortReceive(UART_mpi_port_handle);

    // Clear volatile buffer manually
    for (uint16_t i = 0; i < UART_mpi_buffer_len; i++) {
        UART_mpi_buffer[i] = 0;
    }

    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_DUPLEX);
    MPI_current_uart_rx_mode = MPI_RX_MODE_COMMAND_MODE;
    HAL_Delay(50);  // Stabilization delay

    // Transmit command
    const HAL_StatusTypeDef tx_status = HAL_UART_Transmit(
        UART_mpi_port_handle, bytes_to_send, bytes_to_send_len, MPI_TX_TIMEOUT_DURATION_MS);
    if (tx_status != HAL_OK) {
        MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;
        return 2; // UART TX failure
    }

    // Blocking reception starts here
    uint16_t received_len = 0;

    while (received_len < rx_buffer_max_size) {
        uint8_t byte = 0;
        const HAL_StatusTypeDef rx_status = HAL_UART_Receive(
            UART_mpi_port_handle, &byte, 1, MPI_RX_TIMEOUT_DURATION_MS);

        if (rx_status == HAL_OK) {
            rx_buffer[received_len++] = byte;
        } else if (rx_status == HAL_TIMEOUT) {
            if (received_len == 0) {
                // Timeout waiting for first byte
                LOG_message(
                    LOG_SYSTEM_MPI, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
                    "Timeout waiting for 1st byte. received_len=%u", received_len);
                DEBUG_uart_print_str("rx_buffer[0:10]: ");
                DEBUG_uart_print_array_hex(rx_buffer, 10);
                DEBUG_uart_print_str("\n");

                MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_INACTIVE);
                MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;
                return 4;
            } else {
                // No more bytes coming after partial reception
                break;
            }
        } else {
            // UART error
            MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_INACTIVE);
            MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;
            return 3;
        }
    }

    *rx_buffer_len = received_len;

    LOG_message(
        LOG_SYSTEM_MPI, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "Blocking MPI response received: rx_buffer_len=%u", *rx_buffer_len
    );

    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_INACTIVE);
    MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;

    return 0;  // Success
}


/// @brief The MPI responds to each telecommand with a response code consisting of an echo of the 
///        telecommand code byte and a status byte (either 254 or OxFE) for success or error code otherwise.
/// @param tx_buffer MPI telecommand buffer containing bytes sent
/// @param rx_buffer MPI response buffer containing bytes received
/// @param tx_buffer_size Size of the MPI response buffer
/// @return 0: MPI successfully executed telecommand, 5: MPI failed to execute telecommand, 
///         6: Invalid response from the MPI
uint8_t MPI_validate_command_response(
    const uint8_t command_code, uint8_t *rx_buffer, 
    const uint16_t rx_buffer_len
) {  
    // Ensure enough bytes were received  
    if (rx_buffer_len < 2) {
        return 17; // Error code: MPI rx buffer too small
    }

    // Verify if the MPI response echos the cmd sent
    if (command_code != rx_buffer[0]) {
        return 16; // Error code: Invalid response from the MPI
    }

    // Verify if the MPI response responds with a success byte
    const uint8_t command_status = rx_buffer[1];
    if (command_status != MPI_COMMAND_SUCCESS_RESPONSE_VALUE) {
        return 15; // Error code: MPI failed to execute command
    }

    return 0; //  MPI executed the cmd successfully
}
