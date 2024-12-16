#ifndef __INCLUDE_GUARD__BOOTLOADER_H__
#define __INCLUDE_GUARD__BOOTLOADER_H__

#include <stdint.h>


typedef void (*pFunction)(void);

uint8_t BOOTLOADER_Jump_To_Application(uint32_t address);

void BOOTLOADER_deinitialize_peripherals();

uint8_t BOOTLOADER_Jump_To_Golden_Copy();

#endif // __INCLUDE_GUARD__BOOTLOADER_H__
