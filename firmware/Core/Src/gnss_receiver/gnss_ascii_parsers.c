#include "gnss_receiver/gnss_ascii_parsers.h"
#include "gnss_receiver/gnss_types.h"
#include "log/log.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "transforms/arrays.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/// @brief Parse the received GNSS header into a struct
/// @param data_received - The string obtained from the buffer that is to be parsed into the GNSS_header_response_t struct
/// @param result - GNSS_header_response_t struct that is returned
/// @return 0 if successful, > 0 if an error occurred
uint8_t GNSS_header_response_parser(const char *data_received, GNSS_header_response_t *result)
{

    // TODO: What if there are multiple responses in the string?

    // Check if the buffer is empty
    if (data_received == NULL || data_received[0] == '\0')
    {
        // Empty or NULL string, return an error
        return 1;
    }

    // Find the start and end of the header, which is # and ; resepectively
    const char *sync_char = strchr(data_received, '#');
    const char *delimiter_char = strchr(data_received, ';');

    if (!sync_char || !delimiter_char)
    {
        // Invalid data: No header in gnss response
        return 2;
    }

    // Calculate the length of the header string
    const int header_length = delimiter_char - sync_char + 1;
    if (header_length < 0)
    {
        // Sync character occurs after delimiter character
        return 3;
    }

    char header_buffer[256];
    if ((size_t)header_length >= sizeof(header_buffer))
    {
        // Header is too large for the buffer
        return 4;
    }

    // Copy header string into a buffer
    strncpy(header_buffer, sync_char, header_length);
    header_buffer[header_length] = '\0'; // Null-terminate the substring

    // Parse the data in the header buffer
    uint8_t parse_result;
    char token_buffer[128];

    // Log Name
    parse_result = TCMD_extract_string_arg(header_buffer, 0, token_buffer, sizeof(token_buffer));
    if (parse_result != 0)
    {
        return parse_result;
    }
    strcpy(result->log_name, token_buffer + 1);

    // Time Status
    parse_result = TCMD_extract_string_arg(header_buffer, 4, token_buffer, sizeof(token_buffer));
    if (parse_result != 0)
    {
        return parse_result;
    }
    const uint8_t status_result = GNSS_reference_time_status_str_to_enum(token_buffer, &result->time_status);
    if (status_result != 0)
    {
        // Time Status not recognized
        return status_result;
    }

    char message_buffer[256];
    snprintf(
        message_buffer, sizeof(message_buffer),
        "{\"log_name\":\"%s\",\"time_status\":\"%s\"}\n",
        result->log_name,
        GNSS_reference_time_status_enum_to_str(result->time_status));

    LOG_message(
        LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Header Response: %s",
        message_buffer);

    return 0;
}

/// @brief Parse Received Data
/// @param data_received - Number of bytes in the data block
/// @return 0 if successful, >0 if an error occurred
uint8_t GNSS_bestxyza_data_parser(const char *data_received, GNSS_bestxyza_response_t *result)
{

    // Check if the buffer is empty
    if (data_received[0] == '\0')
    {
        // Empty or NULL string, return an error
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 1: Empty buffer");
        return 1;
    }

    GNSS_header_response_t bestxyza_header;
    const uint8_t header_parse_result = GNSS_header_response_parser(data_received, &bestxyza_header);

    if (header_parse_result != 0)
    {
        // Error in parsing header section
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 2: Error parising the gnss header");
        return 2;
    }

    if (strcmp(bestxyza_header.log_name, "BESTXYZA") != 0)
    {
        // Incorrect log function
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 3: Incorrect log function ie not BESTXYZA");
        return 3;
    }

    // TODO: What if there is multiple commands in the string?
    const char *header_delimiter_char = strchr(data_received, ';');
    const char *bestxyza_data_start = header_delimiter_char + 1;
    const char *asterisk = strchr(bestxyza_data_start, '*');

    if (strcmp(bestxyza_data_start, "\0") == 0)
    {
        // No data after the gnss header within the response
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 4: Missing Data after the header");
        return 4;
    }

    if (!asterisk)
    {
        // No terminator at the end of the bestxyza data, ie no CRC present
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 5: Missing Astericks in response");
        return 5;
    }

    const int bestxyza_data_length = asterisk - bestxyza_data_start + 1;
    char bestxyza_data_buffer[512];
    if ((size_t)bestxyza_data_length >= sizeof(bestxyza_data_buffer))
    {
        // Buffer Overflow Error
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 6: Buffer overflow");
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
    if (parse_result != 0)
    {
        return parse_result;
    }
    uint8_t status_result = GNSS_solution_status_str_to_enum(token_buffer, &result->position_solution_status);
    if (status_result != 0)
    {
        // Invalid string passed
        return status_result;
    }

    // Position Type
    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 1, token_buffer, sizeof(token_buffer));
    if (parse_result != 0)
    {
        return parse_result;
    }
    status_result = GNSS_position_type_str_to_enum(token_buffer, &result->position_type);
    if (status_result != 0)
    {
        // Invalid string passed
        return status_result;
    }

    // Position Coordinates
    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 2, token_buffer, sizeof(token_buffer));
    if (parse_result != 0)
    {
        return parse_result;
    }

    double value = strtod(token_buffer, &end_ptr);
    if (strcmp(end_ptr, token_buffer) == 0)
    {
        // Improper string to double conversion
        return 7;
    }
    double conv_result = value * 1000;

    // TODO: Determine what to do when we exceed data type value
    if (conv_result > INT64_MAX)
    {
        conv_result = INT64_MAX;
    }
    else if (conv_result < INT64_MIN)
    {
        conv_result = INT64_MIN;
    }
    result->position_x_mm = (int64_t)conv_result;

    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 3, token_buffer, sizeof(token_buffer));
    if (parse_result != 0)
    {
        return parse_result;
    }
    value = strtod(token_buffer, &end_ptr);
    if (strcmp(end_ptr, token_buffer) == 0)
    {
        // Improper string to double conversion
        return 7;
    }
    conv_result = value * 1000;

    // TODO: Determine what to do when we exceed data type value
    if (conv_result > INT64_MAX)
    {
        conv_result = INT64_MAX;
    }
    else if (conv_result < INT64_MIN)
    {
        conv_result = INT64_MIN;
    }
    result->position_y_mm = (int64_t)conv_result;

    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 4, token_buffer, sizeof(token_buffer));
    if (parse_result != 0)
    {
        return parse_result;
    }
    value = strtod(token_buffer, &end_ptr);
    if (strcmp(end_ptr, token_buffer) == 0)
    {
        // Improper string to double conversion
        return 7;
    }
    conv_result = value * 1000;

    // TODO: Determine what to do when we exceed data type value
    if (conv_result > INT64_MAX)
    {
        conv_result = INT64_MAX;
    }
    else if (conv_result < INT64_MIN)
    {
        conv_result = INT64_MIN;
    }
    result->position_z_mm = (int64_t)conv_result;

    // Position Coordinates Standard Deviation
    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 5, token_buffer, sizeof(token_buffer));
    if (parse_result != 0)
    {
        return parse_result;
    }
    value = strtod(token_buffer, &end_ptr);
    if (strcmp(end_ptr, token_buffer) == 0)
    {
        // Improper string to double conversion
        return 7;
    }
    conv_result = value * 1000;

    // TODO: Determine what to do when we exceed data type value
    if (conv_result > INT32_MAX)
    {
        conv_result = INT32_MAX;
    }
    else if (conv_result < INT32_MIN)
    {
        conv_result = INT32_MIN;
    }
    result->position_x_std_mm = (int32_t)conv_result;

    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 6, token_buffer, sizeof(token_buffer));
    if (parse_result != 0)
    {
        return parse_result;
    }
    value = strtod(token_buffer, &end_ptr);
    if (strcmp(end_ptr, token_buffer) == 0)
    {
        // Improper string to double conversion
        return 7;
    }
    conv_result = value * 1000;

    // TODO: Determine what to do when we exceed data type value
    if (conv_result > INT32_MAX)
    {
        conv_result = INT32_MAX;
    }
    else if (conv_result < INT32_MIN)
    {
        conv_result = INT32_MIN;
    }
    result->position_y_std_mm = (int32_t)conv_result;

    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 7, token_buffer, sizeof(token_buffer));
    if (parse_result != 0)
    {
        return parse_result;
    }
    value = strtod(token_buffer, &end_ptr);
    if (strcmp(end_ptr, token_buffer) == 0)
    {
        // Improper string to double conversion
        return 7;
    }
    conv_result = value * 1000;

    // TODO: Determine what to do when we exceed data type value
    if (conv_result > INT32_MAX)
    {
        conv_result = INT32_MAX;
    }
    else if (conv_result < INT32_MIN)
    {
        conv_result = INT32_MIN;
    }
    result->position_z_std_mm = (int32_t)conv_result;

    // Differential Age
    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 18, token_buffer, sizeof(token_buffer));
    if (parse_result != 0)
    {
        return parse_result;
    }
    value = strtod(token_buffer, &end_ptr);
    if (strcmp(end_ptr, token_buffer) == 0)
    {
        // Improper string to double conversion
        return 7;
    }
    conv_result = value * 1000;

    // TODO: Determine what to do when we exceed data type value
    if (conv_result > INT64_MAX)
    {
        conv_result = INT64_MAX;
    }
    else if (conv_result < INT64_MIN)
    {
        conv_result = INT64_MIN;
    }
    result->differential_age_ms = (int64_t)conv_result;

    // Solution Age
    parse_result = TCMD_extract_string_arg(bestxyza_data_buffer, 19, token_buffer, sizeof(token_buffer));
    if (parse_result != 0)
    {
        return parse_result;
    }
    value = strtod(token_buffer, &end_ptr);
    if (strcmp(end_ptr, token_buffer) == 0)
    {
        // Improper string to double conversion
        return 7;
    }
    conv_result = value * 1000;

    // TODO: Determine what to do when we exceed data type value
    if (conv_result > INT64_MAX)
    {
        conv_result = INT64_MAX;
    }
    else if (conv_result < INT64_MIN)
    {
        conv_result = INT64_MIN;
    }
    result->solution_age_ms = (int64_t)conv_result;

    char crc[9];
    strncpy(crc, asterisk + 1, 8);
    crc[sizeof(crc) - 1] = '\0';
    result->crc = strtoul(crc, &end_ptr, 16);

    // TODO: Add a check for the CRC

    char sol_age[32];
    GEN_uint64_to_str(result->solution_age_ms, sol_age);
    char diff_age[32];
    GEN_uint64_to_str(result->differential_age_ms, diff_age);
    char pos_x[32];
    GEN_int64_to_str(result->position_x_mm, pos_x);
    char pos_y[32];
    GEN_int64_to_str(result->position_y_mm, pos_y);
    char pos_z[32];
    GEN_int64_to_str(result->position_z_mm, pos_z);

    char message_buffer[2048];
    snprintf(
        message_buffer, sizeof(message_buffer),
        "{\"Position Solution Status\":\"%s\",\"Position Type\":\"%s\",\"Position x in mm\":\"%s\",\"Position y in mm\":\"%s\",\"Position z in mm\":\"%s\","
        "\"Position x std in mm\":\"%lu\",\"Position y std in mm\":\"%lu\",\"Position z std in mm\":\"%lu\",\"Solution Age in ms\":\"%s\",\"Differential age in ms\":\"%s\",\"CRC\":\"%ld\"}\n",
        GNSS_solution_status_enum_to_str(result->position_solution_status),
        GNSS_position_type_enum_to_string(result->position_type),
        pos_x,
        pos_y,
        pos_z,
        result->position_x_std_mm,
        result->position_y_std_mm,
        result->position_z_std_mm,
        sol_age,
        diff_age,
        result->crc);

    LOG_message(
        LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Bestxyza Response: %s",
        message_buffer);

    return 0;
}

/// @brief Parse Received Data
/// @param data_received - Number of bytes in the data block
/// @return 0 if successful, >0 if an error occurred
uint8_t GNSS_timea_data_parser(const char *data_received, GNSS_timea_response_t *result)
{

    // Check if the buffer is empty
    if (data_received[0] == '\0')
    {
        // Empty or NULL string, return an error
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 1: Empty buffer");
        return 1;
    }

    GNSS_header_response_t timea_header;
    const uint8_t header_parse_result = GNSS_header_response_parser(data_received, &timea_header);

    if (header_parse_result != 0)
    {
        // Error in parsing header section
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 2: Header Parsing Error");
        return 2;
    }

    if (strcmp(timea_header.log_name, "TIMEA") != 0)
    {
        // Incorrect log function
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 3: Invalid log function ie not TIMEA");
        return 3;
    }

    // TODO: What if there is multiple commands in the string?
    const char *header_delimiter_char = strchr(data_received, ';');
    const char *timea_data_start = header_delimiter_char + 1;
    const char *asterisk = strchr(timea_data_start, '*');

    const int timea_data_length = asterisk - timea_data_start + 1;

    if (strcmp(timea_data_start, "\0") == 0)
    {
        // No data after the gnss header within the response
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 4: Missing Data after the header");
        return 4;
    }

    if (!asterisk)
    {
        // No terminator at the end of the bestxyza data, ie no CRC present
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 5: Missing Astericks in response");
        return 5;
    }

    // TODO: Change this to be more conservative of storage space
    char timea_data_buffer[512];
    if ((size_t)timea_data_length >= sizeof(timea_data_buffer))
    {
        // Header is too large for the buffer
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Error 6: Buffer Oveflow");
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
    if (parse_result != 0)
    {
        return parse_result;
    }
    uint8_t status_result = GNSS_clock_model_status_str_to_enum(token_buffer, &result->clock_status);
    if (status_result != 0)
    {
        // Invalid string passed
        return status_result;
    }

    // UTC Offset
    parse_result = TCMD_extract_string_arg(timea_data_buffer, 3, token_buffer, sizeof(token_buffer));
    if (parse_result != 0)
    {
        return parse_result;
    }

    double value = strtod(token_buffer, &end_ptr);
    if (strcmp(end_ptr, token_buffer) == 0)
    {
        return 7;
    }

    // TODO: Determine what to do when we exceed data limit ie fall out of bounds of valid int64_t values
    // May have to change to int32_t for optimization.
    if (value > INT64_MAX)
    {
        value = INT64_MAX;
    }
    else if (value < INT64_MIN)
    {
        value = INT64_MIN;
    }
    result->utc_offset = (int64_t)value;

    // UTC Status
    parse_result = TCMD_extract_string_arg(timea_data_buffer, 10, token_buffer, sizeof(token_buffer));
    if (parse_result != 0)
    {
        return parse_result;
    }

    char *asterisk_pos = strchr(token_buffer, '*');
    if (asterisk_pos)
    {
        *asterisk_pos = '\0';
    }
    status_result = GNSS_utc_status_str_to_enum(token_buffer, &result->utc_status);
    if (status_result != 0)
    {
        // Invalid string passed
        return status_result;
    }

    // TODO: Look into using TCMD_extract_hex_array
    // TODO: Add a check for the CRC
    char crc[9];
    strncpy(crc, asterisk + 1, 8);
    crc[sizeof(crc) - 1] = '\0';
    result->crc = strtoul(crc, &end_ptr, 16);

    char utc_offset[32];
    GEN_int64_to_str(result->utc_offset, utc_offset);
    char message_buffer[1024];
    snprintf(
        message_buffer, sizeof(message_buffer),
        "{\"Clock Status\":\"%s\",\"UTC Status\":%s,\"UTC Offset\":\"%s\",\"CRC\":%lx}\n",
        GNSS_clock_model_status_enum_to_string(result->clock_status),
        utc_offset,
        GNSS_utc_status_enum_to_string(result->utc_status),
        result->crc);

    LOG_message(
        LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Timea Response: %s",
        message_buffer);

    return 0;
}