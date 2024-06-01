
#include "telecommands/telecommand_args_helpers.h"

#include "telecommands/flash_telecommand_defs.h"
#include "littlefs/flash_driver.h"
#include "debug_tools/debug_uart.h"


/// @brief Telecommand: Read bytes as hex from a flash address
/// @param args_str No args.
/// @return 0 always
uint8_t TCMDEXEC_flash_activate_each_cs(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    const uint16_t delay_time_ms = 500;

    FLASH_deactivate_chip_select();
    HAL_Delay(delay_time_ms);

    for (uint8_t chip_number = 0; chip_number < FLASH_NUMBER_OF_FLASH_DEVICES; chip_number++) {
        debug_uart_print_str("Activating CS: ");
        debug_uart_print_uint32(chip_number);
        debug_uart_print_str("...");
        FLASH_activate_chip_select(chip_number);
        HAL_Delay(delay_time_ms);

        debug_uart_print_str("Deactivated.\n");
        FLASH_deactivate_chip_select();
        HAL_Delay(delay_time_ms);
    }
    strcpy(response_output_buf, "All CS activated and deactivated.\n");
    return 0;
}


/// @brief Telecommand: Read bytes as hex from a flash address
/// @param args_str Arg 0: Chip Number (CS number) as uint, Arg 1: Flash Address as uint,
///     Arg 2: Number of bytes to read as uint
/// @return 0 on success, >0 on error // TODO
uint8_t TCMDEXEC_flash_read_as_hex(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    const uint16_t max_num_bytes = 256;
    uint64_t chip_num, flash_addr, num_bytes;

    uint8_t arg0_result = TCMD_extract_uint64_arg((char*)args_str, strlen((char*)args_str), 0, &chip_num);
    uint8_t arg1_result = TCMD_extract_uint64_arg((char*)args_str, strlen((char*)args_str), 1, &flash_addr);
    uint8_t arg2_result = TCMD_extract_uint64_arg((char*)args_str, strlen((char*)args_str), 2, &num_bytes);
    
    if (arg0_result != 0 || arg1_result != 0 || arg2_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing arguments: arg0=%d, arg1=%d, arg2=%d",
            arg0_result, arg1_result, arg2_result);
        return 1;
    }

    if (chip_num >= FLASH_NUMBER_OF_FLASH_DEVICES) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Chip number %llu is out of range. Must be 0 to %d.",
            chip_num, FLASH_NUMBER_OF_FLASH_DEVICES - 1);
        return 2;
    }

    if (num_bytes > max_num_bytes || num_bytes == 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Invalid number of bytes to read: %llu. Must be 1 to %d.",
            num_bytes, max_num_bytes);
        return 3;
    }

    uint8_t read_buf[max_num_bytes];
    uint8_t result = FLASH_read_data(&hspi1, chip_num, flash_addr, read_buf, num_bytes);

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

        // add newline separator every 8 bytes
        if (i > 0 && i % 8 == 0) {
            snprintf(
                &response_output_buf[strlen(response_output_buf)],
                response_output_buf_len - strlen(response_output_buf) - 1,
                "\n");
        }
    }
    
    return 0;
}
