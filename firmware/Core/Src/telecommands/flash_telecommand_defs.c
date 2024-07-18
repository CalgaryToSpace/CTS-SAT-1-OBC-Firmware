
#include "telecommands/telecommand_args_helpers.h"

#include "telecommands/flash_telecommand_defs.h"
#include "littlefs/flash_driver.h"
#include "littlefs/flash_benchmark.h"
#include "log/log.h"
#include "debug_tools/debug_uart.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

/// @brief Telecommand: Read bytes as hex from a flash address
/// @param args_str No args.
/// @return 0 always
uint8_t TCMDEXEC_flash_activate_each_cs(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    const uint16_t delay_time_ms = 500;

    FLASH_deactivate_chip_select();
    HAL_Delay(delay_time_ms);

    for (uint8_t chip_number = 0; chip_number < FLASH_NUMBER_OF_FLASH_DEVICES; chip_number++) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "Activating CS: %" PRIu32 "...", chip_number);
        FLASH_activate_chip_select(chip_number);
        HAL_Delay(delay_time_ms);

        FLASH_deactivate_chip_select();
        HAL_Delay(delay_time_ms);
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "Deactivated.");
    }
    LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "All CS activated and deactivated");

    return 0;
}


/// @brief Telecommand: Read bytes as hex from a flash address
/// @param args_str No args.
/// @return 0 always
uint8_t TCMDEXEC_flash_each_is_reachable(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t fail_count = 0;

    FLASH_deactivate_chip_select();

    for (uint8_t chip_number = 0; chip_number < FLASH_NUMBER_OF_FLASH_DEVICES; chip_number++) {
        const FLASH_error_enum_t result = FLASH_is_reachable(&hspi1, chip_number);
        if (result != 0) {
            fail_count++;

            LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "Chip %d is not reachable. Error code %d", chip_number, result);
        }
        else {
            LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "Chip %d is reachable.", chip_number);
        }
    }

    // Overall status
    if (fail_count == 0) {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "All chips are reachable.");
        return 0;
    }
    else {
        LOG_message(LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_CHANNEL_ALL, "%d/%d chips not reachable.", fail_count, FLASH_NUMBER_OF_FLASH_DEVICES);
        return 1;
    }
}


/// @brief Telecommand: Read bytes as hex from a flash address
/// @param args_str Arg 0: Chip Number (CS number) as uint, Arg 1: Flash Address as uint,
///     Arg 2: Number of bytes to read as uint
/// @return 0 on success, >0 on error // TODO: explain better
uint8_t TCMDEXEC_flash_read_hex(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    const uint16_t max_num_bytes = 256;
    uint64_t chip_num, flash_addr, arg_num_bytes;

    uint8_t arg0_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &chip_num);
    uint8_t arg1_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &flash_addr);
    uint8_t arg2_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 2, &arg_num_bytes);
    
    if (arg0_result != 0 || arg1_result != 0 || arg2_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing arguments. Return codes: arg0=%d, arg1=%d, arg2=%d",
            arg0_result, arg1_result, arg2_result);
        return 1;
    }

    if (chip_num >= FLASH_NUMBER_OF_FLASH_DEVICES) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Chip number is out of range. Must be 0 to %d.",
            FLASH_NUMBER_OF_FLASH_DEVICES - 1);
        return 2;
    }

    if (arg_num_bytes > max_num_bytes || arg_num_bytes == 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Invalid number of bytes to read: %lu. Must be 1 to %d.",
            (uint32_t)arg_num_bytes, max_num_bytes); // TODO: fix this cast
        return 3;
    }

    uint8_t read_buf[max_num_bytes];
    uint32_t num_bytes = (uint32_t)arg_num_bytes;
    FLASH_error_enum_t result = FLASH_read_data(&hspi1, chip_num, flash_addr, read_buf, num_bytes);

    if (result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error reading flash: %d", result);
        return 4;
    }

    // Convert read data to hex
    for (uint16_t i = 0; i < num_bytes; i++) {
        snprintf(
            &response_output_buf[strlen(response_output_buf)],
            response_output_buf_len - strlen(response_output_buf) - 1,
            "%02X ", read_buf[i]);

        // add newline separator every 16 bytes
        if (i > 0 && (i + 1) % 16 == 0) {
            snprintf(
                &response_output_buf[strlen(response_output_buf)],
                response_output_buf_len - strlen(response_output_buf) - 1,
                "\n");
        }
    }
    
    return 0;
}


/// @brief Telecommand: Read bytes as hex from a flash address
/// @param args_str Arg 0: Chip Number (CS number) as uint, Arg 1: Flash Address as uint,
///     Arg 2: Hex string of bytes to write, with no delimiters, in all lowercase
/// @return 0 on success, >0 on error // TODO: explain better
uint8_t TCMDEXEC_flash_write_hex(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    const uint16_t max_num_bytes = 256;
    uint16_t num_bytes;
    uint64_t chip_num, flash_addr_u64;

    char hex_str[max_num_bytes * 2 + 1];
    hex_str[0] = '\0';

    uint8_t bytes_to_write[max_num_bytes];

    uint8_t arg0_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &chip_num);
    uint8_t arg1_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &flash_addr_u64);
    
    // FIXME: actually extract the hex from the string
    // uint8_t arg2_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 2, &num_bytes);
    uint8_t arg2_result = 0; // temporary - fake success

    // FIXME: remove this next temp code, and write a string
    bytes_to_write[0] = '\0';
    strcpy((char*)bytes_to_write, "1234567890abcdef_XYZ");
    num_bytes = strlen((char*)bytes_to_write);
    // end temp code
    
    if (arg0_result != 0 || arg1_result != 0 || arg2_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing arguments. Return codes: arg0=%d, arg1=%d, arg2=%d",
            arg0_result, arg1_result, arg2_result);
        return 1;
    }

    if (chip_num >= FLASH_NUMBER_OF_FLASH_DEVICES) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Chip number is out of range. Must be 0 to %d.",
            FLASH_NUMBER_OF_FLASH_DEVICES - 1);
        return 2;
    }

    if (flash_addr_u64 > FLASH_CHIP_SIZE_BYTES) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Flash address is out of range. Must be 0 to %d.",
            FLASH_CHIP_SIZE_BYTES - 1);
        return 3;
    }
    uint32_t flash_addr = (uint32_t)flash_addr_u64;


    FLASH_error_enum_t result = FLASH_write(&hspi1, chip_num, flash_addr, bytes_to_write, num_bytes);

    if (result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error writing flash: %d", result);
        return 4;
    }

    snprintf(
        response_output_buf, response_output_buf_len,
        "Wrote %d bytes to flash at address %lu on chip %d.",
        num_bytes, flash_addr, (uint8_t)chip_num);
    
    return 0;
}


/// @brief Telecommand: Erase a sector of flash memory.
/// @param args_str
/// - Arg 0: Chip Number (CS number) as uint
/// - Arg 1: Flash Address as uint
/// - Arg 2: Number of bytes to erase as uint
/// @return 0 on success, >0 on error // TODO: explain better
uint8_t TCMDEXEC_flash_erase(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t chip_num, flash_addr;

    uint8_t arg0_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &chip_num);
    uint8_t arg1_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &flash_addr);
    
    if (arg0_result != 0 || arg1_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing arguments. Return codes: arg0=%d, arg1=%d",
            arg0_result, arg1_result);
        return 1;
    }

    if (chip_num >= FLASH_NUMBER_OF_FLASH_DEVICES) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Chip number is out of range. Must be 0 to %d.",
            FLASH_NUMBER_OF_FLASH_DEVICES - 1);
        return 2;
    }

    FLASH_error_enum_t result = FLASH_erase(&hspi1, chip_num, flash_addr);

    if (result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error reading flash: %d", result);
        return 4;
    }
    
    return 0;
}

/// @brief Telecommand: Benchmarks the erase/write/read operations on the flash memory module.
/// @param args_str
/// - Arg 0: Chip Number (CS number) as uint
/// - Arg 1: Test Data Address as uint
/// - Arg 2: Test Data Length as uint
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_flash_benchmark_erase_write_read(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t chip_num, test_data_address, test_data_length;

    uint8_t arg0_result = TCMD_extract_uint64_arg((char*)args_str, strlen((char*)args_str), 0, &chip_num);
    uint8_t arg1_result = TCMD_extract_uint64_arg((char*)args_str, strlen((char*)args_str), 1, &test_data_address);
    uint8_t arg2_result = TCMD_extract_uint64_arg((char*)args_str, strlen((char*)args_str), 2, &test_data_length);
    
    if (arg0_result != 0 || arg1_result != 0 || arg2_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing arguments. Return codes: arg0=%d, arg1=%d, arg2=%d",
            arg0_result, arg1_result, arg2_result);
        return 1;
    }

    uint8_t result = FLASH_benchmark_erase_write_read((uint8_t)chip_num, (uint32_t)test_data_address, (uint32_t)test_data_length, response_output_buf, response_output_buf_len);
    response_output_buf[response_output_buf_len - 1] = '\0'; // ensure null-terminated
    if (result != 0) {
        snprintf(
            &response_output_buf[strlen(response_output_buf)],
            response_output_buf_len - strlen(response_output_buf) - 1,
            "Error benchmarking flash: Returned %d", result);
        return 2;
    }
    
    return result;
}
