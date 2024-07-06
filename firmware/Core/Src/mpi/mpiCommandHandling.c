/*
 * mpiCommandHandling.c
 *
 * Created on: Nov 17, 2023
 * Author: Vaibhav Kapoor
 */

#include <../Inc/mpi/mpiCommandHandling.h>
#include <../Inc/mpi/double_buffer.h>
#include "debug_tools/debug_uart.h"
// #include <../Inc/memory_utilities.h>
#include "main.h"
#include "string.h"
#include <stdio.h>

// Global Variables
#define BUFFER_SIZE 160		 // Buffer Size set to accommodate length of a Tele-command packet
DoubleBuffer MPIFrameBuffer; // Double buffer is used to receive incoming MPI Frames for processing

/**
 * @brief Sends commandcode+params to the MPI
 * @param command data (command) being sent to the MPI
 * @returns 0 - success, 1 - failure from the MPI, 2 - failed UART transmission, 3 - failed UART reception
 */
uint8_t sendTelecommandHex(uint8_t *bytes_to_send, size_t bytes_to_send_len, uint8_t *mpi_cmd_response, size_t mpi_cmd_response_len)
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

/**
 * Sends a command to the MPI over UART and returns back the MPI response code (success / fail)
 */
uint8_t sendTelecommand(uint8_t commandCode, const uint8_t *params)
{

	size_t paramSize = sizeof(params);
	uint8_t UART1_txBuffer[3 + paramSize]; // Buffer stores outgoing tele-command frame
	uint8_t UART1_rxBuffer[4];			   // Buffer stores incoming tele-command response from the MPI (command code followed by response code)

	// Add “TC” bytes to the buffer (Tele-Command identifier)
	UART1_txBuffer[0] = 0x54;
	UART1_txBuffer[1] = 0x43;

	// Add the command code to the buffer
	UART1_txBuffer[2] = commandCode;

	// Add the parameters to the buffer if any
	if (paramSize != 0)
	{
		for (size_t i = 0; i < paramSize; i++)
		{
			UART1_txBuffer[3 + i] = params[i];
		}
	}

	//	// Append parameters onto the TX buffer if any
	//	if (parameters != 0xFF) {
	//		UART1_txBuffer[3] = parameters;
	//	}

	// Transmit the command & receive echo from the MPI TODO: confirm why char* for size
	HAL_UART_Transmit(&huart1, (uint8_t *)UART1_txBuffer, strlen((char *)UART1_txBuffer), 100);
	HAL_UART_Receive_DMA(&huart1, UART1_rxBuffer, BUFFER_SIZE);

	// Return the response code from the MPI TODO: confirm what exactly is echoed back, are parameters part of it?
	return UART1_rxBuffer[(sizeof(UART1_rxBuffer) / sizeof(UART1_rxBuffer[0])) - 1];
}

/**
 * Read incoming MPI frames into a double buffer for processing
 * @param - void pointer pvParameters takes in task related information
 * TODO: Error Check incoming MPI Frames & handle buffer overloading
 */
void receiveIncomingMPIFramesTask(void *pvParameters)
{

	// Check if double buffer is initialized
	static BaseType_t doubleBufferInitialized = pdFALSE;
	if (doubleBufferInitialized == pdFALSE)
	{
		DoubleBuffer_Init(&MPIFrameBuffer);
		doubleBufferInitialized = pdTRUE;
	}

	// Temporary buffer to collect incoming frame
	uint8_t buffer[sizeof(MpiFrame_t)];
	MpiFrame_t frame;

	// Continuously read incoming data frames
	while (1)
	{
		HAL_UART_Receive_DMA(&huart1, buffer, BUFFER_SIZE);

		// Copy frame data from the buffer into a formatted Struct
		frame.sync_byte1 = buffer[0];
		frame.sync_byte2 = buffer[1];
		frame.sync_byte3 = buffer[2];
		frame.sync_byte4 = buffer[3];
		frame.frame_counter = (buffer[4] << 8) + buffer[5]; // (buffer[4] << 8) is the same as (buffer[4] * 256)
		frame.board_temperature = (buffer[6] << 8) + buffer[7];
		frame.firmware_version = buffer[8];
		frame.mpi_unit_id = buffer[9];
		frame.detector_status = (buffer[10] << 8) + buffer[11];
		frame.inner_dome_voltage_setting = (buffer[12] << 8) + buffer[13];
		frame.spib_reserved = buffer[14];
		frame.inner_dome_scan_index = buffer[15];
		frame.faceplate_voltage_setting = (buffer[16] << 8) + buffer[17];
		frame.faceplate_voltage_adc_reading = (buffer[18] << 8) + buffer[19];
		frame.inner_dome_voltage_adc_reading = (buffer[20] << 8) + buffer[21];

		// Copy the pixel array (assuming 67 elements)
		for (int i = 0; i < 67; ++i)
		{
			frame.pixels[i] = (buffer[22 + i * 2] << 8) + buffer[23 + i * 2];
		}

		// Copy the cyclic redundancy check (CRC)
		frame.cyclic_redundancy_check = (buffer[156] << 8) + buffer[157];

		// Add data frame to double buffer
		DoubleBuffer_Write(&MPIFrameBuffer, (uint8_t *)&frame);
	}
}

/**
 * Process received MPI frames from the double buffer
 * For testing purposes currently: Write to memory & write over UART
 * @param - void pointer pvParameters takes in task related information
 */
void processIncomingMPIFramesTask(void *pvParameters)
{
	MpiFrame_t frame;

	while (1)
	{
		// Wait until data is ready in the buffer
		if (xSemaphoreTake(MPIFrameBuffer.dataReady, portMAX_DELAY) == pdTRUE)
		{
			// Read data from double buffer
			uint32_t bytesRead = DoubleBuffer_Read(&MPIFrameBuffer, (uint8_t *)&frame, sizeof(MpiFrame_t));

			if (bytesRead > 0)
			{
				processFrameData(frame);
			}
		}
	}

	// Add delay or yield to FreeRTOS if necessary
	// vTaskDelay(pdMS_TO_TICKS(100));
	taskYIELD(); // Yield to other tasks to prevent busy-waiting
}

/**
 * Creates and starts the MPI frame receive and processing tasks for concurrent execution
 */
void StartMPIProcessingTasks()
{
	// Create tasks for parsing and processing MPI frames
	xTaskCreate((TaskFunction_t)receiveIncomingMPIFramesTask, "ParseMPI", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
	xTaskCreate((TaskFunction_t)processIncomingMPIFramesTask, "ProcessMPI", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
}

// This function is responsible for saving / displaying the parsed MPI Frame
// Display over UART is for testing only
void processFrameData(MpiFrame_t frame)
{

	// Define an array of buffers for each variable in the structure that needs to be tested
	char variable_buffer[255];

	// Clear the buffer before use
	variable_buffer[0] = '\0';

	// Concatenate test variable values into the char buffer to send formatted values over
	sprintf(variable_buffer,
			"Frame Counter: %u, Board Temperature: %u, Voltage ADC Reading: %u\n",
			frame.frame_counter, frame.board_temperature,
			frame.faceplate_voltage_adc_reading);

	// Send formatted data over HLPUART for verification
	// HAL_UART_Transmit(&hlpuart1, (uint8_t*)variable_buffer, strlen(variable_buffer), HAL_MAX_DELAY);
	debug_uart_print_str(variable_buffer);

	// TODO: For testing write parsed and raw data to files to verify and update test plans
	writeFrameToMemory(frame);
}

// This function deals with writing data packets to memory using MEMORY UTILITIES
// TODO: Rewrite this method once the littlefs module is merged in
void writeFrameToMemory(MpiFrame_t frame)
{

	// Calling the WRITE function and making sure it's successful
	// if (WRITE_FILE(&hspi1, (uint8_t *)&frame, sizeof(frame)))
	// {
	// 	PRINT_STRING_UART("MPIFrame Written successfully");
	// }
	// else
	// {
	// 	PRINT_STRING_UART("Error Occurred while writing MPIFrame");
	// }
}

// Tele-Command specific functions

/**
 * HV_INNER_DOME_SCAN_MODE Tele-Command
 * Command Code: 2
 * @param - Mode (0: no scan, 1: sawtooth, low to high, 4: sawtooth with beam tracking)
 * @return - 1 for successful transmission & receive by the MPI
 * 		   - 0 if an error occurs
 */
uint8_t sendTC_HV_INNER_DOME_SCAN_MODE(uint8_t mode)
{
	// Check if mode is within valid range
	if (mode != 0 && mode != 1 && mode != 4)
	{
		return 3; // Invalid mode parameter (custom code to distinguish from MPI response codes)
	}

	// Prepare parameters for the command
	uint8_t params[] = {mode};

	// Command has been verified and parsed to be sent to the MPI (Returns MPI response)
	return sendTelecommand(MPI_CMD_HV_INNER_DOME_SCAN_MODE, params);
}

// TODO: Confirm for voltage methods whether they are two settings or a range between them
uint8_t sendTC_HV_SET_FACEPLATE_VOLTAGE(uint16_t voltage)
{
	// Check if voltage is valid
	if (voltage != 65535 || voltage != 0)
	{
		return 3; // Invalid voltage parameter (custom code to distinguish from MPI response codes)
	}

	// Prepare parameters for the command
	uint8_t params[2];
	params[0] = (uint8_t)(voltage >> 8); // MSB of voltage
	params[1] = (uint8_t)(voltage);		 // LSB of voltage

	// Command has been verified and parsed to be sent to the MPI (Returns MPI response)
	return sendTelecommand(MPI_CMD_HV_SET_FACEPLATE_VOLTAGE, params);
}

uint8_t sendTC_HV_SET_SCAN_MIDPOINT_VOLTAGE(uint16_t voltage)
{
	// Check if voltage is valid
	if (voltage != 65535 || voltage != 0)
	{
		return 3; // Invalid voltage parameter (custom code to distinguish from MPI response codes)
	}

	// Prepare parameters for the command
	uint8_t params[2];
	params[0] = (uint8_t)(voltage >> 8); // MSB of voltage
	params[1] = (uint8_t)(voltage);		 // LSB of voltage

	// Command has been verified and parsed to be sent to the MPI (Returns MPI response)
	return sendTelecommand(MPI_CMD_HV_SET_SCAN_MIDPOINT_VOLTAGE, params);
}

uint8_t sendTC_HV_SET_SCAN_AMPLITUDE_VOLTAGE(uint16_t voltage)
{
	// Check if voltage is valid
	if (voltage != 32767 || voltage != 0)
	{
		return 3; // Invalid voltage parameter (custom code to distinguish from MPI response codes)
	}

	// Prepare parameters for the command
	uint8_t params[2];
	params[0] = (uint8_t)(voltage >> 8); // MSB of voltage
	params[1] = (uint8_t)(voltage);		 // LSB of voltage

	// Command has been verified and parsed to be sent to the MPI (Returns MPI response)
	return sendTelecommand(MPI_CMD_HV_SET_SCAN_AMPLITUDE_VOLTAGE, params);
}

uint8_t sendTC_HV_SET_SCAN_NUMBER_OF_STEPS(uint16_t steps)
{
	// TODO: Check if steps is valid (Number of steps per voltage scan is TBD)

	// Prepare parameters for the command
	uint8_t params[2];
	params[0] = (uint8_t)(steps >> 8); // Byte 1 of steps
	params[1] = (uint8_t)(steps);	   // Byte 0 of steps

	// Command has been verified and parsed to be sent to the MPI (Returns MPI response)
	return sendTelecommand(MPI_CMD_HV_SET_SCAN_NUMBER_OF_STEPS, params);
}

uint8_t sendTC_SET_INTEGRATION_PERIOD(uint16_t period)
{
	// Calculate the integration period setting (Based on Frame period tau = 8 + integrationPeriodSetting/12 (ms))
	// float frame_period = 8.0f + (period / 12.0f);

	// TODO: Need more info on the function to write error checks and perform formatting

	// Prepare parameters for the command
	uint8_t params[2];
	params[0] = (uint8_t)(period >> 8); // MSB of integration period setting
	params[1] = (uint8_t)(period);		// LSB of integration period setting

	// Command has been verified and parsed to be sent to the MPI (Returns MPI response)
	return sendTelecommand(MPI_CMD_SET_INTEGRATION_PERIOD, params);
}

uint8_t sendTC_AGC_ENABLE()
{
	// No parameters in this command, create an empty array
	uint8_t params[] = {};

	// Command ready to be sent to the MPI
	return sendTelecommand(MPI_CMD_AGC_ENABLE, params);
}

uint8_t sendTC_AGC_DISABLE()
{
	// No parameters in this command, create an empty array
	uint8_t params[] = {};

	// Command ready to be sent to the MPI
	return sendTelecommand(MPI_CMD_AGC_DISABLE, params);
}

uint8_t sendTC_BL_ENTER_BOOTLOADER()
{
	// No parameters in this command, create an empty array
	uint8_t params[] = {};

	// Command ready to be sent to the MPI
	return sendTelecommand(MPI_CMD_BL_ENTER_BOOTLOADER, params);
}

uint8_t sendTC_BL_EXIT_BOOTLOADER()
{
	// No parameters in this command, create an empty array
	uint8_t params[] = {};

	// Command ready to be sent to the MPI
	return sendTelecommand(MPI_CMD_BL_EXIT_BOOTLOADER, params);
}

uint8_t sendTC_BL_SET_APPSW_CRC(uint16_t CRC_Value)
{
	// TODO: Need to know if there are any error checks needed here

	// Prepare parameters for the command
	uint8_t params[2];
	params[0] = (uint8_t)(CRC_Value >> 8); // Byte 1 of CRC_Value
	params[1] = (uint8_t)(CRC_Value);	   // Byte 0 of CRC_Value

	// Command has been verified and parsed to be sent to the MPI (Returns MPI response)
	return sendTelecommand(MPI_CMD_BL_SET_APPSW_CRC, params);
}

uint8_t sendTC_BL_ERASE_TEMP_APP_MEMORY()
{
	// No parameters in this command, create an empty array
	uint8_t params[] = {};

	// Command ready to be sent to the MPI
	return sendTelecommand(MPI_CMD_BL_ERASE_TEMP_APP_MEMORY, params);
}

// @param - uint32_t + variable payload (2-byte memory offset, 2-byte memory words N, 2*N bytes of memory values)
uint8_t sendTC_BL_STORE_APPSW_CHUNK(uint32_t offset, uint32_t N, uint8_t *memoryValues)
{
	// TODO: Need more clarity on the data being sent here to parse them into the send method...
	uint8_t params[] = {};
	// Error check CRC parameter
	return sendTelecommand(MPI_CMD_BL_STORE_APPSW_CHUNK, params);
}

uint8_t sendTC_BL_UPGRADE_FIRMWARE()
{
	// No parameters in this command, create an empty array
	uint8_t params[] = {};

	// Command ready to be sent to the MPI
	return sendTelecommand(MPI_CMD_BL_UPGRADE_FIRMWARE, params);
}

uint8_t sendTC_AGC_SET_THRESHOLDS(uint32_t threshold)
{
	// Extract lower and upper thresholds from the 32-bit threshold value
	uint16_t lowerThreshold = (uint16_t)(threshold);
	uint16_t upperThreshold = (uint16_t)(threshold >> 16);

	// Check if thresholds are within valid range (0 to 65535)
	if (lowerThreshold < 0 || lowerThreshold > 65535 || upperThreshold < 0 || upperThreshold > 65535)
	{
		return 3; // Invalid voltage parameter (custom code to distinguish from MPI response codes)
	}

	// Prepare parameters for the command
	uint8_t params[4];							// 2 bytes for lower threshold + 2 bytes for upper threshold
	params[0] = (uint8_t)(lowerThreshold);		// LSB of lower threshold
	params[1] = (uint8_t)(lowerThreshold >> 8); // MSB of lower threshold
	params[2] = (uint8_t)(upperThreshold);		// LSB of upper threshold
	params[3] = (uint8_t)(upperThreshold >> 8); // MSB of upper threshold

	// Command has been verified and parsed to be sent to the MPI (Returns MPI response)
	return sendTelecommand(MPI_CMD_AGC_SET_THRESHOLDS, params);
}

uint8_t sendTC_AGC_SET_DISABLED_SHUTTER_DUTY_CYCLE(uint16_t shutterDutyCycle)
{
	// Check if shutterDutyCycle is within valid range (0 to 50000)
	if (shutterDutyCycle < 0 || shutterDutyCycle > 50000)
	{
		return 3; // Invalid voltage parameter (custom code to distinguish from MPI response codes)
	}

	// Prepare parameters for the command
	uint8_t params[2];
	params[0] = (uint8_t)(shutterDutyCycle >> 8); // Byte 1 of shutterDutyCycle
	params[1] = (uint8_t)(shutterDutyCycle);	  // Byte 0 of shutterDutyCycle

	// Command has been verified and parsed to be sent to the MPI (Returns MPI response)
	return sendTelecommand(MPI_CMD_AGC_SET_DISABLED_SHUTTER_DUTY_CYCLE, params);
}

uint8_t sendTC_AGC_SET_PIXEL_RANGE(uint32_t index)
{
	// Extract lower and upper index from the 32-bit index value
	uint16_t lowerIndex = (uint16_t)(index);
	uint16_t upperIndex = (uint16_t)(index >> 16);

	// TODO: Need to know the bounds of the index for error checks

	// Prepare parameters for the command
	uint8_t params[4];
	params[0] = (uint8_t)(lowerIndex);		// LSB of lower threshold
	params[1] = (uint8_t)(lowerIndex >> 8); // MSB of lower threshold
	params[2] = (uint8_t)(upperIndex);		// LSB of upper threshold
	params[3] = (uint8_t)(upperIndex >> 8); // MSB of upper threshold

	// Command has been verified and parsed to be sent to the MPI (Returns MPI response)
	return sendTelecommand(MPI_CMD_AGC_SET_PIXEL_RANGE, params);
}

uint8_t sendTC_AGC_SET_NUMBER_OF_MAXIMA_FOR_CONTROL_VALUE(uint8_t n)
{
	// Check if N is within valid range (1 to 10)
	if (n < 1 || n > 10)
	{
		return 3; // Invalid voltage parameter (custom code to distinguish from MPI response codes)
	}

	// Prepare parameters for the command
	uint8_t params[] = {n};

	// Command has been verified and parsed to be sent to the MPI (Returns MPI response)
	return sendTelecommand(MPI_CMD_AGC_SET_NUMBER_OF_MAXIMA_FOR_CONTROL_VALUE, params);
}

uint8_t sendTC_AGC_SET_NUMBER_OF_PIXELS_FOR_BASELINE_VALUE(uint8_t n)
{
	// Check if N is within valid range (1 to 10)
	if (n < 1 || n > 10)
	{
		return 3; // Invalid voltage parameter (custom code to distinguish from MPI response codes)
	}

	// Prepare parameters for the command
	uint8_t params[] = {n};

	// Command has been verified and parsed to be sent to the MPI (Returns MPI response)
	return sendTelecommand(MPI_CMD_AGC_SET_NUMBER_OF_PIXELS_FOR_BASELINE_VALUE, params);
}

uint8_t sendTC_AGC_SET_STEP_FACTOR(uint8_t n)
{
	// Check if N is within valid range (1 to 10)
	if (n < 1 || n > 255)
	{
		return 3; // Invalid voltage parameter (custom code to distinguish from MPI response codes)
	}

	// TODO: Confirm if n needs to be passed or AGC step factor = (1+N/100.0)

	// Prepare parameters for the command
	uint8_t params[] = {n};

	// Command has been verified and parsed to be sent to the MPI (Returns MPI response)
	return sendTelecommand(MPI_CMD_AGC_SET_STEP_FACTOR, params);
}

uint8_t sendTC_SET_BASELINE_FIRST_PIXEL(uint8_t n)
{
	// Check if N is within valid range (1 to 10)
	if (n < 0 || n > 50)
	{
		return 3; // Invalid voltage parameter (custom code to distinguish from MPI response codes)
	}

	// Prepare parameters for the command
	uint8_t params[] = {n};

	// Command has been verified and parsed to be sent to the MPI (Returns MPI response)
	return sendTelecommand(MPI_CMD_SET_BASELINE_FIRST_PIXEL, params);
}

// Validate the MPI frame for correctness
// uint8_t validateMpiFrame(MpiFrame_t* frame) {
//    // Implement your validation logic here
//    // For example, check synchronization bytes, CRC, etc.
//    // Return 1 if frame is valid, 0 otherwise
//    if (frame->sync_byte1 == 0xAA && frame->sync_byte2 == 0xBB && frame->sync_byte3 == 0xCC && frame->sync_byte4 == 0xDD) {
//        // Frame is valid
//        return 1;
//    } else {
//        // Invalid frame
//        return 0;
//    }
//}
