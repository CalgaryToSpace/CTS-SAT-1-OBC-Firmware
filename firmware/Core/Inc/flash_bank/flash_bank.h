#ifndef __INCLUDE_GUARD__FLASH_BANK_H__
#define __INCLUDE_GUARD__FLASH_BANK_H__
#include <stdint.h>

/// @brief  Flash Partitions
/// @note look in the STM32L4R5ZITx_FLASH.ld file
/// to see the address of each partition, update this as needed
typedef enum
{
    Flash_Partitions_RAM = 0x20000000,
    Flash_Partitions_RAM_2 = 0x10000000,
    Flash_Partitions_RAM_3 = 0x20040000,
    Flash_Partitions_FLASH_BANK1 = 0x8000000, // default FLASH address
    Flash_Partitions_FLASH_BANK2 = 0x8100000
} Flash_Partition_Adressess;

uint32_t Flash_Bank_Write(uint32_t address, uint8_t *data, uint32_t length);

uint8_t Flash_Bank_Read(uint32_t address, uint8_t *buffer, uint32_t length);

uint32_t Flash_Bank_Erase(uint16_t start_page_erase, uint16_t number_of_pages_to_erase);

#endif // __INCLUDE_GUARD__FLASH_BANK_H__