#include "telecommands/stm32_internal_flash_telecommand_defs.h"
#include "stm32_internal_flash/stm32_internal_flash.h"
#include "stm32l4xx_hal.h"
#include "telecommands/telecommand_args_helpers.h"

#include <stdio.h>
#include <string.h>

/// @brief Write data to the internal flash bank starting from address 0x08000000
/// @param args_str
/// - Arg 0: The data to write as a string
/// @note This telecommand is only for testing purposes, it is purposfully not fully fleshed out
/// as there is no intention on using this. Update as needed
/// @note Currently, this is only writes all data given as a string.
/// Thus, it is difficult to write integers directly
uint8_t TCMDEXEC_flash_bank_write(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel, char *response_output_buf, uint16_t response_output_buf_len)
{
    const uint32_t res = Internal_Flash_Bank_Write(INTERNAL_FLASH_PARTITION_FLASH_BANK2, (uint8_t *)args_str, strlen(args_str));
    if (res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error: %lu", res);
        return 1;
    }
    return 0;
}

/// @brief Read data from the internal flash bank starting from address 0x08000000
/// @param args_str
/// - Arg 0: The number of bytes to read as a uint64_t
uint8_t TCMDEXEC_flash_bank_read(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel, char *response_output_buf, uint16_t response_output_buf_len)
{
    uint64_t number_of_bytes_to_read = 0;
    const uint8_t parse_res = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &number_of_bytes_to_read);
    if (parse_res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error Parsing: %u", parse_res);
        return 1;
    }

    uint8_t read_buffer[number_of_bytes_to_read + 1]; // + 1 for null-terminator
    const uint8_t res = Internal_Flash_Bank_Read(INTERNAL_FLASH_PARTITION_FLASH_BANK2, read_buffer, sizeof(read_buffer));
    if (res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error: %u", res);
        return 1;
    }
    for (uint64_t i = 0; i < number_of_bytes_to_read; i++)
    {
        snprintf(response_output_buf + (i * 2), response_output_buf_len - (i * 2), "%02X", read_buffer[i]);
    }

    return 0;
}

/// @brief Erase a range of pages in the internal flash bank
/// @param args_str
/// - Arg 0: The starting page to erase as a uint64_t
/// - Arg 1: The number of pages to erase as a uint64_t
uint8_t TCMDEXEC_flash_bank_erase(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel, char *response_output_buf, uint16_t response_output_buf_len)
{
    uint64_t start_page_erase = 0;

    const uint8_t start_page_erase_parse_res = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &start_page_erase);
    if (start_page_erase_parse_res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error Parsing arg 0: %u", start_page_erase_parse_res);
        return 1;
    }

    uint64_t number_of_pages_to_erase = 0;
    const uint8_t number_of_pages_to_erase_parse_res = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &number_of_pages_to_erase);
    if (number_of_pages_to_erase_parse_res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error Parsing arg 1: %u", number_of_pages_to_erase_parse_res);
        return 1;
    }

    const uint32_t res = Internal_Flash_Bank_Erase((uint16_t)start_page_erase, (uint16_t)number_of_pages_to_erase);
    if (res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error: %lu", res);
        return 1;
    }
    return 0;
}