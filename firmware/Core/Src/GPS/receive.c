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
	uint8_t *start = rec[0];
	while (start < sizeof(rec) / sizeof(rec[0]) && start == "\0")
	{
		if (*start == 0xAA && *(start + 1) == 0x44 && *(start + 2) == 0x12)
		{
			// HAL_UART_Transmit(&hlpuart1, &headerintro, strlen((char *)headerintro), HAL_MAX_DELAY);
			//		if(rec[3] == 28 && strlen(rec) > 28){
			header_status *header_ptr;
			pack_header(start, &header_ptr);

			// move to start of
			start += header_ptr->header_length;
			void *test_ptr;

			switch (header_ptr->msg_id)
			{
			case 241:

				gps_bestxyz_status *log_bestxyz_ptr;
				//		//SEE table 10 rec[3] is the header length
				//		uint8_t header[rec[3]] = rec;
				//		for(int i = 0; i < rec[3]; i++){
				//			header[i] = rec[i];
				//		}
				pack_gps_bestxyz(start, &log_bestxyz_ptr);
				// char sync_1_str[4];							  // Buffer to hold the string representation of sync_1
				// sprintf(sync_1_str, "%d", log_ptr->vel_type); // Convert sync_1 to hexadecimal string
				// HAL_UART_Transmit(&hlpuart1, (uint8_t *)sync_1_str, strlen(sync_1_str), HAL_MAX_DELAY);
				//		}
				test_ptr = log_bestxyz_ptr;

			case 101:
				gps_time_status *log_time_ptr;
				pack_gps_time(start, &log_time_ptr);
				test_ptr = log_bestxyz_ptr;
			case 2065:
				gps_itdetectstatus_entry_count *log_itcount_ptr;
				pack_gps_itdetectstatus(start, &log_itcount_ptr);
				test_ptr = log_bestxyz_ptr;
			case 93:
				gps_rxstatus *log_rxstat_ptr;
				pack_gps_rxstatus(start, &log_rxstat_ptr);
				test_ptr = log_rxstat_ptr;
			// dont know yet what I will be doing for this
			case 128:
				continue;
			}
			start += header_ptr->msg_length;
		}
		else
		{
			// need to somehow show that the logs are not being received properly
			break;
		}
		// CALL API AND STORE FROM REC[0] to Start in the api, get rid of rec after
	}

	return (void *)0;
}

void pack_header(const uint8_t rx_buff[], header_status **result_dest)
{
	//	memcpy(result_dest, rx_buff, sizeof(header_status));
	*result_dest = (header_status *)&rx_buff;
}
void print_header(const header_status **header)
{
	(*header)->msg_id;
}
// need to add check for if header is less than 25 cause then theres an error
// use the message id or type to determine which pack to call

/// NOT SURE WHAT I AM SUPPOSED TO BE DOING WITH THIS INFORMATION
void pack_gps_bestxyz(const uint8_t rx_buff[], gps_bestxyz_status **result_dest)
{
	*result_dest = (gps_bestxyz_status *)&rx_buff;
}

void print_bestxyz(const gps_bestxyz_status **log)
{
	(*log)->sol_age;
}

void pack_gps_time(const uint8_t rx_buff[], gps_time_status **result_dest)
{
	*result_dest = (gps_time_status *)&rx_buff;
}

void print_time(const gps_time_status **log)
{
	(*log)->utc_day;
}

void pack_gps_itdetectstatus(const uint8_t rx_buff[], gps_itdetectstatus_entry_count **result_dest)
{
	*result_dest = (gps_itdetectstatus_entry_count *)&rx_buff;
	itdetectstatus_entry *it_entries[(*result_dest)->num_entries];
	for (int i = 0; i < (*result_dest)->num_entries; i++)
	{
		// 40 is the total offset of itdetectstatus-entry see gps_types.h
		it_entries[i] = (itdetectstatus_entry *)&rx_buff[i * 40 + 1];
		print_it_etry(it_entries[i]);
	}
}
void print_itdetect(const gps_itdetectstatus_entry_count **log)
{
	(*log)->num_entries;
}

void print_itdetect_entry(const itdetectstatus_entry **log)
{
	(*log)->rf_path;
}

void pack_gps_rxstatus(const uint8_t rx_buff[], gps_rxstatus **result_dest)
{
	*result_dest = (gps_rxstatus *)&rx_buff;
}
void print_rxstat(const gps_rxstatus **log)
{
	(*log)->error;
}
// THIS IS OLD DONE BY OTHER MATT BEFORE DECIDED TO DO BINARY
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
