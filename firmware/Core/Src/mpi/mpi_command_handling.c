#include "mpi/mpi_command_handling.h"
#include "mpi/mpi_types.h"
#include "debug_tools/debug_uart.h"
#include "uart_handler/uart_handler.h"
#include "main.h"
#include "stm32l4xx_hal.h"
#include "string.h"
#include <stdio.h>

enum Mpi_Mode current_mpi_mode = NOT_LISTENING_TO_MPI_MODE;

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
	// Set MPI to command mode
	current_mpi_mode = COMMAND_MODE;

	// Transmit Buffer to store CMD+Params formatted to be sent to the MPI
	size_t UART1_txBuffer_size = bytes_to_send_len + 2;
	//size_t UART1_rxBuffer_size = UART1_txBuffer_size + 1;
	uint8_t UART1_txBuffer[UART1_txBuffer_size];	   	// +2 accounts for 'TC' command identifier being appended
	memset(UART1_txBuffer, 0, UART1_txBuffer_size); 	// Initialize all elements to 0

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
	uint32_t timeout_duration_ms = 2000;	// CONFIGURATION PARAMETER //FIXME: change to 100 later
	HAL_StatusTypeDef transmit_status = HAL_UART_Transmit(&huart1, (uint8_t *)UART1_txBuffer, UART1_txBuffer_size, timeout_duration_ms);

	// Check UART transmission
	if (transmit_status != HAL_OK)
	{
		return 1; // Error code: Failed UART transmission
	}

	// Receive MPI response byte by byte (This is done to account for potential errors from the mpi where it doesnt send back an expected response)
	HAL_UART_DMAResume(&huart1);
	HAL_StatusTypeDef receive_status = HAL_UART_Receive_DMA(&huart1, (uint8_t*) &UART_mpi_rx_last_byte, 1);
	
	// Check for UART reception errors
    if (receive_status != HAL_OK) {
        return 2; // Error code: Failed UART reception
    }

	uint32_t UART_mpi_rx_start_time_ms = HAL_GetTick();

	while(1){
		// MPI hasn't sent any data and has timed out
		if(((HAL_GetTick() - UART_mpi_rx_start_time_ms) > timeout_duration_ms) && (UART_mpi_rx_buffer_write_idx == 0)){
			DEBUG_uart_print_str("Break1\n");
			break; // TODO: handle case
		}
		// MPI has sent some data and has timed out
		if (((HAL_GetTick() - UART_mpi_rx_last_byte_write_time_ms) > timeout_duration_ms) && (UART_mpi_rx_buffer_write_idx > 0)) {
			// DEBUG_uart_print_str("Size of MPI response buffer received: ");
			// DEBUG_uart_print_int32((uint32_t)UART_mpi_rx_buffer_write_idx);
			// DEBUG_uart_print_str("\n");

			// Copy the buffer to the last received byte index
			for (uint16_t i = 0; i < UART_mpi_rx_buffer_write_idx; i++) {
				mpi_response[i] = UART_mpi_rx_buffer[i];
			}

			// TODO: Validate MPI response

			// Clear the buffer (memset to 0, but volatile-compatible) and reset the write pointer.
			for (uint16_t i = 0; i < UART_mpi_rx_buffer_len; i++) {
				UART_mpi_rx_buffer[i] = 0;
			}
			UART_mpi_rx_buffer_write_idx = 0;

			DEBUG_uart_print_str("Break2\n");
			break; // TODO: handle case
		}
		// TODO: write_idx >= sizeof(rx_buffer)
	}
	HAL_UART_DMAStop(&huart1);
	//HAL_StatusTypeDef receive_status = HAL_UART_Receive_DMA(&huart1, mpi_response, mpi_response_size);
	
	// Start the UART reception process in interrupt mode from the MPI
    
	// HAL_UART_Receive_DMA(&huart1, mpi_response, mpi_response_size);
	//HAL_Delay(100);

	// Check MPI response for errors: It should be an echo of the cmd sent along with an appended response code
	if (memcmp(UART1_txBuffer, mpi_response, UART1_txBuffer_size) != 0 || mpi_response[UART1_txBuffer_size] != 0x01)
	{
		return 3; // Error code: Invalid response from the MPI
	}
	
	return 0; // Valid response from the MPI
}

// void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
//     if (huart->Instance == USART1) {
//         // Data reception complete
//         // Process the received data in mpi_response
//     }
// }