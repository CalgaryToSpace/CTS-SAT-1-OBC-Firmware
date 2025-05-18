#ifndef INC_ADCS_INTERNAL_DRIVERS_H_
#define INC_ADCS_INTERNAL_DRIVERS_H_

#include "adcs_drivers/adcs_types.h"
#include "main.h" // this is necessary because &hi2c1 is defined in that file

#include <time.h>

#define ADCS_i2c_HANDLE &hi2c1

static const uint8_t ADCS_INCLUDE_CHECKSUM = 1;
static const uint8_t ADCS_NO_CHECKSUM = 0;
static const uint8_t ADCS_CHECKSUM_TIMEOUT_TRIES = 100;
static const uint16_t ADCS_PROCESSED_TIMEOUT_TRIES = 1000;
static const uint16_t ADCS_FILE_POINTER_TIMEOUT_MS  =  60000; // worst-case navigation time for the file pointer is 51 seconds (255 files)
static const uint32_t ADCS_FILE_DOWNLOAD_TIMEOUT_MS = 290000; // we expect about 4.675 minutes to download 1024x1024px files, which are the largest. 

/* Function Definitions */

// byte transforms
uint8_t ADCS_convert_uint16_to_reversed_uint8_array_members(uint8_t *array, uint16_t value, uint16_t index);
uint8_t ADCS_convert_uint32_to_reversed_uint8_array_members(uint8_t *array, uint32_t value, uint16_t index);
uint8_t ADCS_convert_double_to_string(double input, uint8_t precision, char* output_string, uint16_t str_len);
uint8_t ADCS_combine_sd_log_bitmasks(const uint8_t **array_in, const uint8_t array_in_len, uint8_t *array_out);

// TC/TLM functions (basic communication)
uint8_t ADCS_i2c_send_command_and_check(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum);
uint8_t ADCS_i2c_request_telemetry_and_check(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum);
uint8_t ADCS_send_i2c_telecommand(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum);
uint8_t ADCS_send_i2c_telemetry_request(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum);

// CRC functions
uint8_t ADCS_initialize_crc8_checksum();
uint8_t ADCS_calculate_crc8_checksum(uint8_t* buffer, uint16_t len);

#endif /* INC_ADCS_INTERNAL_DRIVERS_H_ */