#include "stm32_internal_flash/stm32_internal_flash.h"

#include "stm32l4r5xx.h"
#include "stm32l4xx_hal.h" // must include this before stm32l4xx_hal_flash.h
#include "stm32l4xx_hal_flash.h"
#include "stm32l4xx_hal_flash_ex.h"
#include <string.h>
/**
 * Writes data to the flash memory.
 *
 * This function takes in an address, a pointer to the data to be written, and the length of the data.
 * It unlocks the flash memory, clears all flash flags, and then writes the data to the flash memory in 8-byte chunks.
 * If any part of the write operation fails, it breaks out of the loop and returns the error status.
 * Finally, it locks the flash memory to disable the flash control register access.
 *
 * @param address The starting address in the flash memory where the data will be written.
 * @param data A pointer to the data to be written to the flash memory.
 * @param length The length of the data to be written.
 *
 * @return The status of the write operation. A value > 0 indicates an error, while a value of 0 indicates success.
 */
uint32_t Internal_Flash_Bank_Write(uint32_t address, uint8_t *data, uint32_t length)
{
    HAL_StatusTypeDef status;
    uint32_t endAddress = address + length;

    status = HAL_FLASH_Unlock();

    if (status != HAL_OK)
    {
        return 1;
    }

    // Clear all FLASH flags before starting the operation
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    // Write data to flash
    for (uint32_t currentAddress = address; currentAddress < endAddress; currentAddress += 8)
    {
        // currentAddress - address is the number of bytes already written
        // const uint64_t double_word = *(uint64_t *)(data + (currentAddress - address));
        uint8_t data_to_write[8] = {0};
        memcpy(data_to_write, data + (currentAddress - address), 8);
        const uint64_t double_word = *(uint64_t *)(data_to_write);
        // Write the word to flash
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, currentAddress, double_word);
        if (status != HAL_OK)
        {
            // If the program operation fails, break the loop
            break;
        }
    }

    // Lock the Flash to disable the flash control register access
    HAL_FLASH_Lock();

    return status;
}

/**
 * Reads data from the flash memory.
 *
 * This function takes in an address, a pointer to a buffer to store the read data, and the length of the data to be read.
 * It checks if the address is within the valid range of the flash memory and returns an error if it's not.
 * If the address is valid, it reads the data from the flash memory and stores it in the provided buffer.
 *
 * @param address The starting address in the flash memory where the data will be read from.
 * @param buffer A pointer to the buffer where the read data will be stored.
 * @param length The length of the data to be read.
 *
 * @return 0 if the read operation is successful, 1 if the address is out of range.
 */
uint8_t Internal_Flash_Bank_Read(uint32_t address, uint8_t *buffer, uint32_t length)
{
    // Check if the address is within the valid range of the flash memory
    if (address < INTERNAL_FLASH_MEMORY_REGION_GOLDEN_COPY_ADDRESS || (address + length) > (FLASH_BANK2_END))
    {
        return 1; // Return error if address is out of range
    }

    for (uint32_t i = 0; i < length - 1; i++)
    {
        buffer[i] = *(uint8_t *)(address + i);
    }
    buffer[length] = '\0';

    return 0; // Return success
}

/**
 * Erases a specified number of pages from the flash bank.
 *
 * This function takes in the starting page to erase and the number of pages to erase.
 * It clears any existing flash errors, unlocks the flash, and then erases the specified pages.
 * If the erase operation is successful, it returns 0. Otherwise, it returns the page error.
 *
 * @param start_page_erase The starting page to erase.
 * @param number_of_pages_to_erase The number of pages to erase.
 *
 * @return 0 if the erase operation is successful, page error otherwise.
 */
uint32_t Internal_Flash_Bank_Erase(uint16_t start_page_erase, uint16_t number_of_pages_to_erase)
{
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef EraseInitStruct =
        {
            .TypeErase = FLASH_TYPEERASE_PAGES,
            .Banks = FLASH_BANK_2,
            .Page = start_page_erase,
            .NbPages = number_of_pages_to_erase};

    uint32_t PageError = 0;
    const HAL_StatusTypeDef erase_status = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
    HAL_FLASH_Lock();
    if (erase_status == HAL_OK)
    {
        return 0;
    }

    return PageError;
}