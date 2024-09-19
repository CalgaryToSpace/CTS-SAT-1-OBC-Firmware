#include "stm32_internal_flash/stm32_internal_flash.h"

#include "stm32l4r5xx.h"
#include "stm32l4xx_hal.h" // must include this before stm32l4xx_hal_flash.h
#include "stm32l4xx_hal_flash.h"
#include "stm32l4xx_hal_flash_ex.h"
#include <string.h>

/// @brief Writes data to the flash memory in chunks of 8 bytes.
/// @param address Address in the flash memory where the data will be written.
/// @param data uint8_t buffer containing the data to be written.
/// @param length Length of the data to be written.
/// @return 0 on success, > 0 on error, 10 if HAL_FLASH_Unlock() failed, 11 if HAL_FLASH_Lock() failed
/// @note Currently, only allowed to write to golden copy region
/// @note if data is not 8 bytes long, it will write the data given, and clear the rest of the data
/// Ex: write: 0x01020304, will write 0x0102030400000000, clearing the 8 bytes infront
uint8_t STM32_Internal_Flash_Bank_Write(uint32_t address, uint8_t *data, uint32_t length)
{
    if (address < STM32_INTERNAL_FLASH_MEMORY_REGION_GOLDEN_COPY_ADDRESS)
    {
        return 1;
    }

    const uint32_t end_address = address + length;

    if (end_address > FLASH_BANK2_END)
    {
        return 2;
    }

    if (HAL_FLASH_Unlock() != HAL_OK)
    {
        return 10;
    }

    // Clear all FLASH flags before starting the operation
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    HAL_StatusTypeDef status = HAL_OK;

    for (uint32_t currentAddress = address; currentAddress < end_address; currentAddress += 8)
    {
        uint8_t data_to_write[8] = {0};

        // TODO: what to do if data is not 8 bytes long
        // Currently, it will set the rest of the values to 0

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

    if (HAL_FLASH_Lock() != HAL_OK)
    {
        return 11;
    }

    switch (status)
    {
    case HAL_OK:
        return 0;
    case HAL_ERROR:
        return 4;
    case HAL_BUSY:
        return 5;
    case HAL_TIMEOUT:
        return 6;
    default:
        return 0;
    }
}

/// @brief Reads data from the flash memory
/// @param address address to start reading from
/// @param buffer buffer to store the read data, must be length long
/// @param length number of bytes to read
/// @return 0 on success, > 0 on error
uint8_t STM32_Internal_Flash_Bank_Read(uint32_t address, uint8_t *buffer, uint32_t length)
{
    if (address + length > FLASH_BANK2_END)
    {
        return 1; // Return error if address is out of range
    }

    for (uint32_t i = 0; i < length; i++)
    {
        buffer[i] = *(uint8_t *)(address + i);
    }

    return 0; // Return success
}

/// @brief Erase pages from bank 2 of flash memory which is located at 0x08100000
/// @param start_page_erase what page to start erasing from
/// @param number_of_pages_to_erase how many pages to erase
/// @param page_error address of page which failed on error, defaults to UINT32_MAX on success
/// @return 0 on success, 1 if HAL_FLASH_Unlock() failed, 2 if HAL_FLASH_Lock() failed
uint8_t STM32_Internal_Flash_Bank_Erase(uint16_t start_page_erase, uint16_t number_of_pages_to_erase, uint32_t *page_error)
{
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    if (HAL_FLASH_Unlock() != HAL_OK)
    {
        return 1;
    }

    FLASH_EraseInitTypeDef EraseInitStruct =
        {
            .TypeErase = FLASH_TYPEERASE_PAGES,
            .Banks = FLASH_BANK_2,
            .Page = start_page_erase,
            .NbPages = number_of_pages_to_erase};

    // page error is uint32_t max on success, contains address of page which failed on error, check
    // stm32l4xx_hal_flash_ex.h for more information or the docs for HAL_FLASHEx_Erase
    const HAL_StatusTypeDef erase_status = HAL_FLASHEx_Erase(&EraseInitStruct, page_error);

    if (HAL_FLASH_Lock() != HAL_OK)
    {
        return 2;
    }

    switch (erase_status)
    {
    case HAL_ERROR:
        return 3;
    case HAL_BUSY:
        return 4;
    case HAL_TIMEOUT:
        return 5;
    // must be HAL_OK
    default:
        return 0;
    }
}