/**
 * flash_driver.h
 *
 * Created on: June 1, 2024
 *     Author: Saksham Puri
 */

#ifndef INC_FLASH_DRIVER_H_
#define INC_FLASH_DRIVER_H_

/*-----------------------------INCLUDES-----------------------------*/
#include "main.h"
#include "littlefs/lfs.h"
#include "littlefs/lfs_util.h"
#include "debug_tools/debug_uart.h"

/*-----------------------------COMMAND VARIABLES-----------------------------*/
// ---------------------------------------- S25FL512S 512Mb (64MB) Datasheet
static const uint8_t FLASH_WRITE = 0x02; // Page Program - Section 9.5.2
static const uint8_t FLASH_READ = 0x03;  // Read - Section 9.4.1

static const uint8_t FLASH_4WRITE = 0x12; // Page Program - Section 9.5.2
static const uint8_t FLASH_4READ = 0x13;  // Read - Section 9.4.1

static const uint8_t FLASH_WRDI = 0x04; // Write Disable - Section 9.3.9
static const uint8_t FLASH_WREN = 0x06; // Write Enable - Section 9.3.8

static const uint8_t FLASH_SE = 0xD8;  // Sector Erase - Section 9.6.1
static const uint8_t FLASH_4SE = 0xDC; // Sector Erase - Section 9.6.1

static const uint8_t FLASH_RDSR1 = 0X05; // Read Status 1 - Section 9.3.1 / 7.6.1
static const uint8_t FLASH_RDSR2 = 0X07; // Read Status 2 - Section 9.3.2
static const uint8_t FLASH_CLSR = 0X30;  // Clear Status - Section 9.3.10

static const uint8_t FLASH_RDCR = 0X35; // Read Config - Section 9.3.3

static const uint8_t FLASH_RESET = 0xF0; // Software Reset - Section 9.9.1

/*-----------------------------DRIVER FUNCTIONS-----------------------------*/
void FLASH_activate_chip_select();
void FLASH_deactivate_chip_select();
void FLASH_read_status_register(SPI_HandleTypeDef *, uint8_t *);
void FLASH_write_enable(SPI_HandleTypeDef *);
void FLASH_write_disable(SPI_HandleTypeDef *);
uint8_t FLASH_erase(SPI_HandleTypeDef *, lfs_block_t);
uint8_t FLASH_write(SPI_HandleTypeDef *, uint8_t *, lfs_block_t, lfs_size_t);
uint8_t FLASH_read(SPI_HandleTypeDef *, uint8_t *, lfs_block_t, lfs_size_t);

#endif /* INC_FLASH_DRIVER_H_ */