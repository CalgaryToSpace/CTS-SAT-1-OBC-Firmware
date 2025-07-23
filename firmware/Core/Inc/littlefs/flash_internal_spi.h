#ifndef INCLUDE_GUARD__FLASH_INTERNAL_SPI_H__
#define INCLUDE_GUARD__FLASH_INTERNAL_SPI_H__
#include <stdint.h>


/*-----------------------------FLASH ERROR CODES-----------------------------*/
typedef enum {
    FLASH_ERR_OK                    = 0,    // No error occurred
    FLASH_ERR_SPI_TRANSMIT_FAILED   = -3,   // Error occurred while transmitting SPI signal
    FLASH_ERR_SPI_RECEIVE_FAILED    = -4,   // Error occurred while receiving SPI signal
    FLASH_ERR_DEVICE_BUSY_TIMEOUT   = -6,   // Took too long for the device to be in standby
    FLASH_ERR_UNKNOWN               = -7,   // Unknown error occurred (code reached where it shouldn't have been possible)
    FLASH_ERR_STATUS_REG_ERROR      = -8,   // Error occurred which was indicated by one of the Status Register Bits.
    FLASH_ERR_SPI_TRANSMIT_TIMEOUT  = -10,  // Timeout when transmitting SPI signal
    FLASH_ERR_SPI_RECEIVE_TIMEOUT   = -11   // Timeout when receiving SPI signal
} FLASH_error_enum_t;


/*-----------------------------FLASH TYPES-----------------------------*/
typedef struct {
    uint8_t *data;
    uint16_t len;
} FLASH_SPI_Data_t;


/*-----------------------------FLASH COMMAND VARIABLES-----------------------------*/
typedef enum {
    // All comments refer to the "MT29F1G 1Gib (128MiB)" Datasheet by Micron

    FLASH_CMD_PAGE_READ        = 0x13, // Read Page
    FLASH_CMD_READ_FROM_CACHE  = 0x03, // Read from Cache

    FLASH_CMD_PROGRAM_LOAD     = 0x02, // Load Program into cache registers
    FLASH_CMD_PROGRAM_EXECUTE     = 0x10, // Send data from cache to memory

    FLASH_CMD_BLOCK_ERASE      = 0xD8, // Block Erase

    FLASH_CMD_WRITE_ENABLE     = 0x06, // Write Enable
    FLASH_CMD_WRITE_DISABLE    = 0x04, // Write Disable

    FLASH_CMD_GET_FEATURES     = 0x0F, // Get Features
    FLASH_CMD_SET_FEATURES     = 0x1F, // Set Features

    FLASH_CMD_READ_ID          = 0x9F, // Read ID (0x2C 0x14)

    FLASH_CMD_RESET            = 0xFF  // Reset operation

} FLASH_command_t;


/*-----------------------------FLASH SPI DRIVER FUNCTIONS-----------------------------*/
FLASH_error_enum_t FLASH_SPI_send_command(FLASH_SPI_Data_t *cmd, uint8_t chip_number);
FLASH_error_enum_t FLASH_SPI_send_command_with_data(FLASH_SPI_Data_t *cmd,FLASH_SPI_Data_t *data, uint8_t chip_number);
FLASH_error_enum_t FLASH_SPI_send_command_receive_response(FLASH_SPI_Data_t *cmd, uint8_t *response, uint16_t response_len, uint8_t chip_number);

void FLASH_SPI_enable_then_disable_chip_select(uint8_t chip_number);

#endif // INCLUDE_GUARD__FLASH_INTERNAL_SPI_H__