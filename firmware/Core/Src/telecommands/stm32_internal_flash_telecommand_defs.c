#include "telecommands/stm32_internal_flash_telecommand_defs.h"
#include "telecommands/telecommand_args_helpers.h"
#include "stm32_internal_flash/stm32_internal_flash.h"

#include "stm32l4xx_hal.h"
#include <stdio.h>
#include <string.h>

/// @brief Write data to the internal flash bank starting from address 0x08000000
/// @param args_str
/// - Arg 0: The data in hex format to write
/// - Arg 1: The offset to start writing from
/// @note This telecommand is only for testing purposes, it is purposfully not fully fleshed out
/// as there is no intention on using this. Update as needed
uint8_t TCMDEXEC_stm32_internal_flash_write(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel, char *response_output_buf, uint16_t response_output_buf_len)
{
    uint8_t write_hex_buffer[PAGESIZE] = {0};
    uint16_t write_hex_buffer_len = 0;
    const uint8_t parse_hex_buffer_res = TCMD_extract_hex_array_arg(args_str, 0, write_hex_buffer, sizeof(write_hex_buffer), &write_hex_buffer_len);
    if (parse_hex_buffer_res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error Parsing Arg 0: %u", parse_hex_buffer_res);
        return 1;
    }

    uint64_t offset = 0;
    const uint8_t parse_offset_res = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &offset);
    if (parse_offset_res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error Parsing Arg 1: %u", parse_offset_res);
        return 1;
    }
    const uint8_t write_res = STM32_Internal_Flash_Bank_Write(STM32_INTERNAL_FLASH_MEMORY_REGION_GOLDEN_COPY_ADDRESS + offset, write_hex_buffer, write_hex_buffer_len);
    if (write_res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error writing to flash: %u", write_res);
        return 1;
    }
    return 0;
}

/// @brief Read data from the internal flash bank starting from address 0x08000000
/// @param args_str
/// - Arg 0: The address to start reading from
/// - Arg 1: The number of bytes to read as a uint64_t

uint8_t TCMDEXEC_stm32_internal_flash_read(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel, char *response_output_buf, uint16_t response_output_buf_len)
{

    uint64_t address = 0;
    const uint8_t parse_address_res = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &address);
    if (parse_address_res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error Parsing Arg 0: %u", parse_address_res);
        return 1;
    }

    uint64_t number_of_bytes_to_read = 0;
    const uint8_t parse_res = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &number_of_bytes_to_read);
    if (parse_res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error Parsing Arg 1: %u", parse_res);
        return 1;
    }

    uint8_t read_buffer[number_of_bytes_to_read];
    const uint8_t res = STM32_Internal_Flash_Bank_Read(address, read_buffer, sizeof(read_buffer));
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

/// @brief Erase a range of pages in the internal flash bank.
/// Only Erases for Flash Bank 2.
/// @param args_str
/// - Arg 0: The starting page to erase as a uint64_t
/// - Arg 1: The number of pages to erase as a uint64_t
uint8_t TCMDEXEC_stm32_internal_flash_erase(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel, char *response_output_buf, uint16_t response_output_buf_len)
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

    uint32_t page_error = 0;
    const uint8_t erase_res = STM32_Internal_Flash_Bank_Erase((uint16_t)start_page_erase, (uint16_t)number_of_pages_to_erase, &page_error);
    if (erase_res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error erasing pages: %u - %u, error: %u, page error: %lu", (uint16_t)start_page_erase, (uint16_t)start_page_erase + (uint16_t)number_of_pages_to_erase, erase_res, page_error);
        return 1;
    }
    return 0;
}