/*
 * mpiCommands.h
 *
 * Created on: Nov 17, 2023
 * Author: Vaibhav Kapoor
 */

#ifndef INC_MPICOMMANDHANDLING_H_
#define INC_MPICOMMANDHANDLING_H_
#include "main.h"
#include "DoubleBuffer.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/**
 * @brief Represents an MPI data frame.
 *
 * This struct lays out the structure of an MPI data frame,
 * which consists of synchronization bytes, frame counter, housekeeping telemetry,
 * image pixel data, and cyclic redundancy check (CRC) bytes.
 */
typedef struct {
	uint8_t sync_byte1;							// First synchronization byte
	uint8_t sync_byte2;							// Second synchronization byte
	uint8_t sync_byte3;							// Third synchronization byte
	uint8_t sync_byte4;							// Fourth synchronization byte
	uint16_t frame_counter;						// Frame counter for the data frame
	int16_t board_temperature;					// Current board temperature
	uint8_t firmware_version;					// Firmware version of the MPI unit
	uint8_t mpi_unit_id;						// ID of the MPI unit
	uint16_t detector_status;					// Current status of the detector
	uint16_t inner_dome_voltage_setting;		// Voltage setting for the inner dome
	uint8_t spib_reserved;						// Reserved field
	uint8_t inner_dome_scan_index;				// Index for inner dome scanning
	uint16_t faceplate_voltage_setting;			// Voltage setting for the faceplate
	uint16_t faceplate_voltage_adc_reading;		// ADC reading for faceplate voltage
	uint16_t inner_dome_voltage_adc_reading;	// ADC reading for inner dome voltage
	uint16_t pixels[67];						// Array of 67 image pixels
	uint16_t cyclic_redundancy_check;			// CRC for data integrity check
} MpiFrame_t;

/**
 * @brief Command Code List
 *
 * Enumerates the various Command Codes used in the MPI.
 * These codes represent different commands that can be sent
 * to control or configure the MPI's functionality.
 */
typedef enum {
	HV_INNER_DOME_SCAN_MODE = 2,
	HV_SET_FACEPLATE_VOLTAGE = 4,
	HV_SET_SCAN_MIDPOINT_VOLTAGE = 5,
	HV_SET_SCAN_AMPLITUDE_VOLTAGE = 6,
	HV_SET_SCAN_NUMBER_OF_STEPS = 7,
	SET_INTEGRATION_PERIOD = 8,
	AGC_ENABLE = 9,
	AGC_DISABLE = 10,
	BL_ENTER_BOOTLOADER = 13,
	BL_EXIT_BOOTLOADER = 14,
	BL_SET_APPSW_CRC = 15,
	BL_ERASE_TEMP_APP_MEMORY = 16,
	BL_STORE_APPSW_CHUNK = 17,
	BL_UPGRADE_FIRMWARE = 18,
	AGC_SET_THRESHOLDS = 20,
	AGC_SET_DISABLED_SHUTTER_DUTY_CYCLE = 21,
	AGC_SET_PIXEL_RANGE = 22,
	AGC_SET_NUMBER_OF_MAXIMA_FOR_CONTROL_VALUE = 23,
	AGC_SET_NUMBER_OF_PIXELS_FOR_BASELINE_VALUE = 24,
	AGC_SET_STEP_FACTOR = 25,
	SET_BASELINE_FIRST_PIXEL = 29
} CommandCode;

void processFrameData(MpiFrame_t frame);
void writeFrameToMemory(MpiFrame_t frame);

// Individual MPI Command functions
uint8_t sendTC_HV_INNER_DOME_SCAN_MODE(uint8_t mode);
uint8_t sendTC_HV_SET_FACEPLATE_VOLTAGE(uint16_t voltage);
uint8_t sendTC_HV_SET_SCAN_MIDPOINT_VOLTAGE(uint16_t voltage);
uint8_t sendTC_HV_SET_SCAN_AMPLITUDE_VOLTAGE(uint16_t voltage);
uint8_t sendTC_HV_SET_SCAN_NUMBER_OF_STEPS(uint16_t steps);
uint8_t sendTC_SET_INTEGRATION_PERIOD(uint16_t period);
uint8_t sendTC_AGC_ENABLE();
uint8_t sendTC_AGC_DISABLE();
uint8_t sendTC_BL_ENTER_BOOTLOADER();
uint8_t sendTC_BL_EXIT_BOOTLOADER();
uint8_t sendTC_BL_SET_APPSW_CRC(uint16_t CRC_Value);
uint8_t sendTC_BL_ERASE_TEMP_APP_MEMORY();
uint8_t sendTC_BL_STORE_APPSW_CHUNK(uint32_t offset, uint32_t N, uint8_t* memoryValues);
uint8_t sendTC_BL_UPGRADE_FIRMWARE();
uint8_t sendTC_AGC_SET_THRESHOLDS(uint32_t threshold);
uint8_t sendTC_AGC_SET_DISABLED_SHUTTER_DUTY_CYCLE(uint16_t shutterDutyCycle);
uint8_t sendTC_AGC_SET_PIXEL_RANGE(uint32_t index);
uint8_t sendTC_AGC_SET_NUMBER_OF_MAXIMA_FOR_CONTROL_VALUE(uint8_t n);
uint8_t sendTC_AGC_SET_NUMBER_OF_PIXELS_FOR_BASELINE_VALUE(uint8_t n);
uint8_t sendTC_AGC_SET_STEP_FACTOR(uint8_t n);
uint8_t sendTC_SET_BASELINE_FIRST_PIXEL(uint8_t n);

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
uint8_t sendTelecommand(uint8_t commandCode, const uint8_t* params);
//uint8_t sendTelecommand(uint8_t commandCode, uint8_t parameters);

// Task methods to run receive and parsing concurrently
void receiveIncomingMPIFramesTask(void *pvParameters);
void processIncomingMPIFramesTask(void *pvParameters);
void StartMPIProcessingTasks();


#endif /* INC_MPICOMMANDHANDLING_H_ */
