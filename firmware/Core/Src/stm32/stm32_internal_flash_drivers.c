#include <string.h>

#include "stm32/stm32_internal_flash_drivers.h"

#include "stm32l4r5xx.h"
#include "stm32l4xx_hal.h" // must include this before stm32l4xx_hal_flash.h
#include "stm32l4xx_hal_flash.h"
#include "stm32l4xx_hal_flash_ex.h"

/// @brief Writes data to the flash memory in chunks of 8 bytes.
/// @param address Address in the flash memory where the data will be written.
/// @param data uint8_t buffer containing the data to be written.
/// @param length Length of the data to be written.
/// @return 0 on success, > 0 on error
/// @note Writes data in chunks of 8 bytes.
/// Ex: Suppose we wanted to write to address 0x00, and suppose that at address 0x00, the first 8 bytes looks like the following:
/// [1,2,3,4,5,6,7,8]. If we wanted to write [25,26,27,28], it would result in the following: [25,26,27,28,0,0,0,0], clearing the rest of the bytes.
STM32_Internal_Flash_Write_Return_t STM32_internal_flash_write(uint32_t address, uint8_t *data, uint32_t length, STM32_Internal_Flash_Write_Status_t *status)
{
    status->lock_status = HAL_OK;
    status->unlock_status = HAL_OK;
    status->write_status = HAL_OK;

    if (address < STM32_INTERNAL_FLASH_MEMORY_REGION_FLASH_BANK_1_ADDRESS)
    {
        return STM32_INTERNAL_FLASH_WRITE_ADDRESS_TOO_LOW;
    }

    const uint32_t end_address = address + length;
    if ((address < STM32_INTERNAL_FLASH_MEMORY_REGION_FLASH_BANK_2_ADDRESS)
     && (end_address > STM32_INTERNAL_FLASH_MEMORY_REGION_FLASH_BANK_2_ADDRESS))
    {
        return STM32_INTERNAL_FLASH_WRITE_ADDRESS_OVERLAPS_BOTH_FLASH_BANKS; // Address range overlaps both flash banks, which we should not allow
    }
    if (end_address > FLASH_BANK2_END)
    {
        return STM32_INTERNAL_FLASH_WRITE_ADDRESS_TOO_HIGH;
    }

    status->unlock_status = HAL_FLASH_Unlock();
    if (status->unlock_status != HAL_OK)
    {
        return STM32_INTERNAL_FLASH_WRITE_UNLOCK_FAILED;
    }

    // Clear all FLASH flags before starting the operation
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    uint32_t offset = 0;

    while (offset < length)
    {
        uint8_t double_word_buf[8];
        
        uint32_t bytes_to_copy = (length - offset >= 8) ? 8 : (length - offset);

        memcpy(double_word_buf, data + offset, bytes_to_copy);
        if (bytes_to_copy < 8)
        {
            // Pad the rest with 0xFF (safe default for flash)
            memset(double_word_buf + bytes_to_copy, 0xFF, 8 - bytes_to_copy);
        }

        uint64_t double_word = *(uint64_t *)double_word_buf;

        status->write_status = HAL_FLASH_Program(
            FLASH_TYPEPROGRAM_DOUBLEWORD,
            address + offset,
            double_word
        );

        if (status->write_status != HAL_OK)
        {
            break;
        }

        offset += 8;
    }

    status->lock_status = HAL_FLASH_Lock();
    if (status->lock_status != HAL_OK)
    {
        return STM32_INTERNAL_FLASH_WRITE_LOCK_FAILED;
    }

    if (status->write_status != HAL_OK)
    {
        return STM32_INTERNAL_FLASH_WRITE_OPERATION_FAILED;
    }

    return STM32_INTERNAL_FLASH_WRITE_SUCCESS;
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

/// @brief Erase pages from provided flash bank of flash memory
/// @param flash_bank 1 or 2, which flash bank to erase 
/// @param start_page_erase what page to start erasing from (0-255 for bank 1, 256-511 for bank 2, inclusive)
/// @param number_of_pages_to_erase how many pages to erase. MAX is 256, MIN is 1.
/// @param page_error address of page which failed on error, defaults to UINT32_MAX on success
/// @return 0 on success, 1 if HAL_FLASH_Unlock() failed, 2 if HAL_FLASH_Lock() failed
uint8_t STM32_internal_flash_page_erase(uint8_t flash_bank, uint16_t start_page_erase, uint16_t number_of_pages_to_erase, uint32_t *page_error)
{
    if (flash_bank != 1 && flash_bank != 2)
    {
        return 1; // Invalid flash bank
    }

    if (flash_bank == 1 && (start_page_erase > FLASH_BANK_1_END_PAGE))
    {
        return 2; // Invalid page range for flash bank 1
    }
    
    if (flash_bank == 2 && (start_page_erase < FLASH_BANK_2_START_PAGE || start_page_erase > FLASH_BANK_2_END_PAGE))
    {
        return 3; // Invalid page range for flash bank 2
    }
    // Start page and flash bank are valid from this point
    if ((number_of_pages_to_erase < 1)
     || (number_of_pages_to_erase > NUMBER_OF_PAGES_PER_FLASH_BANK))
    {
        return 4; // Trying to erase more pages than available in the bank
    }

    const uint16_t end_page = start_page_erase + number_of_pages_to_erase - 1;
    if ((flash_bank == 1 && end_page > FLASH_BANK_1_END_PAGE)
     || (flash_bank == 2 && end_page > FLASH_BANK_2_END_PAGE))
    {
        return 5; // Trying to erase pages that are out of range for the flash bank
    }
    
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    if (HAL_FLASH_Unlock() != HAL_OK)
    {
        return 6;
    }

    FLASH_EraseInitTypeDef EraseInitStruct =
        {
            .TypeErase = FLASH_TYPEERASE_PAGES,
            .Banks = flash_bank == 1 ? FLASH_BANK_1 : FLASH_BANK_2,
            .Page = start_page_erase,
            .NbPages = number_of_pages_to_erase
        };

    // page error is uint32_t max on success, contains address of page which failed on error, check
    // stm32l4xx_hal_flash_ex.h for more information or the docs for HAL_FLASHEx_Erase
    const HAL_StatusTypeDef erase_status = HAL_FLASHEx_Erase(&EraseInitStruct, page_error);

    if (HAL_FLASH_Lock() != HAL_OK)
    {
        return 7;
    }

    switch (erase_status)
    {
    case HAL_ERROR:
        return 8;
    case HAL_BUSY:
        return 9;
    case HAL_TIMEOUT:
        return 10;
    // must be HAL_OK
    default:
        return 0;
    }
}

/// @brief Erase flash bank
/// @param flash_bank 1 or 2, which flash bank to erase 
/// @param page_error address of page which failed on error, defaults to UINT32_MAX on success
/// @return 0 on success, more than 0 on error
uint8_t STM32_internal_flash_bank_erase(uint8_t flash_bank, uint32_t *bank_erase_error)
{
    const uint8_t current_active_flash_bank = STM32_internal_flash_get_active_flash_bank();

    if (flash_bank == current_active_flash_bank)
    {
        return 1; // Cannot erase the active flash bank
    }
    if (flash_bank != 1 && flash_bank != 2)
    {
        return 2; // Invalid flash bank
    }

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    if (HAL_FLASH_Unlock() != HAL_OK)
    {
        return 3;
    }

    FLASH_EraseInitTypeDef EraseInitStruct =
        {
            .TypeErase = FLASH_TYPEERASE_MASSERASE,
            .Banks = flash_bank == 1 ? FLASH_BANK_1 : FLASH_BANK_2,
            .Page = 0,
            .NbPages = NUMBER_OF_PAGES_PER_FLASH_BANK
        };

    // page error is uint32_t max on success, contains address of page which failed on error, check
    // stm32l4xx_hal_flash_ex.h for more information or the docs for HAL_FLASHEx_Erase
    const HAL_StatusTypeDef erase_status = HAL_FLASHEx_Erase(&EraseInitStruct, bank_erase_error);

    if (HAL_FLASH_Lock() != HAL_OK)
    {
        return 4;
    }

    switch (erase_status)
    {
    case HAL_ERROR:
        return 5;
    case HAL_BUSY:
        return 6;
    case HAL_TIMEOUT:
        return 7;
    // must be HAL_OK
    default:
        return 0;
    }
}

/// @brief Gets option bytes configuration from the stm32 internal flash memory
/// @param ob_data pointer to an FLASH_OBProgramInitTypeDef structure that
/// contains the configuration information for the programming.
/// @return 0 always
uint8_t STM32_internal_flash_get_option_bytes(FLASH_OBProgramInitTypeDef *ob_data)
{
    // returns void
    HAL_FLASHEx_OBGetConfig(ob_data);
    return 0;
}

/// @brief Sets the active flash bank to either 1 or 2 (only if it is different than the current active flash bank).
/// @brief On Success, will load application in desired flash bank
/// @brief By Default, the active flash bank is 1
/// @param wanted_active_flash_bank 1 or 2
/// @return 0 on success, > 0 otherwise
uint8_t STM32_internal_flash_set_active_flash_bank(uint8_t wanted_active_flash_bank)
{
    if (wanted_active_flash_bank != 1 && wanted_active_flash_bank != 2) 
    {
        return 1;
    }
    const uint8_t current_active_flash_bank = STM32_internal_flash_get_active_flash_bank();
    if (wanted_active_flash_bank == current_active_flash_bank) 
    {
        return 2;
    }

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);


    if (HAL_FLASH_Unlock() != HAL_OK)
    {
        return 10;
    }

    if (HAL_FLASH_OB_Unlock() != HAL_OK)
    {
        return 20;
    }

    // Get current option bytes configuration
    FLASH_OBProgramInitTypeDef option_byte_init;
    HAL_FLASHEx_OBGetConfig(&option_byte_init);
    const uint32_t wanted_bank_mode = wanted_active_flash_bank == 1 ? OB_BFB2_DISABLE : OB_BFB2_ENABLE;

    option_byte_init.OptionType = OPTIONBYTE_USER;
    option_byte_init.USERType = OB_USER_BFB2;
    option_byte_init.USERConfig = wanted_bank_mode; 

    // Apply the new option byte configuration
    if (HAL_FLASHEx_OBProgram(&option_byte_init) != HAL_OK)
    {
        return 30;
    }

    // Launch the option byte loading to apply changes
    // This will reset the device and load the application in the desired flash bank
    if (HAL_FLASH_OB_Launch() != HAL_OK)
    {
        return 40;
    }

    // Only reachable if loading flash bank was unnsuccessful
    if (HAL_FLASH_OB_Lock() != HAL_OK)
    {
        return 50;
    }

    if (HAL_FLASH_Lock() != HAL_OK)
    {
        return 60;
    }

    return 0;
}

/// @brief Returns active flash bank
/// @return 1 is Flash Bank 1, 2 is Flash Bank 2
uint8_t STM32_internal_flash_get_active_flash_bank()
{
    return (READ_BIT(FLASH->OPTR, FLASH_OPTR_BFB2) >> FLASH_OPTR_BFB2_Pos) + 1;
}
