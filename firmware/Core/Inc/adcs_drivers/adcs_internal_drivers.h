#ifndef INC_ADCS_INTERNAL_DRIVERS_H_
#define INC_ADCS_INTERNAL_DRIVERS_H_

#include "adcs_types.h"
#include "main.h" // this is necessary because &hi2c1 is defined in that file

#define ADCS_I2C_HANDLE &hi2c1
static const uint8_t ADCS_INCLUDE_CHECKSUM = 1;
static const uint8_t ADCS_NO_CHECKSUM = 0;
static const uint8_t ADCS_CHECKSUM_TIMEOUT = 100;
static const uint16_t ADCS_PROCESSED_TIMEOUT = 1000;
#define WRITE_STRUCT_TO_MEMORY(struct_to_write) // memory module function: write struct to memory

/* Function Definitions */

// byte transforms
uint8_t ADCS_convert_uint16_to_reversed_uint8_array_members(uint8_t *array, uint16_t value, int index);
uint8_t ADCS_convert_uint32_to_reversed_uint8_array_members(uint8_t *array, uint32_t value, int index);

// TC/TLM functions (basic communication)
uint8_t ADCS_I2C_telecommand_wrapper(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum);
uint8_t ADCS_I2C_telemetry_wrapper(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum);
uint8_t ADCS_send_I2C_telecommand(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum);
uint8_t ADCS_send_I2C_telemetry_request(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum);

// CRC functions
uint8_t ADCS_COMMS_Crc8Init();
uint8_t ADCS_COMMS_Crc8Checksum(uint8_t* buffer, uint16_t len);

#endif /* INC_ADCS_INTERNAL_DRIVERS_H_ */