#include "gps/gps.h"
#include "gps/gps_types.h"
#include "log/log.h"
#include "telecommands/telecommand_args_helpers.h"
#include "transforms/arrays.h"

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <stdint.h>

#define BUFFER_SIZE 256
#define CRC32_POLYNOMIAL 0xEDB88320L

// TODO: Utilize the CRC functions below from the documentation to perform error checking on GPS response
// Refer to Page 56 of the OEM7 Commands and Logs Reference Manual

/// @brief Calculate a CRC value to be used by CRC calculation functions.
/// @param i
/// - Arg 0: variable name
/// @return 32 bit CRC Value
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
/// @return 32 bit CRC Value
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


/// @brief Assigns a GPS time status based on the provided string.
/// @param status_str The status string to parse.
/// @param status Pointer to GPS_reference_time_status_t where the status will be stored.
/// @return Returns 0 on success, 1 if the status string is unrecognized.
uint8_t assign_gps_time_status(const char *status_str, GPS_reference_time_status_t *status) {
    if (strcmp(status_str, "UNKNOWN") == 0) {
        *status = GPS_UNKNOWN;
    } else if (strcmp(status_str, "APPROXIMATE") == 0) {
        *status = GPS_APPROXIMATE;
    } else if (strcmp(status_str, "COARSEADJUSTING") == 0) {
        *status = GPS_COARSEADJUSTING;
    } else if (strcmp(status_str, "COARSE") == 0) {
        *status = GPS_COARSE;
    } else if (strcmp(status_str, "COARSESTEERING") == 0) {
        *status = GPS_COARSESTEERING;
    } else if (strcmp(status_str, "FREEWHEELING") == 0) {
        *status = GPS_FREEWHEELING;
    } else if (strcmp(status_str, "FINEADJUSTING") == 0) {
        *status = GPS_FINEADJUSTING;
    } else if (strcmp(status_str, "FINE") == 0) {
        *status = GPS_FINE;
    } else if (strcmp(status_str, "FINEBACKUPSTEERING") == 0) {
        *status = GPS_FINEBACKUPSTEERING;
    } else if (strcmp(status_str, "FINESTEERING") == 0) {
        *status = GPS_FINESTEERING;
    } else if (strcmp(status_str, "SATTIME") == 0) {
        *status = GPS_SATTIME;
    } else {
        return 1;  // Unrecognized status string
    }
    return 0;  // Success
}


/// @brief Parse the received GPS header into a struct
/// @param data_received - The string obtained from the buffer that is to be parsed into the gps_response_header struct
/// @param result - gps_response_header struct that is returned
/// @return 0 if successful, > 0 if an error occurred
uint8_t parse_gps_header(const char *data_received, gps_response_header *result){

    // TODO: What if there are multiple responses in the string?

    // Check if the buffer is empty
    if (data_received == NULL || data_received[0] == '\0') {
        // Empty or NULL string, return an error
        return 1;
    }

    // Find the start and end of the header, which is # and ; resepectively
    const char *sync_char = strchr(data_received,'#');
    const char *delimiter_char = strchr(data_received,';');

    if (!sync_char || !delimiter_char) {
        // Invalid data: No header in gps response
        return 2; 
    }

    // Calculate the length of the header string
    const int header_length = delimiter_char - sync_char + 1;
    if (header_length < 0) {
        //Sync character occurs after delimiter character
        return 3;
    }

    char header_buffer[256];
    if ((size_t)header_length >= sizeof(header_buffer)) {
        //Header is too large for the buffer
        return 4;  
    }

    // Copy header string into a buffer
    strncpy(header_buffer, sync_char, header_length);
    header_buffer[header_length] = '\0';  // Null-terminate the substring

    // Parse the data in the header buffer
    uint8_t parse_result;
    char token_buffer[128];

    // Log Name
    parse_result = TCMD_extract_string_arg(header_buffer, 0, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    strcpy(result->log_name, token_buffer + 1);

    // Time Status
    parse_result = TCMD_extract_string_arg(header_buffer, 4, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    const uint8_t status_result = assign_gps_time_status(token_buffer, &result->time_status);
    if (status_result != 0) {
        // Time Status not recognized
        return status_result;
    }

    return 0;
}


/// @brief Assigns a GPS solution status based on the provided string.
/// @param status_str The status string to parse.
/// @param status Pointer to GPS_solution_status_enum_t where the status will be stored.
/// @return Returns 0 on success, 1 if the status string is unrecognized.
uint8_t assign_gps_solution_status(const char *status_str, GPS_solution_status_enum_t *status) {
    if (strcmp(status_str, "SOL_COMPUTED") == 0) {
        *status = GPS_SOL_COMPUTED;
    } else if (strcmp(status_str, "INSUFFICIENT_OBS") == 0) {
        *status = GPS_INSUFFICIENT_OBS;
    } else if (strcmp(status_str, "NO_CONVERGENCE") == 0) {
        *status = GPS_NO_CONVERGENCE;
    } else if (strcmp(status_str, "SINGULARITY") == 0) {
        *status = GPS_SINGULARITY;
    } else if (strcmp(status_str, "COV_TRACE") == 0) {
        *status = GPS_COV_TRACE;
    } else if (strcmp(status_str, "TEST_DIST") == 0) {
        *status = GPS_TEST_DIST;
    } else if (strcmp(status_str, "COLD_START") == 0) {
        *status = GPS_COLD_START;
    } else if (strcmp(status_str, "V_H_LIMIT") == 0) {
        *status = GPS_V_H_LIMIT;
    } else if (strcmp(status_str, "VARIANCE") == 0) {
        *status = GPS_VARIANCE;
    } else if (strcmp(status_str, "RESIDUALS") == 0) {
        *status = GPS_RESIDUALS;
    } else if (strcmp(status_str, "INTEGRITY_WARNING") == 0) {
        *status = GPS_INTEGRITY_WARNING;
    } else {
        return 1;  // Unrecognized status string
    }
    return 0;  // Success
}


/// @brief Assigns a GPS position or velocity type based on the provided string.
/// @param type_str The type string to parse.
/// @param type Pointer to GPS_position_velocity_type_enum_t where the type will be stored.
/// @return Returns 0 on success, 1 if the type string is unrecognized.
uint8_t assign_gps_position_velocity_type(const char *type_str, GPS_position_type_enum_t *type) {
    if (strcmp(type_str, "NONE") == 0) {
        *type = GPS_TYPE_NONE;
    } else if (strcmp(type_str, "FIXEDPOS") == 0) {
        *type = GPS_TYPE_FIXEDPOS;
    } else if (strcmp(type_str, "FIXEDHEIGHT") == 0) {
        *type = GPS_TYPE_FIXEDHEIGHT;
    } else if (strcmp(type_str, "DOPPLER_VELOCITY") == 0) {
        *type = GPS_TYPE_DOPPLER_VELOCITY;
    } else if (strcmp(type_str, "SINGLE") == 0) {
        *type = GPS_TYPE_SINGLE;
    } else if (strcmp(type_str, "PSDIFF") == 0) {
        *type = GPS_TYPE_PSDIFF;
    } else if (strcmp(type_str, "WAAS") == 0) {
        *type = GPS_TYPE_WAAS;
    } else if (strcmp(type_str, "PROPAGATED") == 0) {
        *type = GPS_TYPE_PROPAGATED;
    } else if (strcmp(type_str, "L1_FLOAT") == 0) {
        *type = GPS_TYPE_L1_FLOAT;
    } else if (strcmp(type_str, "NARROW_FLOAT") == 0) {
        *type = GPS_TYPE_NARROW_FLOAT;
    } else if (strcmp(type_str, "L1_INT") == 0) {
        *type = GPS_TYPE_L1_INT;
    } else if (strcmp(type_str, "WIDE_INT") == 0) {
        *type = GPS_TYPE_WIDE_INT;
    } else if (strcmp(type_str, "NARROW_INT") == 0) {
        *type = GPS_TYPE_NARROW_INT;
    } else if (strcmp(type_str, "RTK_DIRECT_INS") == 0) {
        *type = GPS_TYPE_RTK_DIRECT_INS;
    } else if (strcmp(type_str, "INS_SBAS") == 0) {
        *type = GPS_TYPE_INS_SBAS;
    } else if (strcmp(type_str, "INS_PSRSP") == 0) {
        *type = GPS_TYPE_INS_PSRSP;
    } else if (strcmp(type_str, "INS_PSRDIFF") == 0) {
        *type = GPS_TYPE_INS_PSRDIFF;
    } else if (strcmp(type_str, "INS_RTKFLOAT") == 0) {
        *type = GPS_TYPE_INS_RTKFLOAT;
    } else if (strcmp(type_str, "INS_RTKFIXED") == 0) {
        *type = GPS_TYPE_INS_RTKFIXED;
    } else if (strcmp(type_str, "PPP_CONVERGING") == 0) {
        *type = GPS_TYPE_PPP_CONVERGING;
    } else if (strcmp(type_str, "PPP") == 0) {
        *type = GPS_TYPE_PPP;
    } else if (strcmp(type_str, "OPERATIONAL") == 0) {
        *type = GPS_TYPE_OPERATIONAL;
    } else if (strcmp(type_str, "WARNING") == 0) {
        *type = GPS_TYPE_WARNING;
    } else if (strcmp(type_str, "OUT_OF_BOUNDS") == 0) {
        *type = GPS_TYPE_OUT_OF_BOUNDS;
    } else if (strcmp(type_str, "INS_PPP_CONVERGING") == 0) {
        *type = GPS_TYPE_INS_PPP_CONVERGING;
    } else if (strcmp(type_str, "INS_PPP") == 0) {
        *type = GPS_TYPE_INS_PPP;
    } else if (strcmp(type_str, "PPP_BASIC_CONVERGING") == 0) {
        *type = GPS_TYPE_PPP_BASIC_CONVERGING;
    } else if (strcmp(type_str, "PPP_BASIC") == 0) {
        *type = GPS_TYPE_PPP_BASIC;
    } else if (strcmp(type_str, "INS_PPP_BASIC_CONVERGING") == 0) {
        *type = GPS_TYPE_INS_PPP_BASIC_CONVERGING;
    } else if (strcmp(type_str, "INS_PPP_BASIC") == 0) {
        *type = GPS_TYPE_INS_PPP_BASIC;
    } else {
        return 1;  // Unrecognized type string
    }
    return 0;  // Success
}

/// @brief Parse Received Data
/// @param data_received - Number of bytes in the data block
/// @return 0 if successful, >0 if an error occurred
uint8_t parse_bestxyza_data(const char* data_received, gps_bestxyza_response *result) {

    // Check if the buffer is empty
    if (data_received[0] == '\0') {
        // Empty or NULL string, return an error
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 1: Empty buffer"
        );
        return 1;
    }

    gps_response_header bestxyza_header;
    const uint8_t header_parse_result = parse_gps_header(data_received,&bestxyza_header);

    if(header_parse_result != 0){
        // Error in parsing header section
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 2: Error parising the gps header"
        );
        return 2;
    }

    if(strcmp(bestxyza_header.log_name, "BESTXYZA") != 0){
        // Incorrect log function
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 3: Incorrect log function ie not BESTXYZA"
        );
        return 3;
    }

    // TODO: What if there is multiple commands in the string?
    const char *header_delimiter_char = strchr(data_received,';');
    const char* bestxyza_data_start = header_delimiter_char + 1;
    const char* asterisk = strchr(bestxyza_data_start, '*');  

    if(strcmp(bestxyza_data_start, "\0") == 0){
        // No data after the gps header within the response
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 4: Missing Data after the header"
        );
        return 4;
    }  

    if(!asterisk){
        // No terminator at the end of the bestxyza data, ie no CRC present
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 5: Missing Astericks in response"
        );
        return 5;
    }

    const int bestxyza_data_length = asterisk - bestxyza_data_start + 1;
    char bestxyza_data_buffer[512];
    if ((size_t)bestxyza_data_length >= sizeof(bestxyza_data_buffer)) {
        //Buffer Overflow Error
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 6: Buffer overflow"
        );
        return 6;  
    }

    strncpy(bestxyza_data_buffer, bestxyza_data_start, bestxyza_data_length);
    bestxyza_data_buffer[bestxyza_data_length] = '\0';

    // Parse the data in the bestxyza data buffer
    uint8_t parse_result;
    char token_buffer[128];
    char *end_ptr;
    
    // Position Solution Status
    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 0, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    uint8_t status_result = assign_gps_solution_status(token_buffer, &result->position_solution_status);
    if(status_result != 0){
        // Invalid string passed
        return status_result;
    }

    // Position Type
    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 1, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    status_result = assign_gps_position_velocity_type(token_buffer, &result->position_type);
    if(status_result != 0){
        // Invalid string passed
        return status_result;
    }

    // Position Coordinates 
    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 2, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }

    double value = strtod(token_buffer, &end_ptr);
    if(strcmp(end_ptr, token_buffer) == 0){
        // Improper string to double conversion
        return 7;
    }
    double conv_result = value * 1000;

    //TODO: Determine what to do when we exceed data type value
    if(conv_result > INT64_MAX){
        conv_result = INT64_MAX;
    }
    else if(conv_result < INT64_MIN){
        conv_result = INT64_MIN;
    }
    result->position_x_mm = (int64_t) conv_result;

    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 3, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    value = strtod(token_buffer, &end_ptr);
    if(strcmp(end_ptr, token_buffer) == 0){
        // Improper string to double conversion
        return 7;
    }
    conv_result = value * 1000;

    //TODO: Determine what to do when we exceed data type value
    if(conv_result > INT64_MAX){
        conv_result = INT64_MAX;
    }
    else if(conv_result < INT64_MIN){
        conv_result = INT64_MIN;
    }
    result->position_y_mm = (int64_t) conv_result;

    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 4, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    value = strtod(token_buffer, &end_ptr);
    if(strcmp(end_ptr, token_buffer) == 0){
        // Improper string to double conversion
        return 7;
    }
    conv_result = value * 1000;

    //TODO: Determine what to do when we exceed data type value
    if(conv_result > INT64_MAX){
        conv_result = INT64_MAX;
    }
    else if(conv_result < INT64_MIN){
        conv_result = INT64_MIN;
    }
    result->position_z_mm = (int64_t) conv_result;

    // Position Coordinates Standard Deviation
    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 5, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    value = strtod(token_buffer, &end_ptr);
    if(strcmp(end_ptr, token_buffer) == 0){
        // Improper string to double conversion
        return 7;
    }
    conv_result = value * 1000;

    //TODO: Determine what to do when we exceed data type value
    if(conv_result > INT32_MAX){
        conv_result = INT32_MAX;
    }
    else if(conv_result < INT32_MIN){
        conv_result = INT32_MIN;
    }
    result->position_x_std_mm = (int32_t) conv_result;

    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 6, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    value = strtod(token_buffer, &end_ptr);
    if(strcmp(end_ptr, token_buffer) == 0){
        // Improper string to double conversion
        return 7;
    }
    conv_result = value * 1000;

    //TODO: Determine what to do when we exceed data type value
    if(conv_result > INT32_MAX){
        conv_result = INT32_MAX;
    }
    else if(conv_result < INT32_MIN){
        conv_result = INT32_MIN;
    }
    result->position_y_std_mm = (int32_t) conv_result;

    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 7, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    value = strtod(token_buffer, &end_ptr);
    if(strcmp(end_ptr, token_buffer) == 0){
        // Improper string to double conversion
        return 7;
    }
    conv_result = value * 1000;

    //TODO: Determine what to do when we exceed data type value
    if(conv_result > INT32_MAX){
        conv_result = INT32_MAX;
    }
    else if(conv_result < INT32_MIN){
        conv_result = INT32_MIN;
    }
    result->position_z_std_mm = (int32_t) conv_result;

    // Differential Age
    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 18, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    value = strtod(token_buffer, &end_ptr);
    if(strcmp(end_ptr, token_buffer) == 0){
        // Improper string to double conversion
        return 7;
    }
    conv_result = value * 1000;

    //TODO: Determine what to do when we exceed data type value
    if(conv_result > INT64_MAX){
        conv_result = INT64_MAX;
    }
    else if(conv_result < INT64_MIN){
        conv_result = INT64_MIN;
    }
    result->differential_age_ms = (int64_t) conv_result;

    // Solution Age
    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 19, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    value = strtod(token_buffer, &end_ptr);
    if(strcmp(end_ptr, token_buffer) == 0){
        // Improper string to double conversion
        return 7;
    }
    conv_result = value * 1000;

    //TODO: Determine what to do when we exceed data type value
    if(conv_result > INT64_MAX){
        conv_result = INT64_MAX;
    }
    else if(conv_result < INT64_MIN){
        conv_result = INT64_MIN;
    }
    result->solution_age_ms = (int64_t) conv_result;

    char crc[9];
    strncpy(crc, asterisk + 1, 8);
    crc[sizeof(crc)-1] = '\0';
    result->crc = strtoul(crc,&end_ptr, 16);

    // TODO: Add a check for the CRC

    return 0;
}

/// @brief Assigns a GPS Clock Model status based on the provided string.
/// @param status_str The status string to parse.
/// @param status Pointer to GPS_clock_model_status_enum_t where the status will be stored.
/// @return Returns 0 on success, 1 if the status string is unrecognized.
uint8_t assign_gps_clock_model_status(const char *status_str, GPS_clock_model_status_enum_t *status) {
    if (strcmp(status_str, "VALID") == 0) {
        *status = GPS_CLOCK_VALID;
    } else if (strcmp(status_str, "CONVERGING") == 0) {
        *status = GPS_CLOCK_CONVERGING;
    } else if (strcmp(status_str, "ITERATING") == 0) {
        *status = GPS_CLOCK_ITERATING;
    } else if (strcmp(status_str, "INVALID") == 0) {
        *status = GPS_CLOCK_INVALID;
    } else {
        return 1;  // Unrecognized status string
    }
    return 0;  // Success
}

/// @brief Assigns a GPS UTC status based on the provided string.
/// @param status_str The status string to parse.
/// @param status Pointer to GPS_utc_status_enum_t where the status will be stored.
/// @return Returns 0 on success, 1 if the status string is unrecognized.
uint8_t assign_gps_utc_status(const char *status_str, GPS_utc_status_enum_t *status) {
    if (strcmp(status_str, "INVALID") == 0) {
        *status = GPS_UTC_INVALID;
    } else if (strcmp(status_str, "VALID") == 0) {
        *status = GPS_UTC_VALID;
    } else if (strcmp(status_str, "WARNING") == 0) {
        *status = GPS_UTC_WARNING;
    } else {
        return 1;  // Unrecognized status string
    }
    return 0;  // Success
}

/// @brief Parse Received Data
/// @param data_received - Number of bytes in the data block
/// @return 0 if successful, >0 if an error occurred
uint8_t parse_timea_data(const char* data_received, gps_timea_response *result) {

    // Check if the buffer is empty
    if (data_received[0] == '\0') {
        // Empty or NULL string, return an error
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 1: Empty buffer"
        );
        return 1;
    }

    gps_response_header timea_header;
    const uint8_t header_parse_result = parse_gps_header(data_received,&timea_header);

    if(header_parse_result != 0){
        // Error in parsing header section
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 2: Header Parsing Error"
        );
        return 2;
    }

    if(strcmp(timea_header.log_name, "TIMEA") != 0){
        // Incorrect log function
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 3: Invalid log function ie not TIMEA"
        );
        return 3;
    }

    // TODO: What if there is multiple commands in the string?
    const char *header_delimiter_char = strchr(data_received,';');
    const char* timea_data_start = header_delimiter_char + 1;
    const char* asterisk = strchr(timea_data_start, '*');

    const int timea_data_length = asterisk - timea_data_start + 1;

    if(strcmp(timea_data_start, "\0") == 0){
        // No data after the gps header within the response
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 4: Missing Data after the header"
        );
        return 4;
    }

    if(!asterisk){
        // No terminator at the end of the bestxyza data, ie no CRC present
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 5: Missing Astericks in response"
        );
        return 5;
    }

    // TODO: Change this to be more conservative of storage space
    char timea_data_buffer[512];
    if ((size_t)timea_data_length >= sizeof(timea_data_buffer)) {
        //Header is too large for the buffer
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 6: Buffer Oveflow"
        );
        return 6;  
    }

    strncpy(timea_data_buffer, timea_data_start, timea_data_length);
    timea_data_buffer[timea_data_length] = '\0';

    // Parse the data in the bestxyza data buffer
    uint8_t parse_result;
    char token_buffer[128];
    char *end_ptr;
    
    // Clock Model Status
    parse_result = TCMD_extract_string_arg(timea_data_buffer, 0, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    uint8_t status_result = assign_gps_clock_model_status(token_buffer, &result->clock_status);
    if(status_result != 0){
        // Invalid string passed
        return status_result;
    }

    // UTC Offset
    parse_result = TCMD_extract_string_arg(timea_data_buffer, 3, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }

    // TODO: Use TCMD_ascii_to_int64 function instead
    double value = strtod(token_buffer, &end_ptr);

    if(strcmp(end_ptr,token_buffer)==0){
        return 7;
    }

    // // Testing with TCMD_ascii_to_int64 function
    // int64_t test_result;
    // parse_result = TCMD_ascii_to_int64(token_buffer,sizeof(token_buffer),&test_result);
    // if(parse_result!= 0){
    //     // Couldnt convert to a valid int64 value
    //     return 9;
    // }

    // if(test_result > INT64_MAX){
    //     test_result = INT64_MAX;
    // } else if (test_result < INT64_MIN){
    //     test_result = INT64_MIN;
    // }

    // result->utc_offset = test_result;

    
    // TODO: Determine what to do when we exceed data limit ie fall out of bounds of valid int64_t values
    // May not be necessary as max amount of seconds in a year is 3.154e+7 which is way smaller than the max value of int64
    if(value > INT64_MAX){
        value = INT64_MAX;
    } else if (value < INT64_MIN){
        value = INT64_MIN;
    }
    result->utc_offset = (int64_t) value;


    // UTC Status
    parse_result = TCMD_extract_string_arg(timea_data_buffer, 10, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }

    char *asterisk_pos = strchr(token_buffer, '*');
    if (asterisk_pos) {
        *asterisk_pos = '\0';  
    }
    status_result = assign_gps_utc_status(token_buffer, &result->utc_status);
    if(status_result != 0){
        // Invalid string passed
        return status_result;
    }

    // TODO: Look into using TCMD_extract_hex_array
    char crc[9];
    strncpy(crc, asterisk + 1, 8);
    crc[sizeof(crc)-1] = '\0';
    result->crc = strtoul(crc,&end_ptr, 16);

    // TODO: Add a check for the CRC

        // char message_buffer[256];
    //     snprintf(
    //         message_buffer, sizeof(message_buffer),
    //         "{\"Clock Status\":\"%s\",\"UTC Offset\":\"%s\",\"UTC Status\":%lu,\"CRC\":%u}\n",
    //         result.clock_status,
    //         result.utc_offset,
    //         result.utc_status,
    //         result.crc
    //     );
          
    //     DEBUG_uart_print_str(message_buffer);

    return 0;
}

