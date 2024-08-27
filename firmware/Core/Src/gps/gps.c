#include "gps/gps.h"
#include "gps/gps_types.h"
#include "log/log.h"
#include "telecommands/telecommand_args_helpers.h"

#include <stdio.h>
#include <stdlib.h> 
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


const char* get_gps_time_status_description(GPS_reference_time_status_t status) {
    switch (status) {
        case GPS_UNKNOWN: return "Unknown";
        case GPS_APPROXIMATE: return "Approximate";
        case GPS_COARSEADJUSTING: return "Coarse Adjusting";
        case GPS_COARSE: return "Coarse";
        case GPS_COARSESTEERING: return "Coarse Steering";
        case GPS_FREEWHEELING: return "Freewheeling";
        case GPS_FINEADJUSTING: return "Fine Adjusting";
        case GPS_FINE: return "Fine";
        case GPS_FINEBACKUPSTEERING: return "Fine Backup Steering";
        case GPS_FINESTEERING: return "Fine Steering";
        case GPS_SATTIME: return "Satellite Time";
        default: return "Invalid Status";
    }
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
    uint8_t parse_result;
    char token_buffer[256];
    char *end_ptr;

    // Log Name
    parse_result = TCMD_extract_string_arg(header_buffer, 0, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    strcpy(result->log_name, token_buffer + 1);

    LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Log Name = %s.",
            result->log_name
        );

    // Port
    parse_result = TCMD_extract_string_arg(header_buffer, 1, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) { 
        return parse_result;  
    }

    strncpy(result->port, token_buffer, sizeof(result->port) - 1);
    result->port[sizeof(result->port) - 1] = '\0';

    LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Port = %s.",
            result->port
        );

    // Sequence Number
    parse_result = TCMD_extract_string_arg(header_buffer, 2, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    result->sequence_no = strtoul(token_buffer, &end_ptr, 10);
    if (*end_ptr != '\0'){
        return 1;  // Error in conversion
    }

    LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Seq No = %lu.",
            result->sequence_no
        );

    // Idle Time
    parse_result = TCMD_extract_string_arg(header_buffer, 3, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    result->idle_time = strtoul(token_buffer, &end_ptr, 10);

    LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Idle Time = %lu.",
            result->idle_time
        );

    // Time Status
    parse_result = TCMD_extract_string_arg(header_buffer, 4, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    const uint8_t status_result = assign_gps_time_status(token_buffer, &result->time_status);
    if (status_result != 0) {
        // Time Status not recognized
        return 1;
    }

    LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Time status = %s.",
            get_gps_time_status_description(result->time_status)
        );
    
    // Week
    parse_result = TCMD_extract_string_arg(header_buffer, 5, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    result->week = strtoul(token_buffer, &end_ptr, 10);

    LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Week = %lu.",
            result->week
        );

    // seconds
    parse_result = TCMD_extract_string_arg(header_buffer, 6, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    result->seconds = strtoul(token_buffer, &end_ptr, 10);

    LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "seconds = %lu.",
            result->seconds
        );

    // Rx Status
    parse_result = TCMD_extract_string_arg(header_buffer, 7, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    result->rx_status = strtoul(token_buffer, &end_ptr, 16);  // hexadecimal

    LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Rx Status = %x.",
            result->rx_status
        );

    // Rx Sw Version
    parse_result = TCMD_extract_string_arg(header_buffer, 9, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    result->rx_sw_version = strtoul(token_buffer, &end_ptr, 10);

    LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Rx sw Version = %lu.",
            result->rx_sw_version
        );

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
uint8_t assign_gps_position_velocity_type(const char *type_str, GPS_position_velocity_type_enum_t *type) {
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

    gps_response_header bestxyza_header;
    const uint8_t header_parse_result = parse_gps_header(data_received,&bestxyza_header);

    if(header_parse_result != 0){
        // Error in parsing header section
        return 1;
    }

    if(strcmp(bestxyza_header.log_name, "BESTXYZA") != 0){
        // Incorrect log function
        return 2;
    }

    // TODO: What if there is multiple commands in the string?
    const char *header_delimiter_char = strchr(data_received,';');
    const char* bestxyza_data_start = header_delimiter_char + 1;
    const char* asterisk = strchr(bestxyza_data_start, '*');

    if(!asterisk){
        // Not terminator to the end of the bestxyza data, ie no CRC present
        return 3;
    }

    const int bestxyza_data_length = bestxyza_data_start - asterisk + 1;

    if (bestxyza_data_length < 0) {
        //CRC asterick comes before the start of the data: Incomplete bestxyza data
        return 4;
    }

    char bestxyza_data_buffer[1024];
    if ((size_t)bestxyza_data_length >= sizeof(bestxyza_data_buffer)) {
        //Header is too large for the buffer
        return 5;  
    }

    strncpy(bestxyza_data_buffer, bestxyza_data_start, bestxyza_data_length);
    bestxyza_data_buffer[bestxyza_data_length] = '\0';

    // Parse the data in the bestxyza data buffer
    uint8_t parse_result;
    char token_buffer[256];
    
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


    // Velocity Solution Status
    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 8, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    status_result = assign_gps_solution_status(token_buffer, &result->velocity_solution_status);
    if(status_result != 0){
        // Invalid string passed
        return status_result;
    }

    // Velocity Type
    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 9, token_buffer, sizeof(token_buffer));
    if (parse_result != 0) {  
        return parse_result;  
    }
    status_result = assign_gps_position_velocity_type(token_buffer, &result->velocity_type);
    if(status_result != 0){
        // Invalid string passed
        return status_result;
    }




    LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Message Parsed Successfully."
        );

    return 0;
}

