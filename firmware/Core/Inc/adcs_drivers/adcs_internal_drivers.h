#ifndef INC_ADCS_INTERNAL_DRIVERS_H_
#define INC_ADCS_INTERNAL_DRIVERS_H_

#include "adcs_types.h"

#define ADCS_I2C_HANDLE &hi2c1
static const uint8_t ADCS_INCLUDE_CHECKSUM = 1;
static const uint8_t ADCS_NO_CHECKSUM = 0;
#define WRITE_STRUCT_TO_MEMORY(struct_to_write) // memory module function: write struct to memory

/* Function Definitions */

// byte transforms
uint8_t ADCS_switch_order(uint8_t *array, uint16_t value, int index);
uint8_t ADCS_switch_order_32(uint8_t *array, uint32_t value, int index);

// TC/TLM functions (basic communication)
uint8_t ADCS_I2C_telecommand_wrapper(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum);
uint8_t ADCS_I2C_telemetry_wrapper(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum);
uint8_t ADCS_send_I2C_telecommand(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum);
uint8_t ADCS_send_I2C_telemetry_request(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum);

// CRC functions
uint8_t ADCS_COMMS_Crc8Init();
uint8_t ADCS_COMMS_Crc8Checksum(uint8_t* buffer, uint16_t len);

// I2C/UART debug functions
uint8_t ADCS_send_UART_telecommand(UART_HandleTypeDef *huart, uint8_t id, uint8_t* data, uint32_t data_length);


#endif /* INC_ADCS_INTERNAL_DRIVERS_H_ */