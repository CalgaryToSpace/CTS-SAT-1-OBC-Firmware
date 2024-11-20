
#include "telecommands/telecommand_args_helpers.h"

#include "telecommands/flash_telecommand_defs.h"
#include "littlefs/flash_driver.h"
#include "littlefs/flash_benchmark.h"
#include "debug_tools/debug_uart.h"
#include "log/log.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

uint8_t read_buf[FLASH_MAX_BYTES_PER_PAGE];
uint8_t bytes_to_write[FLASH_MAX_BYTES_PER_PAGE];

/// @brief Telecommand: Read bytes as hex from a flash address
/// @param args_str No args.
/// @return 0 always
uint8_t TCMDEXEC_flash_activate_each_cs(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                        char *response_output_buf, uint16_t response_output_buf_len)
{
    const uint16_t delay_time_ms = 500;

    FLASH_deactivate_chip_select();
    HAL_Delay(delay_time_ms);

    for (uint8_t chip_number = 0; chip_number < FLASH_NUMBER_OF_FLASH_DEVICES; chip_number++)
    {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Activating CS: ");
        DEBUG_uart_print_uint32(chip_number);
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "...\n");
        FLASH_activate_chip_select(chip_number);
        HAL_Delay(delay_time_ms);

        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Deactivated.\n");
        FLASH_deactivate_chip_select();
        HAL_Delay(delay_time_ms);
    }
    strcpy(response_output_buf, "All CS activated and deactivated.\n");
    return 0;
}

/// @brief Telecommand: Read bytes as hex from a flash address
/// @param args_str No args.
/// @return 0 always
uint8_t TCMDEXEC_flash_each_is_reachable(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                         char *response_output_buf, uint16_t response_output_buf_len)
{
    uint8_t fail_count = 0;

    FLASH_deactivate_chip_select();

    for (uint8_t chip_number = 0; chip_number < FLASH_NUMBER_OF_FLASH_DEVICES; chip_number++)
    {
        const FLASH_error_enum_t result = FLASH_is_reachable(&hspi1, chip_number);
        if (result != 0)
        {
            fail_count++;

            // append to response buffer
            snprintf(
                &response_output_buf[strlen(response_output_buf)],
                response_output_buf_len - strlen(response_output_buf) - 1,
                "Chip %d is not reachable. Error code: %d\n",
                chip_number, result);
        }
        else
        {
            // append to response buffer
            snprintf(
                &response_output_buf[strlen(response_output_buf)],
                response_output_buf_len - strlen(response_output_buf) - 1,
                "Chip %d is reachable.\n",
                chip_number);
        }
    }

    // append overall status
    if (fail_count == 0)
    {
        snprintf(
            &response_output_buf[strlen(response_output_buf)],
            response_output_buf_len - strlen(response_output_buf) - 1,
            "All chips are reachable.");
        return 0;
    }
    else
    {
        snprintf(
            &response_output_buf[strlen(response_output_buf)],
            response_output_buf_len - strlen(response_output_buf) - 1,
            "%d/%d chips are not reachable.",
            fail_count, FLASH_NUMBER_OF_FLASH_DEVICES);
        return 1;
    }
}

/// @brief Telecommand: Read bytes as hex from a page number
/// @param args_str
/// - Arg 0: Chip Number (CS number) as uint
/// - Arg 1: Page number as uint
/// - Arg 2: Number of bytes to read as uint
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_flash_read_hex(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                char *response_output_buf, uint16_t response_output_buf_len)
{
    uint64_t chip_num_u64, page_num_u64, num_bytes_u64;

    uint8_t arg0_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &chip_num_u64);
    uint8_t arg1_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &page_num_u64);
    uint8_t arg2_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 2, &num_bytes_u64);

    if (arg0_result != 0 || arg1_result != 0 || arg2_result != 0)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing arguments. Return codes: arg0=%d, arg1=%d, arg2=%d",
            arg0_result, arg1_result, arg2_result);
        return 1;
    }

    if (chip_num_u64 >= FLASH_NUMBER_OF_FLASH_DEVICES)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Chip number is out of range. Must be 0 to %d.",
            FLASH_NUMBER_OF_FLASH_DEVICES - 1);
        return 2;
    }

    if (page_num_u64 > (FLASH_CHIP_SIZE_BYTES / FLASH_MAX_BYTES_PER_PAGE) - 1)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Page number is out of range. Must be 0 to %d.",
            (FLASH_CHIP_SIZE_BYTES / FLASH_MAX_BYTES_PER_PAGE) - 1);
        return 3;
    }

    if (num_bytes_u64 > FLASH_MAX_BYTES_PER_PAGE || num_bytes_u64 == 0)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Invalid number of bytes to read: %lu. Must be 1 to %d.",
            (uint32_t)num_bytes_u64, FLASH_MAX_BYTES_PER_PAGE); // TODO: fix this cast
        return 3;
    }

    const uint8_t chip_num = (uint8_t)chip_num_u64;
    const uint32_t page_num = (uint32_t)page_num_u64;
    const uint32_t num_bytes = (uint32_t)num_bytes_u64;
    const FLASH_error_enum_t read_result = FLASH_read_data(&hspi1, chip_num, page_num, read_buf, num_bytes);

    if (read_result != 0)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error reading flash: %d", read_result);
        return 4;
    }

    // Convert read data to hex
    // FIXME: This can't print whole page of data (2048 bytes). Fix so that it can.
    for (uint16_t i = 0; i < num_bytes; i++)
    {
        snprintf(
            &response_output_buf[strlen(response_output_buf)],
            response_output_buf_len - strlen(response_output_buf) - 1,
            "%02X ", read_buf[i]);

        // add newline separator every 16 bytes
        if (i > 0 && (i + 1) % 16 == 0)
        {
            snprintf(
                &response_output_buf[strlen(response_output_buf)],
                response_output_buf_len - strlen(response_output_buf) - 1,
                "\n");
        }
    }

    return 0;
}

/// @brief Telecommand: Write a hex string of bytes to a page number
/// @param args_str
/// - Arg 0: Chip Number (CS number) as uint
/// - Arg 1: Page number as uint
/// - Arg 2: Hex string of bytes to write (any case, allows space/underscore separators)
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_flash_write_hex(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                 char *response_output_buf, uint16_t response_output_buf_len)
{
    uint16_t num_bytes;
    uint64_t chip_num_u64, page_num_u64;

    const uint8_t arg0_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &chip_num_u64);
    const uint8_t arg1_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &page_num_u64);
    const uint8_t arg2_result = TCMD_extract_hex_array_arg(
        args_str, 2, bytes_to_write, FLASH_MAX_BYTES_PER_PAGE, &num_bytes);

    if (arg0_result != 0 || arg1_result != 0 || arg2_result != 0)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing arguments. Return codes: arg0=%d, arg1=%d, arg2=%d",
            arg0_result, arg1_result, arg2_result);
        return 1;
    }

    if (chip_num_u64 >= FLASH_NUMBER_OF_FLASH_DEVICES)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Chip number is out of range. Must be 0 to %d.",
            FLASH_NUMBER_OF_FLASH_DEVICES - 1);
        return 2;
    }

    if (page_num_u64 > (FLASH_CHIP_SIZE_BYTES / FLASH_MAX_BYTES_PER_PAGE) - 1)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Page number is out of range. Must be 0 to %d.",
            (FLASH_CHIP_SIZE_BYTES / FLASH_MAX_BYTES_PER_PAGE) - 1);
        return 3;
    }

    const uint8_t chip_num = (uint8_t)chip_num_u64;
    const uint32_t page_num = (uint32_t)page_num_u64;
    const FLASH_error_enum_t write_result = FLASH_write_data(&hspi1, chip_num, page_num, bytes_to_write, num_bytes);

    if (write_result != 0)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error writing flash: %d", write_result);
        return 4;
    }

    snprintf(
        response_output_buf, response_output_buf_len,
        "Wrote %d bytes to flash at page number %lu on chip %d.",
        num_bytes, page_num, (uint8_t)chip_num);

    return 0;
}

/// @brief Telecommand: Erase a block of flash memory containing the given page number.
/// @param args_str
/// - Arg 0: Chip Number (CS number) as uint
/// - Arg 1: Page number as uint
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_flash_erase(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                             char *response_output_buf, uint16_t response_output_buf_len)
{
    uint64_t chip_num_u64, page_num_u64;

    const uint8_t arg0_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &chip_num_u64);
    const uint8_t arg1_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &page_num_u64);

    if (arg0_result != 0 || arg1_result != 0)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing arguments. Return codes: arg0=%d, arg1=%d",
            arg0_result, arg1_result);
        return 1;
    }

    if (chip_num_u64 >= FLASH_NUMBER_OF_FLASH_DEVICES)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Chip number is out of range. Must be 0 to %d.",
            FLASH_NUMBER_OF_FLASH_DEVICES - 1);
        return 2;
    }

    if (page_num_u64 > (FLASH_CHIP_SIZE_BYTES / FLASH_MAX_BYTES_PER_PAGE) - 1)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Page number is out of range. Must be 0 to %d.",
            (FLASH_CHIP_SIZE_BYTES / FLASH_MAX_BYTES_PER_PAGE) - 1);
        return 3;
    }

    const uint8_t chip_num = (uint8_t)chip_num_u64;
    const uint32_t page_num = (uint32_t)page_num_u64;
    const FLASH_error_enum_t erase_result = FLASH_erase(&hspi1, chip_num, page_num);

    if (erase_result != 0)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error reading flash: %d", erase_result);
        return 4;
    }

    // success
    snprintf(
        &response_output_buf[strlen(response_output_buf)],
        response_output_buf_len - strlen(response_output_buf) - 1,
        "Successfully erased block containing page number %lu on chip %d.\n",
        page_num, chip_num);

    return 0;
}

/// @brief Telecommand: Benchmarks the erase/write/read operations on the flash memory module.
/// @param args_str
/// - Arg 0: Chip Number (CS number) as uint
/// - Arg 1: Test Data Address as uint
/// - Arg 2: Test Data Length as uint
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_flash_benchmark_erase_write_read(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                  char *response_output_buf, uint16_t response_output_buf_len)
{
    uint64_t chip_num, test_data_address, test_data_length;

    uint8_t arg0_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &chip_num);
    uint8_t arg1_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &test_data_address);
    uint8_t arg2_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 2, &test_data_length);

    if (arg0_result != 0 || arg1_result != 0 || arg2_result != 0)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing arguments. Return codes: arg0=%d, arg1=%d, arg2=%d",
            arg0_result, arg1_result, arg2_result);
        return 1;
    }

    uint8_t result = FLASH_benchmark_erase_write_read((uint8_t)chip_num, (uint32_t)test_data_address, (uint32_t)test_data_length, response_output_buf, response_output_buf_len);
    response_output_buf[response_output_buf_len - 1] = '\0'; // ensure null-terminated
    if (result != 0)
    {
        snprintf(
            &response_output_buf[strlen(response_output_buf)],
            response_output_buf_len - strlen(response_output_buf) - 1,
            "Error benchmarking flash: Returned %d", result);
        return 2;
    }

    return result;
}

/// @brief Telecommand: Reset the flash memory module.
/// @param args_str
/// - Arg 0: Chip Number (CS number) as uint
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_flash_reset(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                             char *response_output_buf, uint16_t response_output_buf_len)
{
    uint64_t chip_num_u64;

    const uint8_t arg0_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &chip_num_u64);

    if (arg0_result != 0)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing chip number argument: %d", arg0_result);
        return 1;
    }

    if (chip_num_u64 >= FLASH_NUMBER_OF_FLASH_DEVICES)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Chip number is out of range. Must be 0 to %d.",
            FLASH_NUMBER_OF_FLASH_DEVICES - 1);
        return 2;
    }

    const uint8_t chip_num = (uint8_t)chip_num_u64;
    const uint8_t comms_err = FLASH_reset(&hspi1, chip_num);
    if (comms_err != 0)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error resetting flash chip: %d", comms_err);
        return 2;
    }

    // success
    snprintf(
        &response_output_buf[strlen(response_output_buf)],
        response_output_buf_len - strlen(response_output_buf) - 1,
        " Successfully reset chip %d.\n",
        (uint8_t)chip_num);

    return 0;
}

/// @brief Telecommand: Read and print Status Register value as hex from the flash memory module.
/// @param args_str
/// - Arg 0: Chip Number (CS number) as uint
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_flash_read_status_register(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len)
{
    uint64_t chip_num_u64;

    const uint8_t arg0_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &chip_num_u64);

    if (arg0_result != 0)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing chip number argument: %d", arg0_result);
        return 1;
    }

    if (chip_num_u64 >= FLASH_NUMBER_OF_FLASH_DEVICES)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Chip number is out of range. Must be 0 to %d.",
            FLASH_NUMBER_OF_FLASH_DEVICES - 1);
        return 2;
    }

    const uint8_t chip_num = (uint8_t)chip_num_u64;
    uint8_t status_reg_val;
    const FLASH_error_enum_t comms_err = FLASH_read_status_register(&hspi1, chip_num, &status_reg_val);
    if (comms_err != 0)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error reading status register: %d", comms_err);
        return 2;
    }

    // success
    snprintf(
        &response_output_buf[strlen(response_output_buf)],
        response_output_buf_len - strlen(response_output_buf) - 1,
        " Status Register Value: 0x%02X\n",
        status_reg_val);

    return 0;
}

/// @brief Telecommand: Set the write enable lath to high on the flash memory module
/// @param args_str
/// - Arg 0: Chip Number (CS number) as uint
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_flash_write_enable(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                    char *response_output_buf, uint16_t response_output_buf_len)
{
    uint64_t chip_num_u64;

    const uint8_t arg0_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &chip_num_u64);
    if (arg0_result != 0)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing chip number argument: %d", arg0_result);
        return 1;
    }

    if (chip_num_u64 >= FLASH_NUMBER_OF_FLASH_DEVICES)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Chip number is out of range. Must be 0 to %d.",
            FLASH_NUMBER_OF_FLASH_DEVICES - 1);
        return 2;
    }

    uint8_t chip_num = (uint8_t)chip_num_u64;
    const FLASH_error_enum_t comms_err = FLASH_write_enable(&hspi1, chip_num);
    if (comms_err != 0)
    {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error enabling write latch: %d", comms_err);
        return 2;
    }

    return 0;
}