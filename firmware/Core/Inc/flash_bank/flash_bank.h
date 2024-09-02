#ifndef __INCLUDE_GUARD__FLASH_BANK_H__
#define __INCLUDE_GUARD__FLASH_BANK_H__
#include <stdint.h>

uint32_t Flash_Bank_Write(uint32_t address, uint8_t *data, uint32_t length);

uint8_t Flash_Bank_Read(uint32_t address, uint8_t *buffer, uint32_t length);

uint32_t Flash_Bank_Erase();

#endif // __INCLUDE_GUARD__FLASH_BANK_H__