#include "stm32_internal_flash/stm32_internal_flash.h"

#include "stm32l4r5xx.h"
#include "stm32l4xx_hal.h" // must include this before stm32l4xx_hal_flash.h
#include "stm32l4xx_hal_flash.h"
#include "stm32l4xx_hal_flash_ex.h"
#include <string.h>

/// @brief Writes data to the flash memory.
/// @param address Address in the flash memory where the data will be written.
/// @param data uint8_t buffer containing the data to be written.
/// @param length Length of the data to be written.
/// @return 0 on success, > 0 on error, 10 if HAL_FLASH_Unlock() failed, 11 if HAL_FLASH_Lock() failed
uint32_t Internal_Flash_Bank_Write(uint32_t address, uint8_t *data, uint32_t length)
{
    const uint32_t end_address = address + length;

    if (HAL_FLASH_Unlock() != HAL_OK)
    {
        return 10;
    }

    // Clear all FLASH flags before starting the operation
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    HAL_StatusTypeDef status;

    for (uint32_t currentAddress = address; currentAddress < end_address; currentAddress += 8)
    {
        uint8_t data_to_write[8] = {0};

        // currentAddress - address is the number of bytes we have written
        // since the beginning of the function
        memcpy(data_to_write, data + (currentAddress - address), 8);

        const uint64_t double_word = *(uint64_t *)(data_to_write);

        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, currentAddress, double_word);
        if (status != HAL_OK)
        {
            break;
        }
    }

    // Lock the Flash to disable the flash control register access
    if (HAL_FLASH_Lock() != HAL_OK)
    {
        return 11;
    }

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