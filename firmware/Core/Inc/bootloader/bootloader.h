#ifndef __INCLUDE_GUARD__BOOTLOADER_H__
#define __INCLUDE_GUARD__BOOTLOADER_H__

#include <stdint.h>

typedef struct
{
    uint32_t address;       // application address
    uint32_t *function_ptr; // stack pointer
} Bootloader_Jump_To_Application_Args_t;

void BOOTLOADER_Jump_To_Application(uint32_t address);

void BOOTLOADER_deinitialize_peripherals();

void BOOTLOADER_Jump_To_Golden_Copy();

#endif // __INCLUDE_GUARD__BOOTLOADER_H__
