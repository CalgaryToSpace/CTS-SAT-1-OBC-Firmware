#include "mpi/mpi_command_handling.h"
#include "debug_tools/debug_uart.h"
#include "main.h"
#include "string.h"
#include <stdio.h>

/**
 * @brief Sends commandcode+params to the MPI as bytes
 * @param command data (command) being sent to the MPI
 * @returns 0 - success, 1 - failure from the MPI, 2 - failed UART transmission, 3 - failed UART reception
 */
uint8_t MPI_send_telecommand_hex(const uint8_t *bytes_to_send, size_t bytes_to_send_len, uint8_t *mpi_cmd_response, size_t mpi_cmd_response_len)
{
	// Buffers to store outgoing & incoming data from the MPI
	uint8_t UART1_txBuffer[bytes_to_send_len + 2];	   // +2 accounts for 'TC' command identifier being appended
	memset(UART1_txBuffer, 0, sizeof(UART1_txBuffer)); // Initialize all elements to 0

	// All commands to the MPI must begin with 'TC'
	UART1_txBuffer[0] = 0x54; // Hex representation of 'T'
	UART1_txBuffer[1] = 0x43; // Hex representation of 'C'

	// Add the command bytes / parameters to the buffer
	if (bytes_to_send_len != 0)
	{
		for (size_t i = 0; i < bytes_to_send_len; i++)
		{
			UART1_txBuffer[2 + i] = bytes_to_send[i];
		}
	}

	// Transmit the MPI command
	HAL_StatusTypeDef transmit_status = HAL_UART_Transmit(&huart1, (uint8_t *)UART1_txBuffer, sizeof(UART1_txBuffer), 100);

	// Check UART transmission
	if (transmit_status != HAL_OK)
	{
		return 2; // Error code for failed UART transmission
	}

	// Receive MPI response
	HAL_StatusTypeDef receive_status = HAL_UART_Receive_DMA(&huart1, (uint8_t *)mpi_cmd_response, mpi_cmd_response_len);

	// Check UART reception
	if (receive_status != HAL_OK)
	{
		return 3; // Error Code for failed UART reception
	}

	// Verify MPI response
	if (memcmp(UART1_txBuffer, mpi_cmd_response, *bytes_to_send + 2) != 0)
	{
		return 1; // Error code for invalid response from the MPI
	}
	// Check if the byte after (cmd echo) is 0x01
	if (mpi_cmd_response[bytes_to_send_len + 3] == 0x01)
	{
		return 0; // Valid response from the MPI
	}
	else
	{
		return 1; // Error code for invalid response from the MPI
	}
}
