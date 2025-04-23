//source: https://controllerstech.com/stm32-as-i2c-slave-part-1/
#include "main.h"
#include "stm32l4xx_hal_i2c.h"
#include "log/log.h"
#include "telecommand_exec/telecommand_parser.h"
#include "telecommand_exec/telecommand_executor.h"
#include "telecommand_exec/telecommand_types.h"
#include <memory.h>

/* @brief This file contains code for receiving data from the ax100. 
		This includes i2c interrupts and functions for parsing csp packets and scheduling them for execution as telecommands
*/

int rx_count = 0;
#define COMMS_RX_BUFF_SIZE 255
uint8_t COMMS_rx_buffer[COMMS_RX_BUFF_SIZE];

/// @brief Strips the csp packet header (first 4 bytes) from the packet.
/// @return pointer to the packet data without the header
static uint8_t* strip_csp_packet(uint8_t *packet, size_t packet_size) {
	//TODO: fix
	if (packet_size < 5) {return packet;} else {return packet + 4;}
}

/// @brief exctracts the data from a csp packet, parses it as a telecommand and adds it to the TCMD agenda.
/// @param packet csp packet
/// @return result, 0 on success 1 on failure 
static int32_t schedule_csp_packet_for_tcmd_execution(uint8_t * packet, size_t packet_size) {
	char* packet_data = (char*) strip_csp_packet( packet, packet_size);

	TCMD_parsed_tcmd_to_execute_t parsed_tcmd;
	uint8_t parse_result = TCMD_parse_full_telecommand(
		packet_data,TCMD_TelecommandChannel_RADIO1, &parsed_tcmd
	);
	if (parse_result != 0) {
		LOG_message(
			LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
			"Error parsing telecommand: %d", parse_result
		);
		return 1;
	}

	int32_t result = TCMD_add_tcmd_to_agenda(&parsed_tcmd);

	if (result != 0) {
		LOG_message(
			LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
			"Error adding telecommand to agenda: %ld", result
		);
		return 1;
	}
	return 0;
}

/// @brief A callback function that is called when transmission of the data from the master is complete (at the very end of transmission when the last byte is received!)
/// @param hi2c  the I2C handle
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{	
	COMMS_rx_buffer[COMMS_RX_BUFF_SIZE -1] = '\0'; // enusure the buffer is null terminated

	for (int i = 0; i < rx_count; i++) {
	LOG_message(
		LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
		"I2C listen complete, data: %x", COMMS_rx_buffer[i]);
	}
	schedule_csp_packet_for_tcmd_execution(COMMS_rx_buffer, rx_count);
	
	rx_count =0;
	HAL_I2C_EnableListen_IT(hi2c);
}

/// @brief A callback function that is called when the address of a transmission matches the address of the obc
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
	if (TransferDirection == I2C_DIRECTION_TRANSMIT)  // if the master wants to transmit the data
	{
		// The I2C_FIRST_FRAME implies that the first byte is to be received
		HAL_I2C_Slave_Seq_Receive_IT(hi2c, COMMS_rx_buffer + rx_count, 1, I2C_FIRST_FRAME);
		LOG_message(
			LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
			"first byte received"
		);
	}
	else  // if the master requests the data from the slave
	{
		return;
	}
}

/// @brief A callback function that is called when a single byte is successfully received
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	rx_count++;	
	if (COMMS_rx_buffer[rx_count -1] == '!' || rx_count >= COMMS_RX_BUFF_SIZE-1) {
		// if we have read the maximum number of bytes -1 , the I2C_LAST_FRAME indicates that the slave will not accept any more bytes after the next
		HAL_I2C_Slave_Seq_Receive_IT(hi2c, COMMS_rx_buffer + rx_count, 1, I2C_LAST_FRAME);
	}
	else {
		// The I2C_NEXT_FRAME implies that we are expecting the next byte of the same transfer
		HAL_I2C_Slave_Seq_Receive_IT(hi2c, COMMS_rx_buffer + rx_count, 1, I2C_NEXT_FRAME);
	}
}

/// @brief called when an i2c error occurs
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	LOG_message(
		LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
		"I2C error callback triggered, rx_count: %d", rx_count
	);
	rx_count = 0;
	HAL_I2C_EnableListen_IT(hi2c);
}
