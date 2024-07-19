#include "mpi/mpi_command_handling.h"
#include "debug_tools/debug_uart.h"
#include "main.h"
#include "string.h"
#include <stdio.h>

/**
 * @brief Sends commandcode+params to the MPI as bytes
 * 
 * @param bytes_to_send Buffer containing the CMD + Params (IF ANY) as hex bytes
 * @param bytes_to_send_len Size of CMD buffer
 * @param mpi_cmd_response Buffer to store incoming response from the MPI
 * @param mpi_cmd_response_len Size of response buffer
 * @return 0: Success, 1: Failed UART transmission, 2: Failed UART reception, 3: MPI failed to execute CMD
 */
uint8_t MPI_send_telecommand_hex(const uint8_t *bytes_to_send, size_t bytes_to_send_len, uint8_t mpi_response[], size_t mpi_response_size)
{
	// Transmit Buffer to store CMD+Params formatted to be sent to the MPI
	uint8_t UART1_txBuffer[bytes_to_send_len + 2];	   	// +2 accounts for 'TC' command identifier being appended
	memset(UART1_txBuffer, 0, sizeof(UART1_txBuffer)); 	// Initialize all elements to 0

	// All commands to the MPI must begin with 'TC'
	UART1_txBuffer[0] = 0x54;	// Hex representation of 'T'
	UART1_txBuffer[1] = 0x43;	// Hex representation of 'C'

	// Add the command bytes & parameters to the Transmit Buffer
	if (bytes_to_send_len > 0)
	{
		for (size_t i = 0; i < bytes_to_send_len; i++)
		{
			UART1_txBuffer[2 + i] = bytes_to_send[i];
		}
	}

	// Transmit the MPI command
	uint32_t tx_timeout = 100;
	HAL_StatusTypeDef transmit_status = HAL_UART_Transmit(&huart1, (uint8_t *)UART1_txBuffer, sizeof(UART1_txBuffer), tx_timeout);

	// Check UART transmission
	if (transmit_status != HAL_OK)
	{
		return 1; // Error code: Failed UART transmission
	}

	// Receive MPI response
	HAL_StatusTypeDef receive_status = HAL_UART_Receive_DMA(&huart1, mpi_response, mpi_response_size);

	// Check UART reception
	if (receive_status != HAL_OK)
	{
		return 2; // Error code: Failed UART reception
	}

	// Check MPI response for errors: It should be an echo of the cmd sent along with an appended response code
	if (memcmp(UART1_txBuffer, mpi_response, bytes_to_send_len+2) != 0 || mpi_response[bytes_to_send_len+3] != 0x01)
	{
		return 3; // Error code: Invalid response from the MPI
	}
	
	return 0; // Valid response from the MPI
}
