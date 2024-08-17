#include "gps/gps.h"
#include "gps/gps_types.h"
#include "log/log.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define BUFFER_SIZE 256

#define CRC32_POLYNOMIAL 0xEDB88320L

/* --------------------------------------------------------------------------
Calculate a CRC value to be used by CRC calculation functions.
-------------------------------------------------------------------------- */
uint32_t CRC32Value(uint8_t i) {
    int j;
    uint32_t ulCRC;
    ulCRC = i;
    
    for ( j = 8 ; j > 0; j-- ) {
        if ( ulCRC & 1 )
        ulCRC = ( ulCRC >> 1 ) ^ CRC32_POLYNOMIAL;
        else
        ulCRC >>= 1;
        }
    return ulCRC;
}
/* --------------------------------------------------------------------------
Calculates the CRC-32 of a block of data all at once
ulCount - Number of bytes in the data block
ucBuffer - Data block
-------------------------------------------------------------------------- */
uint32_t CalculateBlockCRC32( uint32_t ulCount, uint8_t *ucBuffer ) {
    uint32_t ulTemp1;
    uint32_t ulTemp2;
    uint32_t ulCRC = 0;
    while ( ulCount-- != 0 ) {
        ulTemp1 = ( ulCRC >> 8 ) & 0x00FFFFFFL;
        ulTemp2 = CRC32Value( ((int) ulCRC ^ *ucBuffer++ ) & 0xFF );
        ulCRC = ulTemp1 ^ ulTemp2;
        }

    return( ulCRC );
}

/// @brief Parse the received GPS header into a struct
/// @param data_received - The string obtained from the buffer that is to be parsed into the gps_response_header struct
/// @param result - gps_response_header struct that is returned
/// @return 0 if successful, >0 if an error occurred
uint8_t parse_gps_header(const char* data_received, gps_response_header *result){

    // Check if the first character is a #
    if( data_received[0] != "#"){

        // Invalid response starting string found
        return 1;
    }

}


/// @brief Parse Received Data
/// @param data_received - Number of bytes in the data block
/// @return 0 if successful, >0 if an error occurred
uint8_t parse_bestxyza_data(const char* data_received, gps_response_header *result) {
    
    //Check if it starts with #, if not return error
    if (data_received[0] != '#') {
        // Not a valid GPS message
        return 1;
    }

    // Find the next comma to denote the end of the function name. 
    // Probably loop through the implemented functions names to see if its valid.
    // Otherwise return an error.
    char* header_end = strchr(data_received, ';');
    if (!header_end) {
        // No header found
        return 2;
    }

    // Parse the header
    char header[BUFFER_SIZE];
    strncpy(header, data_received + 1, header_end - data_received - 1); // Skip the '#'
    header[header_end - data_received - 1] = '\0';

    // Tokenize the header to extract fields
    char* token = strtok(header, ",");
    while (token != NULL) {
        // Process each field
        printf("Header Field: %s\n", token);
        token = strtok(NULL, ",");
    }

    // Now handle the data portion after the header
    char* data_start = header_end + 1;
    char* crc_start = strchr(data_start, '*');
    if (!crc_start) {
        // No CRC found
        return 3;
    }

    // Extract data fields
    char data[BUFFER_SIZE];
    strncpy(data, data_start, crc_start - data_start);
    data[crc_start - data_start] = '\0';

    // Tokenize the data to extract fields
    token = strtok(data, ",");
    while (token != NULL) {
        // Process each data field
        printf("Data Field: %s\n", token);
        token = strtok(NULL, ",");
    }

    // Extract and check the CRC (you will need a CRC function to validate this)
    char crc[9];
    strncpy(crc, crc_start + 1, 8); // CRC is 8 characters long
    crc[8] = '\0';

    printf("CRC: %s\n", crc);

    // Validate the CRC (implementation of the CRC function needed)
    // if (!validate_crc(data_start, crc)) {
    //     printf("CRC Validation Failed\n");
    //     return;
    // }

    printf("Message Parsed Successfully\n");
}

int main() {
    const char* gps_message = "#GPGGA,COM1,0,55.5,SATTIME,2072,133140.000,02000000,58ba,15761;32,2072,136800,8b00602b57a606100004389101eefa4e0eeed24e012f216600007608cd27*1234ABCD";

    ParseGpsData(gps_message);

    return 0;
}
