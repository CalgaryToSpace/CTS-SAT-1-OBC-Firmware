#include "flash_bank_telecommands_defs.h"
#include "flash_bank.h"
#include "stm32l4xx_hal.h"
#include "telecommand_args_helpers.h"

#include <stdio.h>
#include <string.h>

uint8_t TCMDEXEC_Flash_Bank_Write(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel, char *response_output_buf, uint16_t response_output_buf_len)
{
    const uint32_t res = Flash_Bank_Write(Flash_Partitions_FLASH_BANK2, (uint8_t *)args_str, 10);
    if (res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error: %lu", res);
        return 1;
    }
    return 0;
}

uint8_t TCMDEXEC_Flash_Bank_Read(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel, char *response_output_buf, uint16_t response_output_buf_len)
{
    uint64_t number_of_bytes_to_read;
    const uint8_t parse_res = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &number_of_bytes_to_read);
    if (parse_res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error Parsing: %u", parse_res);
        return 1;
    }

    uint8_t read_buffer[number_of_bytes_to_read + 1];
    const uint8_t res = Flash_Bank_Read(Flash_Partitions_FLASH_BANK2, read_buffer, sizeof(read_buffer));
    if (res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error: %u", res);
        return 1;
    }
    snprintf(response_output_buf, response_output_buf_len, "%s", read_buffer);
    return 0;
}

uint8_t TCMDEXEC_Flash_Bank_Erase(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel, char *response_output_buf, uint16_t response_output_buf_len)
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

    const uint32_t res = Flash_Bank_Erase((uint16_t)start_page_erase, (uint16_t)number_of_pages_to_erase);
    if (res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error: %lu", res);
        return 1;
    }
    return 0;
}