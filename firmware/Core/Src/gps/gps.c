#include "log/log.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define BUFFER_SIZE 256
#define CRC32_POLYNOMIAL 0xEDB88320L

/// @brief Calculate a CRC value to be used by CRC calculation functions.
/// @param i
/// - Arg 0: variable name
/// @return 0 if successful, >0 if an error occurred
uint32_t crc32_value(uint8_t i) {
    int j;
    uint32_t crc;
    crc = i;
    
    for ( j = 8 ; j > 0; j-- ) {
        if ( crc & 1 )
        crc = ( crc >> 1 ) ^ CRC32_POLYNOMIAL;
        else
        crc >>= 1;
        }
    return crc;
}

/// @brief Calculates the CRC-32 of a block of data all at once
/// @param ulCount - Number of bytes in the data block
/// @param ucBuffer - Data block
/// @return 0 if successful, >0 if an error occurred
uint32_t calculate_block_crc32( uint32_t ulCount, uint8_t *ucBuffer ) {
    uint32_t temp1;
    uint32_t temp2;
    uint32_t crc = 0;
    while ( ulCount-- != 0 ) {
        temp1 = ( crc >> 8 ) & 0x00FFFFFFL;
        temp2 = crc32_value( ((int) crc ^ *ucBuffer++ ) & 0xFF );
        crc = temp1 ^ temp2;
        }

    return( crc );
}



/// @brief Parse Received Data
/// @param buffer - Number of bytes in the data block
/// @return 0 if successful, >0 if an error occurred
void ParseGpsData(const char* buffer) {
    if (buffer[0] != '#') {
        // Not a valid GPS message
        return;
    }

    // Find the semicolon that marks the end of the header
    char* header_end = strchr(buffer, ';');
    if (!header_end) {
        // No header found
        return;
    }

    // Parse the header
    char header[BUFFER_SIZE];
    strncpy(header, buffer + 1, header_end - buffer - 1); // Skip the '#'
    header[header_end - buffer - 1] = '\0';

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
        return;
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
