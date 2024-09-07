#ifndef __INCLUDE_GUARD__FLASH_DRIVER_H__
#define __INCLUDE_GUARD__FLASH_DRIVER_H__

/*-----------------------------INCLUDES-----------------------------*/
#include <stdint.h>

#include "main.h"

#include "littlefs/lfs.h"

/*----------------------------- CONFIG VARIABLES ----------------------------- */
// Number of CS pins available
#define FLASH_NUMBER_OF_FLASH_DEVICES 8 // TODO: update to 8, or 10 with FRAM maybe

// Total size of a singular Memory Module in bytes
#define FLASH_CHIP_SIZE_BYTES 134217728  // 128MiB // TODO: update

/*-------------------------------FLASH FEATURES-------------------------------*/
// Features that can be accessed using Get Feature command

static const uint8_t FLASH_FEAT_BLOCK_LOCK = 0xA0; // Block Lock 

static const uint8_t FLASH_FEAT_CONFIG = 0xB0; // Configuration Register

static const uint8_t FLASH_FEAT_STATUS = 0xC0; // Status Register

static const uint8_t FLASH_FEAT_DIE_SELECT = 0xD0; // Die Select

/*-----------------------------COMMAND VARIABLES-----------------------------*/
// All comments in this section refer to the "MT29F1G 1Gib (128MiB)" Datasheet by Micron

static const uint8_t FLASH_CMD_PAGE_READ = 0x13; // Read Page
static const uint8_t FLASH_CMD_READ_FROM_CACHE = 0x03; // Read Page

static const uint8_t FLASH_CMD_PROGRAM_LOAD = 0x02; // Load Program into cache resgiters
static const uint8_t FLASH_CMD_PROGRAM_EXEC = 0x10; // Send data from cache to memory

static const uint8_t FLASH_CMD_BLOCK_ERASE = 0xD8; // Block Erase

static const uint8_t FLASH_CMD_WRITE_ENABLE = 0x06;  // Write Enable
static const uint8_t FLASH_CMD_WRITE_DISABLE = 0x04; // Write Disable

static const uint8_t FLASH_CMD_GET_FEATURES = 0x0F; // Get Features
static const uint8_t FLASH_CMD_SET_FEATURES = 0x1F; // Set Features

static const uint8_t FLASH_CMD_READ_ID = 0x9F; // Read ID (0x2C 0x14)

static const uint8_t FLASH_CMD_RESET = 0xFF; // Reset operation

// ------------------- Status Register 1 - Byte Masks -------------------
// Source: Table 5
static const uint8_t FLASH_SR1_WRITE_IN_PROGRESS_MASK = (1 << 0);
static const uint8_t FLASH_SR1_WRITE_ENABLE_LATCH_MASK = (1 << 1);
static const uint8_t FLASH_SR1_PROGRAMMING_ERROR_MASK = (1 << 3);
static const uint8_t FLASH_SR1_ERASE_ERROR_MASK = (1 << 2);

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

/*-----------------------------DRIVER FUNCTIONS-----------------------------*/
void FLASH_activate_chip_select(uint8_t chip_number);
void FLASH_deactivate_chip_select();
FLASH_error_enum_t FLASH_unblock_block_lock(SPI_HandleTypeDef *hspi, uint8_t chip_number, uint8_t *buf);
FLASH_error_enum_t FLASH_read_status_register(SPI_HandleTypeDef *hspi, uint8_t chip_number, uint8_t *buf);
FLASH_error_enum_t FLASH_read_block_lock_register(SPI_HandleTypeDef *hspi, uint8_t chip_number, uint8_t *buf);
FLASH_error_enum_t FLASH_write_enable(SPI_HandleTypeDef *hspi, uint8_t chip_number);
FLASH_error_enum_t FLASH_write_disable(SPI_HandleTypeDef *hspi, uint8_t chip_number);
FLASH_error_enum_t FLASH_erase(SPI_HandleTypeDef *hspi, uint8_t chip_number, lfs_block_t addr);
FLASH_error_enum_t FLASH_write_data(SPI_HandleTypeDef *hspi, uint8_t chip_number, lfs_block_t addr, uint8_t *packet_buffer, lfs_size_t packet_buffer_len);
FLASH_error_enum_t FLASH_read_data(SPI_HandleTypeDef *hspi, uint8_t chip_number, lfs_block_t addr, uint8_t *rx_buffer, lfs_size_t rx_buffer_len);

FLASH_error_enum_t FLASH_is_reachable(SPI_HandleTypeDef *hspi, uint8_t chip_number);
FLASH_error_enum_t FLASH_reset(SPI_HandleTypeDef *hspi, uint8_t chip_number);

#endif /* __INCLUDE_GUARD__FLASH_DRIVER_H__ */
