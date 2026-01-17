#include <stdio.h>
#include <string.h>

#include "stm32l4xx_hal.h"

#include "telecommands/stm32_internal_flash_telecommand_defs.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "stm32/stm32_internal_flash_drivers.h"
#include "littlefs/littlefs_helper.h"
#include "log/log.h"


/// @brief Write data to the internal flash bank starting from address 0x08100000
/// @param args_str
/// - Arg 0: Address to start writing from in hex format 
/// - Arg 1: The data in hex format to write up to PAGESIZE bytes (0x1000 bytes)
/// @note This telecommand is only for testing purposes, it is purposfully not fully fleshed out
/// as there is no intention on using this. Update as needed
//// @return 0 on success, > 0 on error
uint8_t TCMDEXEC_stm32_internal_flash_write(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len)
{
    uint8_t address_buf[4] = {0};
    uint16_t address_len = 0;
    const uint8_t parse_address_res = TCMD_extract_hex_array_arg(args_str, 0, address_buf, sizeof(address_buf), &address_len);
    if (parse_address_res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error Parsing Arg 0: %u", parse_address_res);
        return 1;
    }

    uint8_t write_hex_buffer[PAGESIZE] = {0};
    uint16_t write_hex_buffer_len = 0;
    const uint8_t parse_hex_buffer_res = TCMD_extract_hex_array_arg(args_str, 1, write_hex_buffer, sizeof(write_hex_buffer), &write_hex_buffer_len);
    if (parse_hex_buffer_res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error Parsing Arg 1: %u", parse_hex_buffer_res);
        return 2;
    }
    
    const uint32_t address = (address_buf[0] << 24) |
                             (address_buf[1] << 16) |
                             (address_buf[2] << 8) |
                             address_buf[3]; // Convert to 32-bit address

    STM32_internal_flash_write_status_t status;
    const STM32_internal_flash_write_return_t write_res = STM32_internal_flash_write(address, write_hex_buffer, write_hex_buffer_len, &status);
    if (write_res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error writing to flash: %u\nLock Status: %u\nUnlock Status: %u\nWrite Status: %u", write_res, status.lock_status, status.unlock_status, status.write_status);
        return 1;
    }
    return 0;
}

/// @brief Read data from the internal flash bank
/// @param args_str
/// - Arg 0: The address to start reading from in hex
/// - Arg 1: The number of bytes to read as a uint64_t
//// @return 0 on success, > 0 on error
uint8_t TCMDEXEC_stm32_internal_flash_read(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len)
{
    uint8_t address_buf[4] = {0};
    uint16_t address_len = 0;
    const uint8_t parse_address_res = TCMD_extract_hex_array_arg(args_str, 0, address_buf, sizeof(address_buf), &address_len);

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
        return 2;
    }

    uint8_t read_buffer[number_of_bytes_to_read];
    const uint32_t address = (address_buf[0] << 24) |
                             (address_buf[1] << 16) |
                             (address_buf[2] << 8) |
                             address_buf[3]; // Convert to 32-bit address
    const uint8_t res = STM32_internal_flash_read(address, read_buffer, sizeof(read_buffer));
    if (res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error: %u", res);
        return 3;
    }
    char *p = response_output_buf;
    for (uint64_t i = 0; i < number_of_bytes_to_read && (p - response_output_buf + 2 < response_output_buf_len); i++)
    {
        p += snprintf(p, response_output_buf_len - (p - response_output_buf), "%02X", read_buffer[i]);
    }

    return 0;
}

/// @brief Erase a range of pages in the internal flash bank.
/// @param args_str
/// - Arg 0: Flash Bank to erase (1 or 2)
/// - Arg 1: The starting page to erase (0-255 for bank 1, 256-511 for bank 2)
/// - Arg 2: The number of pages to erase (1-256)
//// @return 0 on success, > 0 on error
uint8_t TCMDEXEC_stm32_internal_flash_page_erase(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len)
{
    uint64_t flash_bank_to_erase = 0;
    const uint8_t flash_bank_to_erase_parse_res = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &flash_bank_to_erase);
    if (flash_bank_to_erase_parse_res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error Parsing arg 0: %u", flash_bank_to_erase_parse_res);
        return 1;
    }
    if (flash_bank_to_erase != 1 && flash_bank_to_erase != 2)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error: Only valid options for flash banks: 1 or 2, got: %u", (uint8_t)flash_bank_to_erase);
        return 1;
    }
    uint64_t start_page_erase = 0;

    const uint8_t start_page_erase_parse_res = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &start_page_erase);
    if (start_page_erase_parse_res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error Parsing arg 1: %u", start_page_erase_parse_res);
        return 1;
    }

    uint64_t number_of_pages_to_erase = 0;
    const uint8_t number_of_pages_to_erase_parse_res = TCMD_extract_uint64_arg(args_str, strlen(args_str), 2, &number_of_pages_to_erase);
    if (number_of_pages_to_erase_parse_res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error Parsing arg 2: %u", number_of_pages_to_erase_parse_res);
        return 1;
    }

    uint32_t page_error = 0;
    const uint8_t erase_res = STM32_internal_flash_page_erase((uint8_t)flash_bank_to_erase, (uint16_t)start_page_erase, (uint16_t)number_of_pages_to_erase, &page_error);
    if (erase_res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error erasing pages: %u -> %u, error: %u, page error: %lu", (uint16_t)start_page_erase, (uint16_t)start_page_erase + (uint16_t)number_of_pages_to_erase, erase_res, page_error);
        return 1;
    }
    return 0;
}

/// @brief Erase an entire flash bank
/// @param args_str
/// - Arg 0: Flash Bank to erase (1 or 2)
/// @return 0 on success, > 0 on error
uint8_t TCMDEXEC_stm32_internal_flash_bank_erase(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len)
{
    uint64_t flash_bank_to_erase = 0;
    const uint8_t flash_bank_to_erase_parse_res = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &flash_bank_to_erase);
    if (flash_bank_to_erase_parse_res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error Parsing arg 0: %u", flash_bank_to_erase_parse_res);
        return 1;
    }

    if (flash_bank_to_erase != 1 && flash_bank_to_erase != 2)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error: Only valid options for flash banks: 1 or 2, got: %u", (uint8_t)flash_bank_to_erase);
        return 2;
    }
    uint32_t bank_erase_error = 0;
    const uint8_t erase_res = STM32_internal_flash_bank_erase((uint8_t)flash_bank_to_erase, &bank_erase_error);
    if (erase_res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error erasing bank: %u", (uint8_t)flash_bank_to_erase);
        return 3;
    }
    snprintf(response_output_buf, response_output_buf_len, "Successfully erased flash bank: %u", (uint8_t)flash_bank_to_erase);
    return 0;
}


/// @brief Get the option bytes configuration from the stm32 internal flash memory
/// @param args_str No args
/// @return 0 on success, > 0 on error
uint8_t TCMDEXEC_stm32_internal_flash_get_option_bytes(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len)
{
    FLASH_OBProgramInitTypeDef option_bytes;
    const uint8_t res = STM32_internal_flash_get_option_bytes(&option_bytes);
    if (res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error: %u", res);
        return 1;
    }

    snprintf(response_output_buf, response_output_buf_len,
         "{\"OptionType\":%lu,\"WRPArea\":%lu,\"WRPStartOffset\":%lu,"
         "\"WRPEndOffset\":%lu,\"RDPLevel\":%lu,\"USERType\":%lu,"
         "\"USERConfig\":%lu,\"PCROPConfig\":%lu,\"PCROPStartAddr\":\"0x%lX\","
         "\"PCROPEndAddr\":\"0x%lX\"}",
         option_bytes.OptionType,
         option_bytes.WRPArea,
         option_bytes.WRPStartOffset,
         option_bytes.WRPEndOffset,
         option_bytes.RDPLevel,
         option_bytes.USERType,
         option_bytes.USERConfig,
         option_bytes.PCROPConfig,
         option_bytes.PCROPStartAddr,
         option_bytes.PCROPEndAddr);

    return 0;
}

/// @brief Given a 1 or 2, switches to the respective flash bank and runs the application stored there if present.
/// Mostly useful for switching between 2 different version of the firmware (1 will be stored in Flash Bank 1, other will be stored in Flash Bank 2)
/// @param args_str 
/// - Arg 0: A 1 or 2. 1 to switch to the application present in Flash Bank 1, 2 to switch to the application present in Flash Bank 2
/// @param response_output_buf Prints error if it occurs
/// @return 0 on success, > 0 otherwise
uint8_t TCMDEXEC_stm32_internal_flash_set_active_flash_bank(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len)
{
    uint64_t desired_active_flash_bank = 0;
    const uint8_t arg_res = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &desired_active_flash_bank);
    if (arg_res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error Parsing Arg 0: %u", arg_res);
        return 1;
    }

    const uint8_t res = STM32_internal_flash_set_active_flash_bank((uint8_t)desired_active_flash_bank);
    if (res != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error changing flash bank: %u", res);
        return 2;
    }
    return 0;
}

/// @brief Prints the active flash bank where the firmware boots from
/// @param response_output_buf Prints the active bank
uint8_t TCMDEXEC_stm32_internal_flash_get_active_flash_bank(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len)
{
    const uint8_t stm32_internal_active_flash_bank = STM32_internal_flash_get_active_flash_bank();

    snprintf(response_output_buf, response_output_buf_len, "Active Bank: %u", stm32_internal_active_flash_bank);
    return 0;
}

/// @brief Telecommand: Write a file to internal flash memory from LittleFS.
/// @param args_str
/// - Arg 0: File name to read from LittleFS
/// - Arg 1: Length to read from the file (in bytes). The maximum length to read is 4096 bytes (4kB).
/// - Arg 2: Offset within the file to start reading (in bytes)
/// - Arg 3: Address in internal flash memory to write to (in hex, 8 characters for 32-bit address)
/// @note To use properly:
///       - The internal flash memory region must be erased before writing.
///       - The address must be in the main flash memory region (0x08000000 to 0x081FFFFF).
uint8_t TCMDEXEC_stm32_internal_flash_write_file_to_internal_flash(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len)
{
    char arg_file_name[LFS_MAX_PATH_LENGTH];
    const uint8_t parse_file_name_result = TCMD_extract_string_arg(args_str, 0, arg_file_name, sizeof(arg_file_name));
    if (parse_file_name_result != 0) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing file name arg: Error %d", parse_file_name_result);
        return 1;
    }

    // Get length of data to read
    uint64_t read_length = 0;
    const uint8_t parse_length_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &read_length);
    if (parse_length_result != 0) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing write length arg: Error %d", parse_length_result);
        return 2;
    }

    // Ensure the read length is not too large
    // Limiting it to 4kB (4096 bytes)
    // because the smallest amount of flash memory we can erase is a page, which is 1kB
    if (read_length > FLASH_PAGE_SIZE) { // 4kB max
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error: Write length too large. Max is %lu bytes.", (unsigned long)FLASH_PAGE_SIZE);
        return 3;
    }
    // Get offset to read from in file
    uint64_t read_offset = 0;
    const uint8_t parse_offset_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 2, &read_offset);
    if (parse_offset_result != 0) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing read offset arg: Error %d", parse_offset_result);
        return 4;
    }

    // Get address to write to (in hex)
    uint8_t write_address_arr[4] = {0}; // 4 bytes for 32-bit address
    uint16_t address_arr_result_len = 0;
    const uint8_t parse_address_result = TCMD_extract_hex_array_arg(
        args_str, 3, write_address_arr, sizeof(write_address_arr), &address_arr_result_len
    );
    if (parse_address_result != 0) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing write address arg: Error %d", parse_address_result);
        return 5;
    }
    const uint32_t write_address = write_address_arr[0] << 24 |
                                   write_address_arr[1] << 16 |
                                   write_address_arr[2] << 8 |
                                   write_address_arr[3]; // Convert to 32-bit address

    if (IS_FLASH_MAIN_MEM_ADDRESS(write_address) == false) {
        // Address is not in the main flash memory region
        snprintf(response_output_buf, response_output_buf_len, "Error: Address 0x%08lX is not in the main flash memory region.", (unsigned long)write_address);
        return 6;
    }

    LOG_message(LOG_SYSTEM_ALL, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
                "TCMDEXEC_fs_write_file_to_internal_flash: Writing %lu bytes from file '%s' to internal flash at address 0x%08lX, read_offset %lu",
                (unsigned long)read_length, arg_file_name, (unsigned long)write_address, (unsigned long)read_offset);

    // Read the file from LittleFS
    uint8_t file_content[read_length];
    memset(file_content, 0, read_length);
    const int32_t bytes_read = LFS_read_file(arg_file_name, read_offset, file_content, read_length);
    if (bytes_read < 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error reading file: %ld", bytes_read);
        return 7;
    }

    // If region is not erased, it will fail
    STM32_internal_flash_write_status_t write_status = {0};
    const STM32_internal_flash_write_return_t write_result = STM32_internal_flash_write(write_address, file_content, read_length, &write_status);
    LOG_message(LOG_SYSTEM_ALL, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
                "Write Status=%d, Lock Status=%d, Unlock Status=%d",
        write_status.write_status, write_status.lock_status, write_status.unlock_status);
    // Handle specific error codes
    switch (write_result)
    {
        case STM32_INTERNAL_FLASH_WRITE_SUCCESS:
            snprintf(response_output_buf,
                     response_output_buf_len,
                     "Successfully wrote %lu (0x%08lX) bytes from file '%s' to internal flash at address 0x%08lX",
                     (uint32_t)read_length, (uint32_t)read_length, arg_file_name, (uint32_t)write_address);
            return 0;
        case STM32_INTERNAL_FLASH_WRITE_ADDRESS_TOO_LOW:
            snprintf(response_output_buf, response_output_buf_len, "Error: Address too low for internal flash write.");
            break;
        case STM32_INTERNAL_FLASH_WRITE_ADDRESS_OVERLAPS_BOTH_FLASH_BANKS:
            snprintf(response_output_buf, response_output_buf_len, "Error: Write address overlaps both flash banks, which is not allowed.");
            break;
        case STM32_INTERNAL_FLASH_WRITE_ADDRESS_TOO_HIGH:
            snprintf(response_output_buf, response_output_buf_len, "Error: Write exceeds internal flash memory bounds.");
            break;
        case STM32_INTERNAL_FLASH_WRITE_UNLOCK_FAILED:
            snprintf(response_output_buf, response_output_buf_len, "Error: Failed to unlock internal flash for writing.");
            break;
        case STM32_INTERNAL_FLASH_WRITE_LOCK_FAILED:
            snprintf(response_output_buf, response_output_buf_len, "Error: Failed to lock internal flash after writing.");
            break;
        case STM32_INTERNAL_FLASH_WRITE_OPERATION_FAILED:
            snprintf(response_output_buf, response_output_buf_len, "Error: Internal flash write operation failed.");
            break;
        default:
            snprintf(response_output_buf, response_output_buf_len, "Error: Unknown error during internal flash write.");
            break;
    }
    // Must be error
    return 8;

}