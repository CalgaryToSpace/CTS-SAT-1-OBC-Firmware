#include "gps/gps.h"
#include "gps/gps_types.h"
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

/// @brief Parse the received GPS header into a struct
/// @param data_received - The string obtained from the buffer that is to be parsed into the gps_response_header struct
/// @param result - gps_response_header struct that is returned
/// @return 0 if successful, >0 if an error occurred
uint8_t parse_gps_header(const char *data_received, gps_response_header *result){

    // TODO: What if there are multiple responses in the string?

    // Find the start and end of the header, which is # and ; resepectively
    const char *sync_char = strchr(data_received,'#');
    const char *delimiter_char = strchr(data_received,';');

    if (!sync_char || !delimiter_char) {
        // Invalid data: No header in gps response
        return 1; 
    }

    // Calculate the length of the header string
    const int header_length = delimiter_char - sync_char + 1;
    if (header_length < 0) {
        //Sync character occurs after delimiter character
        return 2;
    }

    char header_buffer[256];
    if ((size_t)header_length >= sizeof(header_buffer)) {
        //Header is too large for the buffer
        return 3;  
    }

    // Copy header string into a buffer
    strncpy(header_buffer, sync_char, header_length);
    header_buffer[header_length] = '\0';  // Null-terminate the substring

    // Parse the data in the header buffer

    //Extracting the log_name
    char *token;
    char *end_ptr;

    //TCMD_extract_string_arg TODO:Read and apply
    //Skip the reserved bytes etc
    token = strtok(header_buffer, ",");
    if (token && token[0] == '#') {  
        strcpy(result->log_name, token + 1);  
    }

    if (!(token = strtok(NULL, ","))){
        return 1;
    }
    strncpy(result->port, token, sizeof(result->port) - 1);
    result->port[sizeof(result->port) - 1] = '\0';

    if (!(token = strtok(NULL, ","))){
        return 1;
    }
    result->sequence_no = strtoul(token, &end_ptr, 10);
    if (*end_ptr != '\0') return 1;  // Error in conversion

    if (!(token = strtok(NULL, ","))){
        return 1;
    }
    result->idle_time = strtoul(token, &end_ptr, 10);

    if (!(token = strtok(NULL, ","))){
        return 1;
    }
    result->time_status.gps_reference_time_status_ascii = strdup(token);

    if (!(token = strtok(NULL, ","))){
        return 1;
    }
    result->week = strtoul(token, &end_ptr, 10);

    if (!(token = strtok(NULL, ","))){
        return 1;
    }
    result->seconds = strtoul(token, &end_ptr, 10);

    if (!(token = strtok(NULL, ","))){
        return 1;
    }
    result->rx_status = strtoul(token, &end_ptr, 16);  // hexadecimal

    if (!(token = strtok(NULL, ","))){
        return 1;
    }
    result->reserved = strtoul(token, &end_ptr, 16);  // hexadecimal

    if (!(token = strtok(NULL, ","))) return 1;
    result->rx_sw_version = strtoul(token, &end_ptr, 10);

    return 0;
}


/// @brief Parse Received Data
/// @param data_received - Number of bytes in the data block
/// @return 0 if successful, >0 if an error occurred
uint8_t parse_bestxyza_data(const char* data_received, bestxyza_response *result) {
    
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
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Header Field: %s",
            token
        );
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
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Data Field: %s",
            token
        );
        token = strtok(NULL, ",");
    }

    // Extract and check the CRC (you will need a CRC function to validate this)
    char crc[9];
    strncpy(crc, crc_start + 1, 8); // CRC is 8 characters long
    crc[8] = '\0';

    LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "CRC: %s.",
            crc
        );

    // Validate the CRC (implementation of the CRC function needed)
    // if (!validate_crc(data_start, crc)) {
    //     printf("CRC Validation Failed\n");
    //     return;
    // }

    LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Message Parsed Successfully."
        );

    return 0;
}

