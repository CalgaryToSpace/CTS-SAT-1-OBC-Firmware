#include "adcs_drivers/adcs_types.h"
#include "adcs_drivers/adcs_command_ids.h"
#include "adcs_drivers/adcs_struct_packers.h"
#include "adcs_drivers/adcs_types_to_json.h"
#include "adcs_drivers/adcs_commands.h"
#include "adcs_drivers/adcs_internal_drivers.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "stm32l4xx_hal.h"

/* Basic Telecommand Functions */ 

/// @brief Sends a telecommand over I2C to the ADCS, checks that it's been acknowledged, and returns the ACK error flag.
/// @param[in] id Valid ADCS telecommand ID (see Firmware Reference Manual)
/// @param[in] data Data array to send the raw data bytes; length must be at least data_length (should contain the correct number of bytes for the given telecommand ID)
/// @param[in] data_length Length of the data array.
/// @param[in] include_checksum Tells the ADCS whether to use a CRC8 checksum; should be either ADCS_INCLUDE_CHECKSUM or ADCS_NO_CHECKSUM 
/// @return 0 if successful, 1 if invalid ID, 2 if incorrect parameter length, 3 if incorrect parameter value, and 4 if failed CRC
uint8_t ADCS_I2C_telecommand_wrapper(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum) {
	ADCS_CMD_Ack_Struct ack;
	uint8_t cmd_status;
	uint8_t num_checksum_tries = 0; // number of tries to resend command if checksum fails
    do {
		// Send telecommand
		cmd_status = ADCS_send_I2C_telecommand(id, data, data_length, include_checksum);

		// poll Acknowledge Telemetry Format until the Processed flag equals 1
		if (cmd_status == 0) {
			uint8_t ack_status;
			uint8_t num_ack_tries = 0;
			while (!ack.processed) {
				ack_status = ADCS_CMD_Ack(&ack); // confirm telecommand validity by checking the TC Error flag of the last read TC Acknowledge Telemetry Format.
				if (ack_status != 0 && ack_status != 4) {
					return ack_status; // there was an error in the command not related to checksum
				}
				num_ack_tries++;
				if (num_ack_tries > ADCS_PROCESSED_TIMEOUT) {
					return 5; // command failed to process in time
					// note: sending another telecommand when the first has not been processed
					// will result in an error in the next telecommand sent
					// as the ADCS telecommand buffer has been overrun/corrupted
				}
			}
		} else {
			return cmd_status; // if the HAL had an error, tell us what that is
		}
		num_checksum_tries++;
    } while (ack.error_flag == ADCS_ERROR_FLAG_CRC && num_checksum_tries < ADCS_CHECKSUM_TIMEOUT);  // if the checksum doesn't check out, keep resending the request

	return ack.error_flag; // if the HAL was successful and the ADCS command had an error, tell us what it is
}

/// @brief Sends a telemetry request over I2C to the ADCS, and resends repeatedly if the checksums don't match.
/// @param[in] id Valid ADCS telemetry request ID (see Firmware Reference Manual)
/// @param[in] include_checksum Tells the ADCS whether to use a CRC8 checksum; should be either ADCS_INCLUDE_CHECKSUM or ADCS_NO_CHECKSUM 
/// @param[in] data_length Length of the data array.
/// @param[out] data Data array to write the raw telemetry bytes to; length must be at least data_length (should contain the correct number of bytes for the given telemetry request ID)
/// @return 0 if successful, other numbers if the HAL failed to transmit or receive data. 
uint8_t ADCS_I2C_telemetry_wrapper(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum) {
	uint8_t checksum_check = ADCS_send_I2C_telemetry_request(id, data, data_length, include_checksum);
	uint8_t num_checksum_tries = 0;
	while (checksum_check == 4 && num_checksum_tries < ADCS_CHECKSUM_TIMEOUT) {
		// if the checksum doesn't check out, keep resending the request up to timeout
		checksum_check = ADCS_send_I2C_telemetry_request(id, data, data_length, include_checksum);
		num_checksum_tries++;
	}
	return checksum_check;
}


/// @brief Sends a telecommand over I2C to the ADCS.
/// @param[in] id Valid ADCS telecommand ID (see Firmware Reference Manual)
/// @param[in] data Data array to send the raw data bytes; length must be at least data_length (should contain the correct number of bytes for the given telecommand ID)
/// @param[in] data_length Length of the data array.
/// @param[in] include_checksum Tells the ADCS whether to use a CRC8 checksum; should be either ADCS_INCLUDE_CHECKSUM or ADCS_NO_CHECKSUM 
/// @return 0 if successful, 4 if the checksums don't match, other numbers if the HAL failed to transmit or receive data.
uint8_t ADCS_send_I2C_telecommand(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum) {
	uint8_t ADCS_CMD_status; 
	
	// allocate only required memory
	uint8_t buf[data_length + include_checksum]; // add additional bit for checksum if needed

	// fill buffer with data 
	for (uint8_t i = 0; i < data_length; i++) {
		buf[i] = data[i];
	}

	// include checksum following data if enabled
	if (include_checksum) {buf[data_length] = ADCS_COMMS_Crc8Checksum(data, data_length);}

	ADCS_CMD_status = HAL_I2C_Mem_Write(ADCS_I2C_HANDLE, ADCS_I2C_ADDRESS << 1, id, 1, buf, sizeof(buf), ADCS_HAL_TIMEOUT);

	/* When sending a command to the CubeACP, it is possible to include an 8-bit CRC checksum.
	For instance, when sending a command that has a length of 8 bytes, it is possible to include a
	9th byte that is computed from the previous 8 bytes. The extra byte will be interpreted as a
	checksum and used to validate the message. If the checksum fails, the command will be
	ignored. For I2C communication, the Tc Error Status in the Telecommand Acknowledge telemetry frame
	(Table 39: Telecommand Acknowledge Telemetry Format) will have a value of 4. */

	return ADCS_CMD_status;
}

/// @brief Sends a telemetry request over I2C to the ADCS.
/// @param[in] id Valid ADCS telemetry request ID (see Firmware Reference Manual)
/// @param[in] include_checksum Tells the ADCS whether to use a CRC8 checksum; should be either ADCS_INCLUDE_CHECKSUM or ADCS_NO_CHECKSUM 
/// @param[in] data_length Length of the data array.
/// @param[out] data Data array to write the raw telemetry bytes to; length must be at least data_length (should contain the correct number of bytes for the given telemetry request ID)
/// @return 0 if successful, 4 if the checksums don't match, other numbers if the HAL failed to transmit or receive data. 
uint8_t ADCS_send_I2C_telemetry_request(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum) {
	// Telemetry Request Format:
	// Note: requires a repeated start condition; data_length is number of bits to read.
	// [start], ADCS_I2C_WRITE_ADDRESS, id, [start] ADCS_I2C_READ_ADDRESS, [read all the data], [stop]
	// The defines in adcs_types.h already include the 7th bit of the ID to distinguish TLM and TC

	/* When requesting telemetry through I2C, it is possible to read one extra byte past the allowed
	length of the telemetry frame. In this case, the extra byte will also be an 8-bit checksum
	computed by the CubeACP and can be used by the interfacing OBC to validate the message.*/
	uint8_t adcs_tlm_status;

	// Allocate only required memory
	uint8_t temp_data[data_length + include_checksum];
		// temp data used for checksum checking

	adcs_tlm_status = HAL_I2C_Mem_Read(ADCS_I2C_HANDLE, ADCS_I2C_ADDRESS << 1, id, 1, temp_data, sizeof(temp_data), ADCS_HAL_TIMEOUT);

	for (uint8_t i = 0; i < data_length; i++) {
			// populate external data, except for checksum byte
			data[i] = temp_data[i];
	}

	if (include_checksum) {
		uint8_t checksum = temp_data[data_length];
		uint8_t checksum_test = ADCS_COMMS_Crc8Checksum(data, data_length);
		if (checksum != checksum_test) {
			return 0x04;
		}
	}

	return adcs_tlm_status;

}

uint8_t ADCS_send_UART_telecommand(UART_HandleTypeDef *huart, uint8_t id, uint8_t* data, uint32_t data_length) {
	// WARNING: DEPRECATED FUNCTION
	// This function is incomplete, and will not be updated.
	// USE AT YOUR OWN RISK.

	// Telemetry Request or Telecommand Format:
	// ADCS_UART_ESCAPE_BYTE, ADCS_UART_START_MESSAGE [uint8_t TLM/TC ID], ADCS_UART_ESCAPE_BYTE, ADCS_UART_END_MESSAGE
	// The defines in adcs_types.h already include the 7th bit of the ID to distinguish TLM and TC
	// data bytes can be up to a maximum of 8 bytes; data_length ranges from 0 to 8

	// Check id to identify if it's Telecommand or Telemetry Request
	uint8_t telemetry_request = id & 128; // 1 = TLM, 0 = TC

	// Allocate only required memory by checking first bit of ID
	uint8_t buf[5 + (!telemetry_request)*data_length];

	// Fill buffer with ESC, SOM and ID
	buf[0] = ADCS_UART_ESCAPE_BYTE;
	buf[1] = ADCS_UART_START_MESSAGE;
	buf[2] = id;

	if (telemetry_request) {
		// If transmitting Telemetry Request
		// Fill buffer with ESC and EOM without data_length
		buf[3] = ADCS_UART_ESCAPE_BYTE;
		buf[4] = ADCS_UART_END_MESSAGE;
	} else {
		// Fill buffer with Data if transmitting a Telecommand
		for (uint8_t i = 0; i < data_length; i++) {
			buf[i + 3] = data[i];
		}
		// Fill buffer with ESC and EOM
		buf[3 + data_length] = ADCS_UART_ESCAPE_BYTE;
		buf[4 + data_length] = ADCS_UART_END_MESSAGE;
	}

	// Transmit the TLM or TC via UART
	HAL_UART_Transmit(huart, buf, strlen((char*)buf), HAL_MAX_DELAY);

	// receiving from telecommand: data is one byte exactly
	// receiving from telemetry request: data is up to 8 bytes

	// Allocate only required memory
	uint8_t buf_rec[6 + (telemetry_request)*(data_length-1)];

	// Start receiving acknowledgment or reply from the CubeComputer
	HAL_UART_Receive(huart, buf_rec, strlen((char*)buf_rec), HAL_MAX_DELAY);

	if (telemetry_request) {
		// Ignoring ESC, EOM, SOM and storing the rest of the values in data
		for (uint8_t i = 3; i < sizeof(buf_rec)-2; i++) {
			// put the data into the data array excluding TC ID or TLM ID
			data[i-3] = buf_rec[i];
		}

		return 0x00;
	}

	return buf_rec[3]; // buf_rec[3] contains the TC Error Flag

  // The reply will contain two data bytes, the last one being the TC Error flag.
  // The receipt of the acknowledge will indicate that another telecommand may be sent.
  // Sending another telecommand before the acknowledge will corrupt the telecommand buffer.
}

/// @brief Swap low and high bytes of uint16 to turn into uint8 and put into specified index of an array
/// @param[in] value Value to split and swap the order of.
/// @param[in] index Index in array to write the result. (Array must contain at least two bytes, with index pointing to the first)
/// @param[out] array Data array to write the two bytes to at the specified index and index + 1.
/// @return 0 once complete.
uint8_t ADCS_switch_order(uint8_t *array, uint16_t value, int index) {
    array[index] = (uint8_t)(value & 0xFF); // Insert the low byte of the value into the array at the specified index
    array[index + 1] = (uint8_t)(value >> 8); // Insert the high byte of the value into the array at the next index
	return 0;
}

// Swap low and high bytes of uint32 to turn into uint8 and put into specified index of an array
/// @brief Swap low and high bytes of uint32 to turn into uint8 and put into specified index of an array
/// @param[in] value Value to split and swap the order of.
/// @param[in] index Index in array to write the result. (Array must contain at least four bytes, with index pointing to the first)
/// @param[out] array Data array to write the four bytes to at the specified index and the three subsequent indices.
/// @return 0 once complete.
uint8_t ADCS_switch_order_32(uint8_t *array, uint32_t value, int index) {
    array[index] = (uint8_t)(value & 0xFF); // Insert the low byte of the value into the array at the specified index
    array[index + 1] = (uint8_t)((value >> 8) & 0xFF); // Insert the second byte of the value into the array at the next index
	array[index + 2] = (uint8_t)((value >> 16) & 0xFF); // Insert the third byte of the value into the array at the next next index
    array[index + 3] = (uint8_t)(value >> 24); // Insert the high byte of the value into the array at the next next next index
	return 0;
}

/// @brief Initialise the lookup table for 8-bit CRC calculation.
/// @return 0 once successful.
uint8_t CRC8Table[256];
uint8_t ADCS_COMMS_Crc8Init()
	{
	int val;
	for (int i = 0; i < 256; i++)
	{
		val = i;
		for (int j = 0; j < 8; j++)
		{
			if (val & 1)
			val ^= ADCS_CRC_POLY;
			val >>= 1;
		}
		CRC8Table[i] = val;
	}
	return 0;
}

/// @brief Calculates an 8-bit CRC value
/// @param[in] buffer the buffer containing data for which to calculate the crc value
/// @param[in] len the number of bytes of valid data in the buffer
/// @return the CRC value calculated (which is 0xFF for an empty buffer)
uint8_t ADCS_COMMS_Crc8Checksum(uint8_t* buffer, uint16_t len)
{
	if (len == 0) return 0xff;

	uint16_t i;
	uint8_t crc = 0;

	for (i = 0; i < len; i++)
		crc = CRC8Table[crc ^ buffer[i]];

	return crc;
}