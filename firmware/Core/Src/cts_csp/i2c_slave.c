//source: https://controllerstech.com/stm32-as-i2c-slave-part-1/
#include "main.h"
#include "cts_csp/i2c_slave.h"
#include "stm32l4xx_hal_i2c.h"
#include "log/log.h"

extern I2C_HandleTypeDef hi2c1;

int count = 0;
uint8_t framebuffer[FRAMEBUFFER_SIZE];

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
	HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
	if (TransferDirection == I2C_DIRECTION_TRANSMIT)  // if the master wants to transmit the data
	{
		// receive using sequential function.
		// The I2C_FIRST_AND_LAST_FRAME implies that the slave will send a NACK after receiving "entered" num of bytes
		//TODO: is this the correct transfer mode? see hal_i2c.h file.
		HAL_I2C_Slave_Seq_Receive_IT(hi2c, framebuffer, FRAMEBUFFER_SIZE, I2C_FIRST_AND_LAST_FRAME);
	}
	else  // if the master requests the data from the slave
	{
		return;
	}
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	framebuffer[255] = '\0';
	LOG_message(
		LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
		"Frame received, data:\n%s", framebuffer
	);

		
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	HAL_I2C_EnableListen_IT(hi2c);
}
