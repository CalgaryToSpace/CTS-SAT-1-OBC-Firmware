#ifndef __INCLUDE_GUARD__FLASH_BANK_H__
#define __INCLUDE_GUARD__FLASH_BANK_H__
#include <stdint.h>
typedef enum
{
    Flash_Partitions_RAM = 0x20000000,
    Flash_Partitions_RAM_2 = 0x10000000,
    Flash_Partitions_RAM_3 = 0x20040000,
    Flash_Partitions_FLASH_BANK1 = 0x8000000,
    Flash_Partitions_FLASH_BANK2 = 0x08100000 // default FLASH address
} Flash_Partition_Adressess;

uint32_t Flash_Bank_Write(uint32_t address, uint8_t *data, uint32_t length);

uint8_t Flash_Bank_Read(uint32_t address, uint8_t *buffer, uint32_t length);

uint32_t Flash_Bank_Erase();

#endif // __INCLUDE_GUARD__FLASH_BANK_H__