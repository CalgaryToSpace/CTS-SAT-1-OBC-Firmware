/**
 *     Author: Saksham Puri
 */

#ifndef __INCLUDE_GUARD__FLASH_DRIVER_H__
#define __INCLUDE_GUARD__FLASH_DRIVER_H__

/*-----------------------------INCLUDES-----------------------------*/
#include <stdint.h>

#include "main.h"

#include "littlefs/lfs.h"


/*-----------------------------INCLUDES-----------------------------*/

// number of CS pins available
#define FLASH_NUMBER_OF_FLASH_DEVICES 8 // TODO: update to 8, or 10 with FRAM maybe
#define FLASH_CHIP_SIZE_BYTES 67108864 // 64MiB // TODO: update

/*-----------------------------COMMAND VARIABLES-----------------------------*/
// ---------------------------------------- S25FL512S 512Mb (64MB) Datasheet
static const uint8_t FLASH_READ = 0x03;  // Read - Section 9.4.1
static const uint8_t FLASH_4READ = 0x13;  // Read - Section 9.4.1

static const uint8_t FLASH_WRITE = 0x02; // Page Program - Section 9.5.2
static const uint8_t FLASH_4WRITE = 0x12; // Page Program - Section 9.5.2

static const uint8_t FLASH_SE = 0xD8;  // Sector Erase - Section 9.6.1
static const uint8_t FLASH_4SE = 0xDC; // Sector Erase - Section 9.6.1

static const uint8_t FLASH_WRDI = 0x04; // Write Disable - Section 9.3.9
static const uint8_t FLASH_WREN = 0x06; // Write Enable - Section 9.3.8

static const uint8_t FLASH_RDSR1 = 0x05; // Read Status 1 - Section 9.3.1 / 7.6.1
static const uint8_t FLASH_RDSR2 = 0x07; // Read Status 2 - Section 9.3.2
static const uint8_t FLASH_CLSR = 0x30;  // Clear Status - Section 9.3.10

static const uint8_t FLASH_RDCR = 0x35; // Read Config - Section 9.3.3

static const uint8_t FLASH_RESET = 0xF0; // Software Reset - Section 9.9.1


/*-----------------------------DRIVER FUNCTIONS-----------------------------*/
void FLASH_activate_chip_select(uint8_t chip_number);
void FLASH_deactivate_chip_select();
uint8_t FLASH_read_status_register(SPI_HandleTypeDef *hspi, uint8_t chip_number, uint8_t *buf);
uint8_t FLASH_write_enable(SPI_HandleTypeDef *hspi, uint8_t chip_number);
uint8_t FLASH_write_disable(SPI_HandleTypeDef *hspi, uint8_t chip_number);
uint8_t FLASH_erase(SPI_HandleTypeDef *hspi, uint8_t chip_number, lfs_block_t addr);
uint8_t FLASH_write(SPI_HandleTypeDef *hspi, uint8_t chip_number, lfs_block_t addr, uint8_t *packet_buffer, lfs_size_t size);
uint8_t FLASH_read_data(SPI_HandleTypeDef *hspi, uint8_t chip_number, lfs_block_t addr, uint8_t *rx_buffer, lfs_size_t rx_buffer_len);

uint8_t FLASH_is_reachable(SPI_HandleTypeDef *hspi, uint8_t chip_number);

#endif /* __INCLUDE_GUARD__FLASH_DRIVER_H__ */
