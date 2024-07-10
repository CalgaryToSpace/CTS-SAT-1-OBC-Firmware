#include "mpi/mpi_command_handling.h"
#include "mpi/mpi_types.h"
#include "debug_tools/debug_uart.h"
#include "uart_handler/uart_handler.h"
#include "main.h"
#include "stm32l4xx_hal.h"
#include "string.h"
#include <stdio.h>

#define TX_TIMEOUT_DURATION_MS 100	// Timeout duration for transmit in milliseconds
#define RX_TIMEOUT_DURATION_MS 100	// Timeout duration for receive in milliseconds

MPI_rx_mode_t current_mpi_mode = NOT_LISTENING_TO_MPI_MODE;

/// @brief Sends commandcode+params to the MPI as bytes
/// @param bytes_to_send Buffer containing the telecommand + params (IF ANY) as hex bytes
/// @param bytes_to_send_len Size of telecommand buffer
/// @param MPI_rx_buffer Buffer to store incoming response from the MPI
/// @param MPI_rx_buffer_max_size The maximum size of the MPI response buffer
/// @param MPI_rx_buffer_len Pointer to variable that will contain the length of the populated MPI response buffer 
/// @return 0: Success, 1: No bytes to send, 2: Failed UART transmission, 3: Failed UART reception, 4: MPI unresponsive, 5: MPI failed to execute CMD
uint8_t MPI_send_telecommand_get_response(const uint8_t *bytes_to_send, const size_t bytes_to_send_len, uint8_t *MPI_rx_buffer, size_t MPI_rx_buffer_max_size, uint16_t *MPI_rx_buffer_len)
{
	// Set MPI to command mode
	current_mpi_mode = COMMAND_MODE;

	// Transmit the MPI command
	HAL_StatusTypeDef transmit_status = HAL_UART_Transmit(&huart1, bytes_to_send, bytes_to_send_len, TX_TIMEOUT_DURATION_MS);

	// Check UART transmission
	if (transmit_status != HAL_OK)
	{
		return 2; // Error code: Failed UART transmission
	}

	// Receive MPI response byte by byte (This is done to account for potential errors from the mpi where it doesnt send back an expected response)
	HAL_UART_DMAResume(&huart1);
	HAL_StatusTypeDef receive_status = HAL_UART_Receive_DMA(&huart1, (uint8_t*) &UART_mpi_rx_last_byte, 1);
	
	// Check for UART reception errors
    if (receive_status != HAL_OK) {
        return 3; // Error code: Failed UART reception
    }

	// Record start time for mpi response reception
	uint32_t UART_mpi_rx_start_time_ms = HAL_GetTick();

	while(1){
		// MPI hasn't sent any data and has timed out
		if(((HAL_GetTick() - UART_mpi_rx_start_time_ms) > RX_TIMEOUT_DURATION_MS) && (UART_mpi_rx_buffer_write_idx == 0)){
			return 4; // Error code: MPI unresponsive
		}

		// MPI has sent some data and has timed out
		if (((HAL_GetTick() - UART_mpi_rx_last_byte_write_time_ms) > RX_TIMEOUT_DURATION_MS) && (UART_mpi_rx_buffer_write_idx > 0)) {
			// Copy the buffer to the last received byte index
			for (uint16_t i = 0; i < UART_mpi_rx_buffer_write_idx; i++) {
				MPI_rx_buffer[i] = UART_mpi_rx_buffer[i];
			}
			
			// Passing the length of the response buffer back
			*MPI_rx_buffer_len = UART_mpi_rx_buffer_write_idx;
			
			// Clear the buffer (memset to 0, but volatile-compatible) and reset the write pointer.
			for (uint16_t i = 0; i < UART_mpi_rx_buffer_len; i++) {
				UART_mpi_rx_buffer[i] = 0;
			}
			UART_mpi_rx_buffer_write_idx = 0;
			HAL_UART_DMAStop(&huart1);

			// Validate MPI response
			uint8_t MPI_response_result = MPI_validate_telecommand_response(bytes_to_send, MPI_rx_buffer, bytes_to_send_len);

			// Error found during validation
			if(MPI_response_result > 0){
				return MPI_response_result;
			}

			return 0; // MPI successfully executed the telecommand
			// break;
		}
		// TODO: write_idx >= sizeof(rx_buffer) HANDLE OVERFLOW
	}
}

/// @brief The MPI responds to each telecommand with a response code consisting of an echo of the telecommand and a success byte (either 1 for success or 0 for fail).
/// @param MPI_tx_buffer MPI telecommand buffer containing bytes sent
/// @param MPI_rx_buffer MPI response buffer containing bytes received
/// @param MPI_tx_buffer_size Size of the MPI response buffer
/// @return 0: MPI successfully executed telecommand, 5: MPI failed to execute telecommand, 6: Invalid response from the MPI
uint8_t MPI_validate_telecommand_response(const uint8_t *MPI_tx_buffer, uint8_t *MPI_rx_buffer, const uint8_t MPI_tx_buffer_size){
	// Check MPI response for errors: It should be an echo of the cmd sent along with an appended response code
	if (memcmp(MPI_tx_buffer, MPI_rx_buffer, MPI_tx_buffer_size) == 0){
		if(MPI_rx_buffer[MPI_tx_buffer_size] == 0x01){
			return 0; //  MPI executed the cmd successfully
		}
		return 5; // Error code: MPI failed to execute command
	}
	return 6; // Error code: Invalid response from the MPI
}
