#ifndef __INCLUDE_GUARD__STM32_INTERNAL_FLASH_H__
#define __INCLUDE_GUARD__STM32_INTERNAL_FLASH_H__
#include <stdint.h>

/// @brief  Flash Partitions
/// @note look in the STM32L4R5ZITx_FLASH.ld file
/// to see the address of each partition, update this as needed
typedef enum
{
    INTERNAL_FLASH_PARTITION_RAM = 0x20000000,
    INTERNAL_FLASH_PARTITION_RAM_2 = 0x10000000,
    INTERNAL_FLASH_PARTITION_RAM_3 = 0x20040000,
    INTERNAL_FLASH_PARTITION_FLASH_BANK1 = 0x8000000, // default FLASH address
    INTERNAL_FLASH_PARTITION_FLASH_BANK2 = 0x8100000
} Intternal_Flash_Partition_Adressess;

uint32_t Internal_Flash_Bank_Write(uint32_t address, uint8_t *data, uint32_t length);

uint8_t Internal_Flash_Bank_Read(uint32_t address, uint8_t *buffer, uint32_t length);

uint32_t Internal_Flash_Bank_Erase(uint16_t start_page_erase, uint16_t number_of_pages_to_erase);

#endif // __INCLUDE_GUARD__STM32_INTERNAL_FLASH_H__