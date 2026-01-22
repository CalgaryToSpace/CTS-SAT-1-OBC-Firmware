#ifndef INCLUDE_GUARD__FLASH_DRIVER_H__
#define INCLUDE_GUARD__FLASH_DRIVER_H__

/*-----------------------------INCLUDES-----------------------------*/
#include <stdint.h>

#include "littlefs/flash_internal_spi.h"

/*----------------------------- CONFIG VARIABLES ----------------------------- */
// Number of CS pins available
#define FLASH_NUMBER_OF_FLASH_DEVICES 8 // TODO: update to 8, or 10 with FRAM maybe

// Total size of a singular Memory Module in bytes
#define FLASH_CHIP_SIZE_BYTES 134217728  // 128MiB // TODO: update

// Number of pages contained within a single block of memory module
#define FLASH_CHIP_PAGES_PER_BLOCK 64

// NAND Flash Memory Datasheet https://www.farnell.com/datasheets/3151163.pdf
// Each page is divided into a 2048-byte data storage region, and a 128 bytes spare area (2176 bytes total).
#define FLASH_CHIP_PAGE_SIZE_BYTES 2048

#define FLASH_CHIP_BLOCK_SIZE_BYTES (FLASH_CHIP_PAGE_SIZE_BYTES * FLASH_CHIP_PAGES_PER_BLOCK)

/*----------------------------- FLASH DATA TYPES -----------------------------*/

/*
A struct representing the the location of a block, page and byte within that page. More compact representations are available (uint64_t) but this is easier to use.
- row_address: address to the start of a page in pages (NOT bytes). 
- col_address: address to a specific byte in the page (in bytes, between [0, 2047]). If you are writing a full page, this should be 0.
*/
typedef struct {
    uint32_t row_address;
    uint32_t col_address;
} FLASH_Physical_Address_t;



/*-------------------------------FLASH FEATURES-------------------------------*/
// Features that can be accessed using Get Feature command
typedef enum {
    FLASH_FEAT_BLOCK_LOCK = 0xA0, // Block Lock 
    FLASH_FEAT_CONFIG = 0xB0, // Configuration Register
    FLASH_FEAT_STATUS = 0xC0, // Status Register
    FLASH_FEAT_DIE_SELECT = 0xD0, // Die Select
} FLASH_FEATURE_REGISTER_ADDR;

typedef enum {
    FLASH_FEAT_STATE_ENABLED = 0x01,
    FLASH_FEAT_STATE_DISABLED = 0x00,
} FLASH_Feat_State_Enum_t;


// ------------------- Status Register 1 - Byte Masks -------------------
// Source: Table 5
static const uint8_t FLASH_OP_IN_PROGRESS_MASK = (1 << 0);
static const uint8_t FLASH_SR1_WRITE_ENABLE_LATCH_MASK = (1 << 1);
static const uint8_t FLASH_SR1_PROGRAMMING_ERROR_MASK = (1 << 3);
static const uint8_t FLASH_SR1_ERASE_ERROR_MASK = (1 << 2);


/*-----------------------------DRIVER FUNCTIONS-----------------------------*/
FLASH_error_enum_t FLASH_init(uint8_t chip_number);
FLASH_error_enum_t FLASH_read_status_register(uint8_t chip_number, uint8_t *response);

FLASH_error_enum_t FLASH_erase_block(uint8_t chip_number, FLASH_Physical_Address_t address);
FLASH_error_enum_t FLASH_program_page(uint8_t chip_number, FLASH_Physical_Address_t address, uint8_t *data, uint32_t data_len);
FLASH_error_enum_t FLASH_read_page(
    uint8_t chip_number, FLASH_Physical_Address_t address, uint8_t *rx_buffer, uint32_t rx_buffer_size
);


FLASH_error_enum_t FLASH_is_reachable(uint8_t chip_number);
FLASH_error_enum_t FLASH_reset(uint8_t chip_number);
void FLASH_enable_then_disable_chip_select(uint8_t chip_number);


#endif /* INCLUDE_GUARD__FLASH_DRIVER_H__ */
