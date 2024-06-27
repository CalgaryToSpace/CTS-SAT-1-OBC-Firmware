/*
 * receive.c
 *
 *  Created on: Jul 4, 2022
 *      Author: mswel
 *
 * Updated on 2024-04-23
 *  Author: Matjaz Cigler
 */

#include "../../Inc/GPS/receive.h"
#include "../../Inc/GPS/gps_types.h"

// for now assuming that we are only getting one bestxyz log per second so just unpack that
// in the future need to get all the logs that are taken within that second and parse them apart need to first
//  - get their id then using that and the header length need to take however many bits are required, then need to
void receive_log(int wait)
{
	uint8_t recintro[] = "\r\nentire rec\r\n";
	uint8_t headerintro[] = "\r\nheader msg_id\r\n";
	uint8_t rec[1000];
	HAL_UART_Receive(&huart3, rec, 1000, wait);
	HAL_UART_Transmit(&hlpuart1, recintro, strlen((char *)recintro), HAL_MAX_DELAY);
	HAL_UART_Transmit(&hlpuart1, rec, 100, HAL_MAX_DELAY);
	//	HAL_UART_Transmit(&hlpuart1, recintro, sizeof(recintro), HAL_MAX_DELAY);

	//	HAL_UART_Transmit(&hlpuart1, rec, sizeof(rec), HAL_MAX_DELAY);
	// somehow here will eventually unpack based on the header length
	// SEE TABLE !): Binary Message Response structure (these are the sync bits)
	// extra note, how I understand it is that if the header size (including sync bits isnt 28 the gps has been
	// reconficured

	if (rec[0] == 0xAA && rec[1] == 0x44 && rec[2] == 0x12)
	{
		HAL_UART_Transmit(&hlpuart1, &headerintro, strlen((char *)headerintro), HAL_MAX_DELAY);
		//		if(rec[3] == 28 && strlen(rec) > 28){
		header_status *header_ptr;
		gps_bestxyz_status *log_ptr;
		//		//SEE table 10 rec[3] is the header length
		//		uint8_t header[rec[3]] = rec;
		//		for(int i = 0; i < rec[3]; i++){
		//			header[i] = rec[i];
		//		}
		pack_header(rec, &header_ptr);
		pack_gps_bestxyz(rec, &log_ptr, header_ptr->header_length);
		char sync_1_str[4];							  // Buffer to hold the string representation of sync_1
		sprintf(sync_1_str, "%d", log_ptr->vel_type); // Convert sync_1 to hexadecimal string
		HAL_UART_Transmit(&hlpuart1, (uint8_t *)sync_1_str, strlen(sync_1_str), HAL_MAX_DELAY);
		//		}
	}

	return (void *)0;
}

void pack_header(const uint8_t rx_buff[], header_status **result_dest)
{
	//	memcpy(result_dest, rx_buff, sizeof(header_status));
	*result_dest = (header_status *)rx_buff;
}
// need to add check for if header is less than 25 cause then theres an error
// use the message id or type to determine which pack to call
void pack_gps_bestxyz(const uint8_t rx_buff[], gps_bestxyz_status **result_dest, uint8_t H)
{
	*result_dest = (header_status *)&rx_buff[H];
	//	result_dest->sync_1 = rx_buff[0];  // Sync_1: 8-bit value at offset 0
	//	result_dest->sync_2 = rx_buff[1];  // Sync_2: 8-bit value at offset 1
	//	result_dest->sync_3 = rx_buff[2];  // Sync_3: 8-bit value at offset 2
	//	result_dest->header_length = rx_buff[3];  // Header length: 8-bit value at offset 3
	//	result_dest->msg_id = (rx_buff[5] << 8) | rx_buff[4];  // Message ID: 16-bit value at offset 4
	//	result_dest->msg_type = rx_buff[6];  // Message type: 8-bit value at offset 6
	//	result_dest->prt_addr = rx_buff[7];  // Port address: 8-bit value at offset 7
	//	result_dest->msg_length = (rx_buff[9] << 8) | rx_buff[8];  // Message length: 16-bit value at offset 8
	//	result_dest->sequence = (rx_buff[11] << 8) | rx_buff[10];  // Sequence: 16-bit value at offset 10
	//	result_dest->idle_time = rx_buff[12];  // Idle time: 8-bit value at offset 12
	//	result_dest->time_status = (GPS_TIME_STATUS_enum_t)rx_buff[12];  // Time status: Enum value at offset 12
	//	result_dest->week = (rx_buff[14] << 8) | rx_buff[13];  // GPS reference week number: 16-bit value at offset 14
	//	result_dest->ms = (rx_buff[18] << 24) | (rx_buff[17] << 16) | (rx_buff[16] << 8) | rx_buff[15];  // ms GPSEC: 32-bit value at offset 16
	//	result_dest->rcvr_status = (rx_buff[22] << 24) | (rx_buff[21] << 16) | (rx_buff[20] << 8) | rx_buff[19];  // Receiver status: 32-bit value at offset 20
	//	result_dest->reserved = (rx_buff[24] << 8) | rx_buff[23];  // Reserved: 16-bit value at offset 24
	//	result_dest->rcvr_version = (rx_buff[26] << 8) | rx_buff[25];  // Receiver version: 16-bit value at offset 26
	//	result_dest->psol_status = (rx_buff[H + 8] << 24) | (rx_buff[H + 7] << 16) | (rx_buff[H + 6] << 8) | rx_buff[H + 5];  // P-sol status: 32-bit value at offset H
	//	result_dest->pos_type = (rx_buff[H + 12] << 24) | (rx_buff[H + 11] << 16) | (rx_buff[H + 10] << 8) | rx_buff[H + 9];  // Position type: 32-bit value at offset H+4
	//	result_dest->p_x = ((int64_t)rx_buff[H + 16] << 56) | ((int64_t)rx_buff[H + 15] << 48) | ((int64_t)rx_buff[H + 14] << 40) | ((int64_t)rx_buff[H + 13] << 32) |
	//	                    ((int64_t)rx_buff[H + 12] << 24) | ((int64_t)rx_buff[H + 11] << 16) | ((int64_t)rx_buff[H + 10] << 8) | rx_buff[H + 9];  // Position coord X: 64-bit value at offset H+8
	//	result_dest->p_y = ((int64_t)rx_buff[H + 24] << 56) | ((int64_t)rx_buff[H + 23] << 48) | ((int64_t)rx_buff[H + 22] << 40) | ((int64_t)rx_buff[H + 21] << 32) |
	//	                    ((int64_t)rx_buff[H + 20] << 24) | ((int64_t)rx_buff[H + 19] << 16) | ((int64_t)rx_buff[H + 18] << 8) | rx_buff[H + 17];  // Position coord Y: 64-bit value at offset H+16
	//	result_dest->p_z = ((int64_t)rx_buff[H + 32] << 56) | ((int64_t)rx_buff[H + 31] << 48) | ((int64_t)rx_buff[H + 30] << 40) | ((int64_t)rx_buff[H + 29] << 32) |
	//	                    ((int64_t)rx_buff[H + 28] << 24) | ((int64_t)rx_buff[H + 27] << 16) | ((int64_t)rx_buff[H + 26] << 8) | rx_buff[H + 25];  // Position coord Z: 64-bit value at offset H+24
	//	result_dest->p_x_omega = (rx_buff[H + 40] << 24) | (rx_buff[H + 39] << 16) | (rx_buff[H + 38] << 8) | rx_buff[H + 37];  // Std dev P-X: 32-bit value at offset H+32
	//	result_dest->p_y_omega = (rx_buff[H + 44] << 24) | (rx_buff[H + 43] << 16) | (rx_buff[H + 42] << 8) | rx_buff[H + 41];  // Std dev P-Y: 32-bit value at offset H+36
	//	result_dest->p_z_omega = (rx_buff[H + 48] << 24) | (rx_buff[H + 47] << 16) | (rx_buff[H + 46] << 8) | rx_buff[H + 45];  // Std dev P-Z: 32-bit value at offset H+40
	//	result_dest->vsol_status = (rx_buff[H + 52] << 24) | (rx_buff[H + 51] << 16) | (rx_buff[H + 50] << 8) | rx_buff[H + 49];  // V-sol status: 32-bit value at offset H+44
	//	result_dest->vel_type = (rx_buff[H + 58] << 8) | rx_buff[H + 57];  // Velocity type: 16-bit value at offset H+48
	//	result_dest->v_x = ((int64_t)rx_buff[H + 64] << 56) | ((int64_t)rx_buff[H + 63] << 48) | ((int64_t)rx_buff[H + 62] << 40) | ((int64_t)rx_buff[H + 61] << 32) |
	//	                   ((int64_t)rx_buff[H + 60] << 24) | ((int64_t)rx_buff[H + 59] << 16) | ((int64_t)rx_buff[H + 58] << 8) | rx_buff[H + 57];  // Velocity coord X: 64-bit value at offset H+52
	//	result_dest->v_y = ((int64_t)rx_buff[H + 72] << 56) | ((int64_t)rx_buff[H + 71] << 48) | ((int64_t)rx_buff[H + 70] << 40) | ((int64_t)rx_buff[H + 69] << 32) |
	//	                   ((int64_t)rx_buff[H + 68] << 24) | ((int64_t)rx_buff[H + 67] << 16) | ((int64_t)rx_buff[H + 66] << 8) | rx_buff[H + 65];  // Velocity coord Y: 64-bit value at offset H+60
	//	result_dest->v_z = ((int64_t)rx_buff[H + 80] << 56) | ((int64_t)rx_buff[H + 79] << 48) | ((int64_t)rx_buff[H + 78] << 40) | ((int64_t)rx_buff[H + 77] << 32) |
	//	                   ((int64_t)rx_buff[H + 76] << 24) | ((int64_t)rx_buff[H + 75] << 16) | ((int64_t)rx_buff[H + 74] << 8) | rx_buff[H + 73];  // Velocity coord Z: 64-bit value at offset H+68
	//	result_dest->v_x_omega = (rx_buff[H + 84] << 24) | (rx_buff[H + 83] << 16) | (rx_buff[H + 82] << 8) | rx_buff[H + 81];  // Std dev V-X: 32-bit value at offset H+76
	//	result_dest->v_y_omega = (rx_buff[H + 88] << 24) | (rx_buff[H + 87] << 16) | (rx_buff[H + 86] << 8) | rx_buff[H + 85];  // Std dev V-Y: 32-bit value at offset H+80
	//	result_dest->v_z_omega = (rx_buff[H + 92] << 24) | (rx_buff[H + 91] << 16) | (rx_buff[H + 90] << 8) | rx_buff[H + 89];  // Std dev V-Z: 32-bit value at offset H+84
	//	result_dest->stn_id[0] = rx_buff[H + 96];  // Base station identification: Character 1 at offset H+88
	//	result_dest->stn_id[1] = rx_buff[H + 97];  // Base station identification: Character 2 at offset H+89
	//	result_dest->stn_id[2] = rx_buff[H + 98];  // Base station identification: Character 3 at offset H+90
	//	result_dest->stn_id[3] = rx_buff[H + 99];  // Base station identification: Character 4 at offset H+91
	//	result_dest->v_latency = (rx_buff[H + 100] << 24) | (rx_buff[H + 101] << 16) | (rx_buff[H + 102] << 8) | rx_buff[H + 103];  // Velocity time tag: 32-bit value at offset H+92
	//	result_dest->diff_age = (rx_buff[H + 104] << 24) | (rx_buff[H + 105] << 16) | (rx_buff[H + 106] << 8) | rx_buff[H + 107];  // Differential age: 32-bit value at offset H+96
	//	result_dest->sol_age = (rx_buff[H + 108] << 24) | (rx_buff[H + 109] << 16) | (rx_buff[H + 110] << 8) | rx_buff[H + 111];  // Solution age: 32-bit value at offset H+100
	//	result_dest->num_svs = rx_buff[H + 112];  // Number of satellites tracked: 8-bit value at offset H+104
	//	result_dest->num_soln_svs = rx_buff[H + 113];  // Number of satellites used in solution: 8-bit value at offset H+105
	//	result_dest->num_ggL1 = rx_buff[H + 114];  // Number of satellites with L1/E1/B1 signals used in solution: 8-bit value at offset H+106
	//	result_dest->solnMultiSVs = rx_buff[H + 115];  // Number of satellites with multi-frequency signals used in solution: 8-bit value at offset H+107
	//	result_dest->reserved2 = rx_buff[H + 116];  // Reserved: 8-bit value at offset H+108
	//	result_dest->ext_sol_stat = rx_buff[H + 117];  // Extended solution status: 8-bit value at offset H+109
	//	result_dest->gal_beidou_mask = rx_buff[H + 118];  // Galileo and BeiDou signals used mask: 8-bit value at offset H+110
	//	result_dest->gps_glonass_mask = rx_buff[H + 119];  // GPS and GLONASS signals used mask: 8-bit value at offset H+111
	//	result_dest->crc[0] = rx_buff[H + 120];  // 32-bit CRC (byte 0): 8-bit value at offset H+112
	//	result_dest->crc[1] = rx_buff[H + 121];  // 32-bit CRC (byte 1): 8-bit value at offset H+113
	//	result_dest->crc[2] = rx_buff[H + 122];  // 32-bit CRC (byte 2): 8-bit value at offset H+114
	//	result_dest->crc[3] = rx_buff[H + 123];  // 32-bit CRC (byte 3): 8-bit value at offset H+115
	//
	//
	//
	//
}

char **gpsParseReceive(char *received)
{
	if (received[0] != '#')
	{ // Messages should start with "#'
		return (void *)0;
	}

	char **toReturn = (char **)malloc(sizeof(char *) * 11); // Allocate mem for fields of the response
	char *current = received + 1;							// Do not include the starting "#"
	int cursor = 0;
	int index = 0;
	int fieldStart = 0;

	// Loop through all the characters of the message.
	// Once a segment is passed, look back to see how
	// big it was and then add all the member characters
	// to the return variable.

	while ((*current) != '*')
	{ // "*" indicates the end of the fields
		if ((*current) == ',' || (*current) == ';')
		{ // Fields are seperated by "," or ";"
			toReturn[index] = (char *)malloc((sizeof(char) * (cursor - fieldStart)) + 1);
			int j = 0;
			for (int i = fieldStart; i < cursor; i++)
			{
				toReturn[index][j] = received[i];
				j++;
			}
			toReturn[j] = '\n';
			fieldStart = cursor + 1;
			cursor += 1;
			index += 1;
		}
		else
		{
			cursor += 1;
		}
	}
	// TODO - implement CRC check

	return toReturn;
}

unsigned long CRC32Value(int i)
{
	int j;
	unsigned long ulCRC;
	ulCRC = i;
	for (j = 8; j > 0; j--)
	{
		if (ulCRC & 1)
		{
			ulCRC = (ulCRC >> 1) ^ CRC32_POLYNOMIAL;
		}
		else
		{
			ulCRC >>= 1;
		}
	}
	return ulCRC;
}

unsigned long CalculateBlockCRC32(unsigned long ulCount, unsigned char *ucBuffer)
{
	unsigned long ulTemp1;
	unsigned long ulTemp2;
	unsigned long ulCRC = 0;
	while (ulCount-- != 0)
	{
		ulTemp1 = (ulCRC >> 8) & 0x00FFFFFFL;
		ulTemp2 = CRC32Value(((int)ulCRC ^ *ucBuffer++) & 0xFF);
		ulCRC = ulTemp1 ^ ulTemp2;
	}
	return (ulCRC);
}
