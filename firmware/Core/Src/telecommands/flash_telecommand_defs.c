
#include "telecommand_exec/telecommand_args_helpers.h"

#include "telecommands/flash_telecommand_defs.h"
#include "littlefs/flash_driver.h"
#include "littlefs/flash_benchmark.h"
#include "debug_tools/debug_uart.h"
#include "log/log.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

uint8_t read_buf[FLASH_CHIP_PAGE_SIZE_BYTES];
uint8_t bytes_to_write[FLASH_CHIP_PAGE_SIZE_BYTES];



/// @brief Telecommand: Read bytes as hex from a flash address
/// @param args_str No args.
/// @return 0 always
uint8_t TCMDEXEC_flash_each_is_reachable(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint8_t fail_count = 0;


    for (uint8_t chip_number = 0; chip_number < FLASH_NUMBER_OF_FLASH_DEVICES; chip_number++) {
        const FLASH_error_enum_t result = FLASH_is_reachable(chip_number);
        if (result != 0) {
            fail_count++;

            // append to response buffer
            snprintf(
                &response_output_buf[strlen(response_output_buf)],
                response_output_buf_len - strlen(response_output_buf) - 1,
                "Chip %d is not reachable. Error code: %d\n",
                chip_number, result);
        }
        else {
            // append to response buffer
            snprintf(
                &response_output_buf[strlen(response_output_buf)],
                response_output_buf_len - strlen(response_output_buf) - 1,
                "Chip %d is reachable.\n",
                chip_number);
        }
    }

    // append overall status
    if (fail_count == 0) {
        snprintf(
            &response_output_buf[strlen(response_output_buf)],
            response_output_buf_len - strlen(response_output_buf) - 1,
            "All chips are reachable.");
        return 0;
    }
    else {
        snprintf(
            &response_output_buf[strlen(response_output_buf)],
            response_output_buf_len - strlen(response_output_buf) - 1,
            "%d/%d chips are not reachable.",
            fail_count, FLASH_NUMBER_OF_FLASH_DEVICES);
        return 1;
    }
}



/// @brief Telecommand: Reset the flash memory module.
/// @param args_str 
/// - Arg 0: Chip Number (CS number) as uint
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_flash_reset(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t chip_num_u64;

    const uint8_t arg0_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &chip_num_u64);

    if (arg0_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing chip number argument: %d", arg0_result);
        return 1;
    }

    if (chip_num_u64 >= FLASH_NUMBER_OF_FLASH_DEVICES) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Chip number is out of range. Must be 0 to %d.",
            FLASH_NUMBER_OF_FLASH_DEVICES - 1);
        return 2;
    }

    const uint8_t chip_num = (uint8_t)chip_num_u64;
    const uint8_t comms_err = FLASH_reset(chip_num);
    if (comms_err != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error resetting flash chip: %d",comms_err);
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
// uint8_t TCMDEXEC_flash_read_status_register(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
//                         char *response_output_buf, uint16_t response_output_buf_len) {
//     uint64_t chip_num_u64;

//     const uint8_t arg0_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &chip_num_u64);

//     if (arg0_result != 0) {
//         snprintf(
//             response_output_buf, response_output_buf_len,
//             "Error parsing chip number argument: %d", arg0_result);
//         return 1;
//     }

//     if (chip_num_u64 >= FLASH_NUMBER_OF_FLASH_DEVICES) {
//         snprintf(
//             response_output_buf, response_output_buf_len,
//             "Chip number is out of range. Must be 0 to %d.",
//             FLASH_NUMBER_OF_FLASH_DEVICES - 1);
//         return 2;
//     }

//     const uint8_t chip_num = (uint8_t)chip_num_u64;
//     uint8_t status_reg_val;
//     const FLASH_error_enum_t comms_err = FLASH_read_status_register(chip_num, &status_reg_val);
//     if (comms_err != 0) {
//         snprintf(
//             response_output_buf, response_output_buf_len,
//             "Error reading status register: %d",comms_err);
//         return 2;
//     }

//     // success
//     snprintf(
//         &response_output_buf[strlen(response_output_buf)],
//         response_output_buf_len - strlen(response_output_buf) - 1,
//         " Status Register Value: 0x%02X\n", 
//         status_reg_val);

//     return 0;
// }
