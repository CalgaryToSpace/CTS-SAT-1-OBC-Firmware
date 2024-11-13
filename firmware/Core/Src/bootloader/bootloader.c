#include "bootloader/bootloader.h"
#include "stm32_internal_flash_drivers/stm32_internal_flash_drivers.h"

#include "main.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

typedef void (*pFunction)(void);

void BOOTLOADER_deinitialize_peripherals()
{

    HAL_UART_DeInit(&huart1);
    HAL_UART_DeInit(&huart2);
    HAL_UART_DeInit(&huart3);
    HAL_UART_DeInit(&huart4);
    HAL_UART_DeInit(&huart5);

    HAL_I2C_DeInit(&hi2c1);
    HAL_I2C_DeInit(&hi2c2);
    HAL_I2C_DeInit(&hi2c3);
    HAL_I2C_DeInit(&hi2c4);

    HAL_SPI_DeInit(&hspi1);

    // think about adding hcrc in main.c to main.h
    // HAL_CRC_DeInit(&hcrc);

    HAL_TIM_Base_DeInit(&htim16);

    // Disable all interrupts
    __disable_irq();

    // Reset the RCC clock configuration to the default reset state
    HAL_RCC_DeInit();

    // Deinitialize the HAL library
    HAL_DeInit();
    // Disable the SysTick interrupt and timer
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
}

void BOOTLOADER_Jump_To_Application(uint32_t address)
{
    // // Check if there is a valid stack pointer in the app's vector table (first address)
    // uint32_t appStack = *(__IO uint32_t *)address;
    // if (((appStack & 0x2FFE0000) == 0x20000000) || (((appStack & 0x1FFF8000) == 0x10000000)))
    // { // Check if within valid SRAM range

    //     BOOTLOADER_deinitialize_peripherals();

    //     // // Set the Vector Table Offset Register (VTOR) to the application's vector table
    //     // SCB->VTOR = address;

    //     // // Load the new stack pointer and jump to the reset handler (address + 4)
    //     // __set_MSP(*(__IO uint32_t *)address);                              // Set the stack pointer
    //     // pFunction appEntry = (pFunction)(*(__IO uint32_t *)(address + 4)); // Reset handler
    //     // appEntry();                                                        // Jump to the application

    //     // This should never be reached if the jump succeeds
    // }
    // else
    // {
    //     // Invalid stack pointer, handle error (e.g., blink an LED or reset)
    //     while (1)
    //     {
    //         // Error loop
    //     }
    // }
    BOOTLOADER_deinitialize_peripherals();
    // const uint32_t first = address & 0xFFFF0000;
    // const uint32_t second = address & 0x0000FFFF;
    const Bootloader_Jump_To_Application_Args_t *vector_p = (Bootloader_Jump_To_Application_Args_t *)(address);
    // asm("msr msp, %0; bx %1;" : : "r"(first), "r"(second));
    __set_MSP(address);
    pFunction appEntry = (pFunction)(*(__IO uint32_t *)(vector_p->function_ptr)); // Reset handler
    appEntry();                                                                   // Jump to the application
    // asm("msr msp, %0; bx %1;" : : "r"(vector_p->address), "r"(vector_p->function_ptr));
}

void BOOTLOADER_Jump_To_Golden_Copy()
{
    BOOTLOADER_Jump_To_Application(STM32_INTERNAL_FLASH_MEMORY_REGION_GOLDEN_COPY_ADDRESS);
    return;
}