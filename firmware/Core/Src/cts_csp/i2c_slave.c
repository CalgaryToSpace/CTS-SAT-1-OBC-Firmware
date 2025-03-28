//source: https://controllerstech.com/stm32-as-i2c-slave-part-1/
#include "main.h"
#include "cts_csp/i2c_slave.h"
#include "stm32l4xx_hal_i2c.h"
#include "log/log.h"
#include <telecommand_types.h>
#include "telecommands/telecommand_parser.h"
#include "telecommands/telecommand_executor.h"
#include <memory.h>

extern I2C_HandleTypeDef hi2c1;
#define RX_BUFF_SIZE 255
int rx_count = 0;
uint8_t rx_buffer[RX_BUFF_SIZE];

void strip_csp_packet(char *result, uint8_t *packet, size_t packet_size) {
	memcpy(result, packet + 4, packet_size - 4);
}

/// @brief exctracts the data from a csp packet, parses it as a telecommand and adds it to the agenda
/// @param packet raw csp packet
/// @return result, 0 on success 1 on failure 
int32_t schedule_csp_packet_data_for_tcmd_execution(uint8_t * packet, size_t packet_size) {
	char packet_data[255];
	strip_csp_packet(packet_data, rx_buffer, packet_size);

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
			"Error adding telecommand to agenda: %d", result
		);
		return 1;
	}
	return 0;
}


void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{	
	rx_buffer[RX_BUFF_SIZE -1] = '\0'; // enusure the buffer is null terminated

	for (int i = 0; i < rx_count; i++) {
	LOG_message(
		LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
		"I2C listen complete, data: %x", rx_buffer[i]);
	}
	schedule_csp_packet_data_for_tcmd_execution(rx_buffer, rx_count);
	
	rx_count =0;
	HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
	if (TransferDirection == I2C_DIRECTION_TRANSMIT)  // if the master wants to transmit the data
	{
		// receive using sequential function.
		// The I2C_FIRST_AND_LAST_FRAME implies that the slave will send a NACK after receiving "entered" num of bytes
		//TODO: is this the correct transfer mode? see hal_i2c.h file.
		HAL_I2C_Slave_Seq_Receive_IT(hi2c, rx_buffer + rx_count, 1, I2C_FIRST_FRAME);
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

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	rx_count++;
	if (rx_count < RX_BUFF_SIZE-1) {
		HAL_I2C_Slave_Seq_Receive_IT(hi2c, rx_buffer + rx_count, 1, I2C_NEXT_FRAME);
	}
	else {
		HAL_I2C_Slave_Seq_Receive_IT(hi2c, rx_buffer + rx_count, 1, I2C_LAST_FRAME);
	}
	
	LOG_message(
		LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
		"byte received, data:\n%s", rx_buffer
	);

		
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	rx_count = 0;
	LOG_message(
		LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
		"I2C error callback triggered");
	HAL_I2C_EnableListen_IT(hi2c);
}
