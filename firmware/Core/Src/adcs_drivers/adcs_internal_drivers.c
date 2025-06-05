#include "adcs_drivers/adcs_types.h"
#include "adcs_drivers/adcs_command_ids.h"
#include "adcs_drivers/adcs_struct_packers.h"
#include "adcs_drivers/adcs_types_to_json.h"
#include "adcs_drivers/adcs_commands.h"
#include "adcs_drivers/adcs_internal_drivers.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "stm32l4xx_hal.h"

/* Basic Telecommand Functions */ 

/// @brief Sends a telecommand over I2C to the ADCS, checks that it's been acknowledged, and returns the ACK error flag.
/// @param[in] id Valid ADCS telecommand ID (see Firmware Reference Manual)
/// @param[in] data Data array to send the raw data bytes; length must be at least data_length (should contain the correct number of bytes for the given telecommand ID)
/// @param[in] data_length Length of the data array.
/// @param[in] include_checksum Tells the ADCS whether to use a CRC8 checksum; should be either ADCS_INCLUDE_CHECKSUM or ADCS_NO_CHECKSUM 
/// @return 0 if successful, 1 if invalid ID, 2 if incorrect parameter length, 3 if incorrect parameter value, and 4 if failed CRC
uint8_t ADCS_i2c_send_command_and_check(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum) {
    ADCS_cmd_ack_struct_t ack;
    uint8_t cmd_status;
    uint8_t num_checksum_tries = 0; // number of tries to resend command if checksum fails
    do {
        // Send telecommand
        cmd_status = ADCS_send_i2c_telecommand(id, data, data_length, include_checksum);

        // poll Acknowledge Telemetry Format until the Processed flag equals 1
        if (cmd_status == 0) {
            uint8_t ack_status;
            uint8_t num_ack_tries = 0;
            while (!ack.processed) {
                ack_status = ADCS_cmd_ack(&ack); // confirm telecommand validity by checking the TC Error flag of the last read TC Acknowledge Telemetry Format.
                num_ack_tries++;
                if (num_ack_tries > ADCS_PROCESSED_TIMEOUT_TRIES) {
                    return 5; // command failed to process in time
                    // note: sending another telecommand when the first has not been processed
                    // will result in an error in the next telecommand sent
                    // as the ADCS telecommand buffer has been overrun/corrupted
                }
                if ((ack_status != 0) && (ack_status != 4)) {
                    return ack_status; // there was an error in the command not related to checksum
                }
            }
        } else {
            return cmd_status; // if the HAL had an error, tell us what that is
        }
        num_checksum_tries++;
    } while (ack.error_flag == ADCS_ERROR_FLAG_CRC && num_checksum_tries < ADCS_CHECKSUM_TIMEOUT_TRIES);  // if the checksum doesn't check out, keep resending the request
    
    HAL_Delay(4);
    const uint8_t ack_status = ADCS_cmd_ack(&ack);
    if ((ack_status != 0) && (ack_status != 4)) {
        return ack_status; // there was an error in the command not related to checksum
    }

    return ack.error_flag; // if the HAL was successful and the ADCS command had an error, tell us what it is
}

/// @brief Sends a telemetry request over I2C to the ADCS, and resends repeatedly if the checksums don't match.
/// @param[in] id Valid ADCS telemetry request ID (see Firmware Reference Manual)
/// @param[in] include_checksum Tells the ADCS whether to use a CRC8 checksum; should be either ADCS_INCLUDE_CHECKSUM or ADCS_NO_CHECKSUM 
/// @param[in] data_length Length of the data array.
/// @param[out] data Data array to write the raw telemetry bytes to; length must be at least data_length (should contain the correct number of bytes for the given telemetry request ID)
/// @return 0 if successful, other numbers if the HAL failed to transmit or receive data. 
uint8_t ADCS_i2c_request_telemetry_and_check(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum) {
    uint8_t checksum_check = ADCS_send_i2c_telemetry_request(id, data, data_length, include_checksum);
    uint8_t num_checksum_tries = 0;
    while (checksum_check == 4 && num_checksum_tries < ADCS_CHECKSUM_TIMEOUT_TRIES) {
        // if the checksum doesn't check out, keep resending the request up to timeout
        checksum_check = ADCS_send_i2c_telemetry_request(id, data, data_length, include_checksum);
        num_checksum_tries++;
    }
    return checksum_check;
}


/// @brief Sends a telecommand over I2C to the ADCS.
/// @param[in] id Valid ADCS telecommand ID (see Firmware Reference Manual)
/// @param[in] data Data array to send the raw data bytes; length must be at least data_length (should contain the correct number of bytes for the given telecommand ID)
/// @param[in] data_length Length of the data array.
/// @param[in] include_checksum Tells the ADCS whether to use a CRC8 checksum; should be either ADCS_INCLUDE_CHECKSUM or ADCS_NO_CHECKSUM 
/// @return 0 if successful, 4 if the checksums don't match, other numbers if the HAL failed to transmit or receive data.
uint8_t ADCS_send_i2c_telecommand(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum) {
    uint8_t ADCS_CMD_status; 
    
    // allocate only required memory
    uint8_t buf[data_length + include_checksum]; // add additional bit for checksum if needed

    // fill buffer with data 
    for (uint32_t i = 0; i < data_length; i++) {
        buf[i] = data[i];
    }

    // include checksum following data if enabled
    if (include_checksum) {buf[data_length] = ADCS_calculate_crc8_checksum(data, data_length);}

    ADCS_CMD_status = HAL_I2C_Mem_Write(ADCS_i2c_HANDLE, ADCS_i2c_ADDRESS << 1, id, 1, buf, sizeof(buf), ADCS_HAL_TIMEOUT);

    /* When sending a command to the CubeACP, it is possible to include an 8-bit CRC checksum.
    For instance, when sending a command that has a length of 8 bytes, it is possible to include a
    9th byte that is computed from the previous 8 bytes. The extra byte will be interpreted as a
    checksum and used to validate the message. If the checksum fails, the command will be
    ignored. For I2C communication, the Tc Error Status in the Telecommand Acknowledge telemetry frame
    (Table 39: Telecommand Acknowledge Telemetry Format) will have a value of 4. */

    return ADCS_CMD_status;
}

/// @brief Sends a telemetry request over I2C to the ADCS.
/// @param[in] id Valid ADCS telemetry request ID (see Firmware Reference Manual)
/// @param[in] include_checksum Tells the ADCS whether to use a CRC8 checksum; should be either ADCS_INCLUDE_CHECKSUM or ADCS_NO_CHECKSUM 
/// @param[in] data_length Length of the data array.
/// @param[out] data Data array to write the raw telemetry bytes to; length must be at least data_length (should contain the correct number of bytes for the given telemetry request ID)
/// @return 0 if successful, 4 if the checksums don't match, other numbers if the HAL failed to transmit or receive data. 
uint8_t ADCS_send_i2c_telemetry_request(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum) {
    // Telemetry Request Format:
    // Note: requires a repeated start condition; data_length is number of bits to read.
    // [start], ADCS_i2c_WRITE_ADDRESS, id, [start] ADCS_i2c_READ_ADDRESS, [read all the data], [stop]
    // The defines in adcs_types.h already include the 7th bit of the ID to distinguish TLM and TC

    /* When requesting telemetry through I2C, it is possible to read one extra byte past the allowed
    length of the telemetry frame. In this case, the extra byte will also be an 8-bit checksum
    computed by the CubeACP and can be used by the interfacing OBC to validate the message.*/
    uint8_t adcs_tlm_status;

    // Allocate only required memory
    uint8_t temp_data[data_length + include_checksum];
        // temp data used for checksum checking

    adcs_tlm_status = HAL_I2C_Mem_Read(ADCS_i2c_HANDLE, ADCS_i2c_ADDRESS << 1, id, 1, temp_data, sizeof(temp_data), ADCS_HAL_TIMEOUT);

    for (uint32_t i = 0; i < data_length; i++) {
            // populate external data, except for checksum byte
            data[i] = temp_data[i];
    }

    if (include_checksum) {
        uint8_t checksum = temp_data[data_length];
        uint8_t checksum_test = ADCS_calculate_crc8_checksum(data, data_length);
        if (checksum != checksum_test) {
            return 0x04;
        }
    }

    return adcs_tlm_status;

}

/// @brief Swap low and high bytes of a uint16 to turn into uint8 and put into specified index of an array
/// @param[in] value Value to split and swap the order of.
/// @param[in] index Index in array to write the result. (Array must contain at least two bytes, with index pointing to the first)
/// @param[out] array Data array to write the two bytes to at the specified index and index + 1.
/// @return 0 once complete.
uint8_t ADCS_convert_uint16_to_reversed_uint8_array_members(uint8_t *array, uint16_t value, uint16_t index) {
    array[index] = (uint8_t)(value & 0xFF); // Insert the low byte of the value into the array at the specified index
    array[index + 1] = (uint8_t)(value >> 8); // Insert the high byte of the value into the array at the next index
    return 0;
}

/// @brief Reverse the order of the four bytes of a uint32 to turn into uint8 and put into specified index of an array
/// @param[in] value Value to split and swap the order of.
/// @param[in] index Index in array to write the result. (Array must contain at least four bytes, with index pointing to the first)
/// @param[out] array Data array to write the four bytes to at the specified index and the three subsequent indices.
/// @return 0 once complete.
uint8_t ADCS_convert_uint32_to_reversed_uint8_array_members(uint8_t *array, uint32_t value, uint16_t index) {
    array[index] = (uint8_t)(value & 0xFF); // Insert the low byte of the value into the array at the specified index
    array[index + 1] = (uint8_t)((value >> 8) & 0xFF); // Insert the second byte of the value into the array at the next index
    array[index + 2] = (uint8_t)((value >> 16) & 0xFF); // Insert the third byte of the value into the array at the next next index
    array[index + 3] = (uint8_t)(value >> 24); // Insert the high byte of the value into the array at the next next next index
    return 0;
}

/// @brief Convert a double into a string with a given decimal precision.
/// @note This is used because snprintf doesn't support printing doubles.
/// @param[in] input Value to convert.
/// @param[in] precision Number of decimal places to use.
/// @param[out] output_string Character array to write the string number to.
/// @param[in] str_len Length of the output string.
/// @return 0 once complete, 1 if the string is too short, 2 if there was an error printing to the string.
uint8_t ADCS_convert_double_to_string(double input, uint8_t precision, char* output_string, uint16_t str_len) {
    if (str_len < 3*precision) {
        return 1;
    }
    
    int32_t data_int_portion = (int32_t)(input); 
    uint32_t data_decimal_portion = (uint32_t)(fabs((input - data_int_portion)) * pow(10, precision)); 
    
    int16_t snprintf_ret;
    if (data_int_portion == 0 && input < 0) {
        snprintf_ret = snprintf(output_string, str_len, "-%ld.%0*lu", data_int_portion, precision, data_decimal_portion);
    } else {
        snprintf_ret = snprintf(output_string, str_len, "%ld.%0*lu", data_int_portion, precision, data_decimal_portion);
    }
    
    if (snprintf_ret < 0) {
        return 2;
    }

    return 0;
}

/// @brief Take an arbitrary number (up to 63) of 10-byte uint8 arrays and return a single array which is the bitwise OR of all of them. 
/// @param[in] array_in Array of pointers to 10-byte uint8 data arrays
/// @param[in] array_in_len Size of the array_in array
/// @param[out] array_out 10-byte uint8 data array to send the result to
/// @return 0 once complete.
uint8_t ADCS_combine_sd_log_bitmasks(const uint8_t **array_in, const uint8_t array_in_len, uint8_t *array_out) {
    memset(array_out, 0, ADCS_SD_LOG_BITFIELD_LENGTH_BYTES); // reset array_out to all zeroes
    for (uint8_t i = 0; i < array_in_len; i++) {
        for (uint8_t j = 0; j < ADCS_SD_LOG_BITFIELD_LENGTH_BYTES; j++) {
            // iterate through array_out and bitwise OR each element with
            // the corresponding element in the array array_in[i]
            array_out[j] |= array_in[i][j];
        }
    }
    return 0;
}

static uint8_t CRC8Table[256];

/// @brief Initialize the lookup table for 8-bit CRC calculation. Code provided by ADCS Firmware Reference Manual (Section 5.5).
/// @return 0 once successful.
uint8_t ADCS_initialize_crc8_checksum() {
    int val;
    for (uint16_t i = 0; i < 256; i++)
    {
        val = i;
        for (uint8_t j = 0; j < 8; j++)
        {
            if (val & 1)
            val ^= ADCS_CRC_POLY;
            val >>= 1;
        }
        CRC8Table[i] = val;
    }
    return 0;
}

/// @brief Calculates an 8-bit CRC value. Code provided by ADCS Firmware Reference Manual (Section 5.5).
/// @param[in] buffer the buffer containing data for which to calculate the crc value
/// @param[in] len the number of bytes of valid data in the buffer
/// @return the CRC value calculated (which is 0xFF for an empty buffer)
uint8_t ADCS_calculate_crc8_checksum(uint8_t* buffer, uint16_t len)
{
    if (len == 0) return 0xff;

    uint16_t i;
    uint8_t crc = 0;

    for (i = 0; i < len; i++)
        crc = CRC8Table[crc ^ buffer[i]];

    return crc;
}