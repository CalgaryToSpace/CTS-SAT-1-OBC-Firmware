/*
 * receive.ch
 *  Created on: Jul 4, 2022
 *      Author: mswel
 *  Updated on 2024-04-23
 *  Author: Matjaz Cigler
 */

#ifndef INC_RECEIVE_H_
#define INC_RECEIVE_H_
#define CRC32_POLYNOMIAL 0xEDB88320L

#include "main.h"
#include <string.h>
#include "stm32l4xx_hal.h"
#include "gps_types.h"

extern UART_HandleTypeDef hlpuart1;
extern UART_HandleTypeDef huart3;

enum Info{
	Sync = 0,
	Message = 1,
	Port = 2,
	Sequence = 3,
	IdleTime = 4,
	TimeStatus = 5,
	Week = 6,
	Seconds = 7,
	ReceiverStatus = 8,
	Reserved = 9,
	ReceiverSWVersion = 10
};

void receive_log(int wait);

void pack_header(const uint8_t rx_buff[], header_status **result_dest);
void pack_gps_bestxyz(const uint8_t rx_buff[], gps_bestxyz_status **result_dest, uint8_t H);

char** gpsParseReceive(char* received);

/* --------------------------------------------------------------------------
From OEM7_Commands_Logs_Manual.pdf
Calculate a CRC value to be used by CRC calculation functions.
-------------------------------------------------------------------------- */
unsigned long CRC32Value(int i);

/* --------------------------------------------------------------------------
From OEM7_Commands_Logs_Manual.pdf
Calculates the CRC-32 of a block of data all at once
ulCount - Number of bytes in the data block
ucBuffer - Data block
-------------------------------------------------------------------------- */
unsigned long CalculateBlockCRC32( unsigned long ulCount, unsigned char*ucBuffer );

#endif /* INC_RECEIVE_H_ */


