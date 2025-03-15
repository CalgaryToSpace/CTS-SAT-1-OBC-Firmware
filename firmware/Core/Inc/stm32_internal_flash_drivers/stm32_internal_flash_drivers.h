#ifndef __INCLUDE_GUARD__STM32_INTERNAL_FLASH_DRIVERS_H__
#define __INCLUDE_GUARD__STM32_INTERNAL_FLASH_DRIVERS_H__

#include <stdint.h>

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_flash.h"

/// @brief  Flash Partitions
/// @note look in the STM32L4R5ZITx_FLASH.ld file
/// to see the address of each partition, update this as needed
typedef enum
{
    STM32_INTERNAL_FLASH_MEMORY_REGION_RAM_ADDRESS = 0x20000000,
    STM32_INTERNAL_FLASH_MEMORY_REGION_RAM_2_ADDRESS = 0x10000000,
    STM32_INTERNAL_FLASH_MEMORY_REGION_RAM_3_ADDRESS = 0x20040000,
    STM32_INTERNAL_FLASH_MEMORY_REGION_FLASH_ADDRESS = 0x8000000, // default boot address
    STM32_INTERNAL_FLASH_MEMORY_REGION_GOLDEN_COPY_ADDRESS = 0x08100000
} STM32_INTERNAL_FLASH_memory_region_addresses_t;

typedef struct
{
    HAL_StatusTypeDef lock_status;
    HAL_StatusTypeDef unlock_status;
    HAL_StatusTypeDef write_status;
} STM32_Internal_Flash_Write_Status_t;

uint8_t STM32_internal_flash_write(uint32_t address, uint8_t *data, uint32_t length, STM32_Internal_Flash_Write_Status_t *status);

uint8_t STM32_internal_flash_read(uint32_t address, uint8_t *buffer, uint32_t length);

uint8_t STM32_internal_flash_erase(uint16_t start_page_erase, uint16_t number_of_pages_to_erase, uint32_t *page_error);

uint8_t STM32_internal_flash_get_option_bytes(FLASH_OBProgramInitTypeDef *ob_data);

uint8_t STM32_internal_flash_set_active_flash_bank(uint8_t wanted_active_flash_bank);

uint8_t STM32_internal_flash_get_active_flash_bank();

#endif // __INCLUDE_GUARD__STM32_INTERNAL_FLASH_DRIVERS_H__
