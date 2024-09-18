#include "gps/gps.h"

#include <stdint.h>

#define BUFFER_SIZE 256
#define CRC32_POLYNOMIAL 0xEDB88320L

// TODO: Utilize the CRC functions below from the documentation to perform error checking on GPS response
// Refer to Page 56 of the OEM7 Commands and Logs Reference Manual

/// @brief Calculate a CRC value to be used by CRC calculation functions.
/// @param i
/// - Arg 0: variable name
/// @return 32 bit CRC Value
uint32_t GPS_crc32_value(uint8_t i) {
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
/// @return 32 bit CRC Value
uint32_t GPS_calculate_block_crc32( uint32_t ulCount, uint8_t *ucBuffer ) {
    uint32_t temp1;
    uint32_t temp2;
    uint32_t crc = 0;
    while ( ulCount-- != 0 ) {
        temp1 = ( crc >> 8 ) & 0x00FFFFFFL;
        temp2 = GPS_crc32_value( ((int) crc ^ *ucBuffer++ ) & 0xFF );
        crc = temp1 ^ temp2;
        }

    return( crc );
}
