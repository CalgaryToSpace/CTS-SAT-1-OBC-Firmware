/*
 * mpiCommands.h
 *
 * Created on: Nov 17, 2023
 * Author: Vaibhav Kapoor
 */

#ifndef INC_MPICOMMANDHANDLING_H_
#define INC_MPICOMMANDHANDLING_H_
#include "main.h"
#include "mpi/double_buffer.h"
#include "mpi/mpi_types.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/**
 * @brief Command Code List
 *
 * Enumerates the various Command Codes used in the MPI.
 * These codes represent different commands that can be sent
 * to control or configure the MPI's functionality.
 */
typedef enum
{
	MPI_CMD_HV_INNER_DOME_SCAN_MODE = 2,
	MPI_CMD_HV_SET_FACEPLATE_VOLTAGE = 4,
	MPI_CMD_HV_SET_SCAN_MIDPOINT_VOLTAGE = 5,
	MPI_CMD_HV_SET_SCAN_AMPLITUDE_VOLTAGE = 6,
	MPI_CMD_HV_SET_SCAN_NUMBER_OF_STEPS = 7,
	MPI_CMD_SET_INTEGRATION_PERIOD = 8,
	MPI_CMD_AGC_ENABLE = 9,
	MPI_CMD_AGC_DISABLE = 10,
	MPI_CMD_BL_ENTER_BOOTLOADER = 13,
	MPI_CMD_BL_EXIT_BOOTLOADER = 14,
	MPI_CMD_BL_SET_APPSW_CRC = 15,
	MPI_CMD_BL_ERASE_TEMP_APP_MEMORY = 16,
	MPI_CMD_BL_STORE_APPSW_CHUNK = 17,
	MPI_CMD_BL_UPGRADE_FIRMWARE = 18,
	MPI_CMD_AGC_SET_THRESHOLDS = 20,
	MPI_CMD_AGC_SET_DISABLED_SHUTTER_DUTY_CYCLE = 21,
	MPI_CMD_AGC_SET_PIXEL_RANGE = 22,
	MPI_CMD_AGC_SET_NUMBER_OF_MAXIMA_FOR_CONTROL_VALUE = 23,
	MPI_CMD_AGC_SET_NUMBER_OF_PIXELS_FOR_BASELINE_VALUE = 24,
	MPI_CMD_AGC_SET_STEP_FACTOR = 25,
	MPI_CMD_SET_BASELINE_FIRST_PIXEL = 29
} MPI_CMD_CODE_T;

void processFrameData(MpiFrame_t frame);
void writeFrameToMemory(MpiFrame_t frame);

// Individual MPI Command functions
uint8_t SEND_MPI_TC_HV_INNER_DOME_SCAN_MODE(uint8_t mode);
uint8_t SEND_MPI_TC_HV_SET_FACEPLATE_VOLTAGE(uint16_t voltage);
uint8_t SEND_MPI_TC_HV_SET_SCAN_MIDPOINT_VOLTAGE(uint16_t voltage);
uint8_t SEND_MPI_TC_HV_SET_SCAN_AMPLITUDE_VOLTAGE(uint16_t voltage);
uint8_t SEND_MPI_TC_HV_SET_SCAN_NUMBER_OF_STEPS(uint16_t steps);
uint8_t SEND_MPI_TC_SET_INTEGRATION_PERIOD(uint16_t period);
uint8_t SEND_MPI_TC_AGC_ENABLE();
uint8_t SEND_MPI_TC_AGC_DISABLE();
uint8_t SEND_MPI_TC_BL_ENTER_BOOTLOADER();
uint8_t SEND_MPI_TC_BL_EXIT_BOOTLOADER();
uint8_t SEND_MPI_TC_BL_SET_APPSW_CRC(uint16_t CRC_Value);
uint8_t SEND_MPI_TC_BL_ERASE_TEMP_APP_MEMORY();
uint8_t SEND_MPI_TC_BL_STORE_APPSW_CHUNK(uint32_t offset, uint32_t N, uint8_t *memoryValues);
uint8_t SEND_MPI_TC_BL_UPGRADE_FIRMWARE();
uint8_t SEND_MPI_TC_AGC_SET_THRESHOLDS(uint32_t threshold);
uint8_t SEND_MPI_TC_AGC_SET_DISABLED_SHUTTER_DUTY_CYCLE(uint16_t shutterDutyCycle);
uint8_t SEND_MPI_TC_AGC_SET_PIXEL_RANGE(uint32_t index);
uint8_t SEND_MPI_TC_AGC_SET_NUMBER_OF_MAXIMA_FOR_CONTROL_VALUE(uint8_t n);
uint8_t SEND_MPI_TC_AGC_SET_NUMBER_OF_PIXELS_FOR_BASELINE_VALUE(uint8_t n);
uint8_t SEND_MPI_TC_AGC_SET_STEP_FACTOR(uint8_t n);
uint8_t SEND_MPI_TC_SET_BASELINE_FIRST_PIXEL(uint8_t n);

/**
 * @brief
 *
 * @param bytes_to_send
 * @param bytes_to_send_length
 * @param mpi_command_response
 * @return 0 - Success, 1 - MPI side failure, 2 - Transmit Error, 3 - Receive Error
 */
uint8_t sendTelecommandHex(uint8_t *bytes_to_send, size_t bytes_to_send_len, uint8_t *mpi_cmd_response, size_t mpi_cmd_response_len);

/**
 * Sends tele-command to the MPI over UART
 * @param - commandCode references to the corresponding MPI command
 * 			(Example: AGC_ENABLE has command code 9 and takes in no parameters (NULL))
 * 			Refer to MPI Control Doc - Table 3 for more details
 * @param - params holds command parameters
 * 			Set to NULL when no parameters are required
 * @return - MPI response code
 * 		 	 1 for successful transmission & receive by the MPI
 * 		     0 if an error occurs
 */
uint8_t sendTelecommand(uint8_t commandCode, const uint8_t *params);
// uint8_t sendTelecommand(uint8_t commandCode, uint8_t parameters);

// Task methods to run receive and parsing concurrently
void receiveIncomingMPIFramesTask(void *pvParameters);
void processIncomingMPIFramesTask(void *pvParameters);
void StartMPIProcessingTasks();

#endif /* INC_MPICOMMANDHANDLING_H_ */
