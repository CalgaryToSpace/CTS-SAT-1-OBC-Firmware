
#include "main.h"
#include "littlefs/flash_benchmark.h"
#include "littlefs/flash_driver.h"

#include <string.h>
#include <stdio.h>


/// @brief Benchmarks the erase/read/write operations on the flash memory module.
/// @param chip_num Chip number to use.
/// @param test_data_address Address to erase, write, and then read back from.
/// @param test_data_length Must be <= 2048 byte (the size of a page) since only one page is read in the benchmark.
/// @param response_str 
/// @param response_str_len 
/// @return 0 on success. 1 if erase failed. 2 if write failed. 3 if read failed. 4 if verify failed.
/// @details This function will erase the flash memory, write test data to it, read it back, and verify the read data.
///       The test data is a sequence of bytes from 0 to 255. The response_str is valid whether or not there are errors.
uint8_t FLASH_benchmark_erase_write_read(uint8_t chip_num, uint32_t test_data_address, uint16_t test_data_length, char* response_str, uint16_t response_str_len) {
    response_str[0] = '\0';

    if (test_data_length > 2048) {
        snprintf(
            &response_str[strlen(response_str)],
            response_str_len - strlen(response_str),
            "Error: test_data_length must be <= 2048 bytes\n");
        return 5;
    }

    FLASH_Physical_Address_t physical_address = {
        .row_address = test_data_address, 
        .col_address = 0};
    // Erase
    const uint32_t erase_start_time = HAL_GetTick();
    const FLASH_error_enum_t erase_result = FLASH_erase_block(chip_num, physical_address);
    if (erase_result != 0) {
        snprintf(
            &response_str[strlen(response_str)],
            response_str_len - strlen(response_str),
            "Erase failed. FLASH_erase() return: %d\n", erase_result);
        return 1;
    }
    const uint32_t erase_end_time = HAL_GetTick();
    snprintf(
        &response_str[strlen(response_str)],
        response_str_len - strlen(response_str),
        "Erase: %lu ms\n", erase_end_time - erase_start_time);

    // Write
    uint8_t write_buffer[test_data_length];
    for (uint32_t i = 0; i < test_data_length; i++) {
        // Add 42 below, because errors are likely to happen around powers of 2,
        // so this avoids having a 0/255 right on a power of 2 boundary.
        write_buffer[i] = (i + 42) % 256;
    }
    // TODO: for very large writes, split into multiple writes (instead of allocating the whole amount on the stack)
    const uint32_t write_send_start_time = HAL_GetTick();
    const FLASH_error_enum_t write_result = FLASH_program_page(chip_num, physical_address, write_buffer, test_data_length);
    if (write_result != 0) {
        snprintf(
            &response_str[strlen(response_str)],
            response_str_len - strlen(response_str),
            "Write failed. FLASH_write return: %d\n", write_result);
        return 2;
    }
    const uint32_t write_send_end_time = HAL_GetTick();
    snprintf(
        &response_str[strlen(response_str)],
        response_str_len - strlen(response_str),
        "Write: %lu ms\n",
        write_send_end_time - write_send_start_time);

    // Read
    const uint32_t read_start_time = HAL_GetTick();
    uint8_t read_buffer[test_data_length];
    const FLASH_error_enum_t read_result = FLASH_read_page(chip_num, physical_address, read_buffer, test_data_length);
    if (read_result != 0) {
        snprintf(
            &response_str[strlen(response_str)],
            response_str_len - strlen(response_str),
            "Read failed. FLASH_read_data return: %d\n", read_result);
        return 3;
    }
    const uint32_t read_end_time = HAL_GetTick();
    snprintf(
        &response_str[strlen(response_str)],
        response_str_len - strlen(response_str),
        "Read: %lu ms\n", read_end_time - read_start_time);

    // Verify Read
    for (uint32_t i = 0; i < test_data_length; i++) {
        if (read_buffer[i] != write_buffer[i]) {
            snprintf(
                &response_str[strlen(response_str)],
                response_str_len - strlen(response_str),
                "Verify failed at index %lu. Expected %u, got %u.\n", i, write_buffer[i], read_buffer[i]);
            return 4;
        }
    }

    return 0;
}
