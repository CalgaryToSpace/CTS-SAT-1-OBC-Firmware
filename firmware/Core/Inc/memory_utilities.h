/**
 * memory_utilities.h
 *
 * Created on: May 2, 2024
 *     Author: Saksham Puri
 */

#ifndef INC_MEMORY_UTILITIES_H_
#define INC_MEMORY_UTILITIES_H_

/*-----------------------------INCLUDES-----------------------------*/
#include "main.h"
#include "littlefs/lfs.h"
#include "littlefs/lfs_util.h"

/*-----------------------------VARIABLES-----------------------------*/
                                            //S25FL512S 512Mb (64MB) Datasheet
static const uint8_t FLASH_WRITE = 0x02;    //Page Program - Section 9.5.2
static const uint8_t FLASH_READ = 0x03;     //Read - Section 9.4.1

static const uint8_t FLASH_WRDI = 0x04;     //Write Disable - Section 9.3.9
static const uint8_t FLASH_WREN = 0x06;     //Write Enable - Section 9.3.8

static const uint8_t FLASH_SE = 0xD8;       //Sector Erase - Section 9.6.1

static const uint8_t FLASH_RDSR1 = 0X05;    //Read Status 1 - Section 9.3.1
static const uint8_t FLASH_RDSR2 = 0X07;    //Read Status 2 - Section 9.3.2
static const uint8_t FLASH_CLSR = 0X30;     //Clear Status - Section 9.3.10

static const uint8_t FLASH_RDCR = 0X35;     //Read Config - Section 9.3.3

static const uint8_t FLASH_RESET = 0xF0;    //Software Reset - Section 9.9.1

/*---------------------------SETUP FUNCTIONS---------------------------*/
void LFS_Config();
int block_device_read(const struct lfs_config *, lfs_block_t, lfs_off_t, void *,
                      lfs_size_t);
int block_device_prog(const struct lfs_config *, lfs_block_t, lfs_off_t,
                      const void *, lfs_size_t);
int block_device_erase(const struct lfs_config *, lfs_block_t);
int block_device_sync(const struct lfs_config *);

/*---------------------------FUNCTIONS---------------------------*/
void PULL_CS();
void SET_CS();
void READ_STATUS_REGISTER(SPI_HandleTypeDef *, uint8_t *);
void ENABLE_WREN(SPI_HandleTypeDef *);
void ENABLE_WRDI(SPI_HandleTypeDef *);
uint8_t MEM_CLEAR_LFS(SPI_HandleTypeDef *, lfs_block_t);
uint8_t WRITE_LFS(SPI_HandleTypeDef *, uint8_t *, lfs_block_t, lfs_size_t);
uint8_t READ_LFS(SPI_HandleTypeDef *, uint8_t *, lfs_block_t, lfs_size_t);

void INITIALIZE();
int8_t FORMAT();
int8_t MOUNT();
int8_t UNMOUNT();
int8_t FORCE_MOUNT();
int8_t WRITE_FILE(char *, void *, uint32_t);
int8_t READ_FILE(char *, void *, uint32_t);

#endif /* INC_MEMORY_UTILITIES_H_ */
