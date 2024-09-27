#include "stm32_internal_flash_drivers/stm32_internal_flash_drivers.h"

#include "stm32l4r5xx.h"
#include "stm32l4xx_hal.h" // must include this before stm32l4xx_hal_flash.h
#include "stm32l4xx_hal_flash.h"
#include "stm32l4xx_hal_flash_ex.h"
#include <string.h>

/// @brief Writes data to the flash memory in chunks of 8 bytes.
/// @param address Address in the flash memory where the data will be written.
/// @param data uint8_t buffer containing the data to be written.
/// @param length Length of the data to be written.
/// @return 0 on success, > 0 on error
/// @note Currently, only allowed to write to golden copy region
/// @note Writes data in chunks of 8 bytes.
/// Ex: Suppose we wanted to write to address 0x00, and suppose that at address 0x00, the first 8 bytes looks like the following:
/// [1,2,3,4,5,6,7,8]. If we wanted to write [25,26,27,28], it would result in the following: [25,26,27,28,0,0,0,0], clearing the rest of the bytes.
uint8_t STM32_internal_flash_write(uint32_t address, uint8_t *data, uint32_t length, STM32_Internal_Flash_Write_Status_t *status)
{
    status->lock_status = HAL_OK;
    status->unlock_status = HAL_OK;
    status->write_status = HAL_OK;

    if (address < STM32_INTERNAL_FLASH_MEMORY_REGION_GOLDEN_COPY_ADDRESS)
    {
        return 1;
    }

    const uint32_t end_address = address + length;

    if (end_address > FLASH_BANK2_END)
    {
        return 2;
    }

    status->unlock_status = HAL_FLASH_Unlock();
    if (status->unlock_status != HAL_OK)
    {
        return 3;
    }

    // Clear all FLASH flags before starting the operation
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    for (uint32_t current_address = address; current_address < end_address; current_address += 8)
    {
        uint8_t data_to_write[8] = {0};

        // TODO: what to do if data is not 8 bytes long
        // Currently, it will set the rest of the values to 0

        // current_address - address is the number of bytes we have written
        // since the beginning of the function
        memcpy(data_to_write, data + (current_address - address), 8);

        const uint64_t double_word = *(uint64_t *)(data_to_write);

        status->write_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, current_address, double_word);
        if (status->write_status != HAL_OK)
        {
            break;
        }
    }

    status->lock_status = HAL_FLASH_Lock();
    if (status->lock_status != HAL_OK)
    {
        return 4;
    }

    if (status->write_status != HAL_OK)
    {
        return 5;
    }

    return 0;
}

/// @brief Reads data from the flash memory
/// @param address address to start reading from
/// @param buffer buffer to store the read data, must be length long
/// @param length number of bytes to read
/// @return 0 on success, > 0 on error
uint8_t STM32_internal_flash_read(uint32_t address, uint8_t *buffer, uint32_t length)
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
uint8_t STM32_internal_flash_erase(uint16_t start_page_erase, uint16_t number_of_pages_to_erase, uint32_t *page_error)
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

/// @brief Gets option bytes configuration from the stm32 internal flash memory
/// @param ob_data pointer to an FLASH_OBProgramInitTypeDef structure that
/// contains the configuration information for the programming.
/// @return 0 on success, > 0 on error
uint8_t STM32_internal_flash_get_option_bytes(FLASH_OBProgramInitTypeDef *ob_data)
{
    // returns void
    HAL_FLASHEx_OBGetConfig(ob_data);
    return 0;
}