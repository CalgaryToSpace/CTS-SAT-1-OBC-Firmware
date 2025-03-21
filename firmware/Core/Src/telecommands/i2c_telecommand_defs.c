#include "telecommands/i2c_telecommand_defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

const uint32_t I2C_scan_number_of_trials = 3;
const uint32_t I2C_scan_timeout_ms = 5;

/// @brief Scans the I2C bus for devices. Prints out a grid of all devices, with addresses for those found.
/// @param args_str
/// - Arg 0: I2C bus to scan (1-4)
/// @return 0 if successful, 1 if error.
uint8_t TCMDEXEC_scan_i2c_bus_verbose(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    I2C_HandleTypeDef* hi2c;
    const uint8_t bus_to_scan = atoi(args_str);

    // Figure out which bus we want to scan
    switch(bus_to_scan) {
        case 1:
            hi2c = &hi2c1;
            break;
        case 2:
            hi2c = &hi2c2;
            break;
        case 3: 
            hi2c = &hi2c3;
            break;
        case 4:
            hi2c = &hi2c4;
            break;
        default:
            hi2c = NULL;
            snprintf(response_output_buf, response_output_buf_len, "Enter a valid I2C bus!\n");
            return 1;
    }

    char msg[16];
    uint8_t count_success = 0;
    uint8_t count_error = 0;
    uint8_t count_timeout = 0;
    uint8_t count_busy = 0;
    uint8_t count_misc = 0;

    // Go through all possible i2c addresses
    for (uint16_t i = 0; i < 128; i++) {
        const HAL_StatusTypeDef i2c_device_status = HAL_I2C_IsDeviceReady(hi2c, (i<<1), I2C_scan_number_of_trials, I2C_scan_timeout_ms);

        // Calculate remaining space in buffer. -1 for null terminator
        const size_t remaining_space = response_output_buf_len - strlen(response_output_buf) - 1;

        if (i2c_device_status == HAL_OK) {
            // O for OK
            snprintf(msg, sizeof(msg), " 0x%02x ", i);
            count_success++;
        } 
        else if (i2c_device_status == HAL_ERROR) {
            // E for Error
            snprintf(msg, sizeof(msg), "  _E_ ");
            count_error++;
        }
        else if (i2c_device_status == HAL_TIMEOUT) {
            // T for timeout
            snprintf(msg, sizeof(msg), "  _T_ ");
            count_timeout++;
        }
        else if (i2c_device_status == HAL_BUSY) {
            // B for busy
            snprintf(msg, sizeof(msg), "  _B_ ");
            count_busy++;
        }
        else {
            // M for Misc. error
            snprintf(msg, sizeof(msg), "  _M_ ");
            count_misc++;
        }

        // Add newline every 16 addresses.
        if ((i + 1) % 16 == 0 && i > 0) {
            snprintf(&msg[strlen(msg)], sizeof(msg) - strlen(msg), "\n");
        }
        strncat(response_output_buf, msg, remaining_space);
    }

    // Add counts to the end of the response.
    snprintf(
        &response_output_buf[strlen(response_output_buf)],
        response_output_buf_len - strlen(response_output_buf),
        "\nSuccess: %d, Error: %d, Timeout: %d, Busy: %d, Misc: %d\n",
        count_success, count_error, count_timeout, count_busy, count_misc
    );
    
    return 0;
}

/// @brief Scans the I2C bus for devices. Prints out the addresses of devices found.
/// @param args_str
/// - Arg 0: I2C bus to scan (1-4)
/// @return 0 if successful, 1 if error.
uint8_t TCMDEXEC_scan_i2c_bus(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    I2C_HandleTypeDef* hi2c;
    const uint8_t bus_to_scan = atoi(args_str);

    switch(bus_to_scan) {
        case 1: hi2c = &hi2c1; break;
        case 2: hi2c = &hi2c2; break;
        case 3: hi2c = &hi2c3; break;
        case 4: hi2c = &hi2c4; break;
        default:
            snprintf(response_output_buf, response_output_buf_len, "Enter a valid I2C bus!\n");
            return 1;
    }

    uint8_t state_counts[5] = {0}; // [OK, ERROR, BUSY, TIMEOUT, MISC]
    HAL_StatusTypeDef address_states[128];

    // Scan the bus
    for (uint16_t i = 0; i < 128; i++) {
        address_states[i] = HAL_I2C_IsDeviceReady(hi2c, (i<<1), I2C_scan_number_of_trials, I2C_scan_timeout_ms);
        switch (address_states[i]) {
            case HAL_OK: state_counts[0]++; break;
            case HAL_ERROR: state_counts[1]++; break;
            case HAL_BUSY: state_counts[2]++; break;
            case HAL_TIMEOUT: state_counts[3]++; break;
            default: state_counts[4]++; break;
        }
    }

    // Determine majority state
    uint8_t majority_state = 0;
    uint8_t max_count = state_counts[0];
    for (uint8_t j = 1; j < 5; j++) {
        if (state_counts[j] > max_count) {
            max_count = state_counts[j];
            majority_state = j;
        }
    }

    snprintf(
        response_output_buf, response_output_buf_len,
        "{\"majority_state\":\"%s\"",
        majority_state == 0 ? "OK" : 
        majority_state == 1 ? "ERROR" : 
        majority_state == 2 ? "BUSY" : 
        majority_state == 3 ? "TIMEOUT" : "MISC"
    );
    
    size_t buf_len = strlen(response_output_buf);
    
    for (uint16_t i = 0; i < 128; i++) {
        if (address_states[i] != majority_state) {
            const size_t remaining_space = response_output_buf_len - buf_len - 1;
            if (remaining_space <= 0) break;
    
            const char *state_str = 
                address_states[i] == HAL_OK ? "OK" :
                address_states[i] == HAL_ERROR ? "ERROR" :
                address_states[i] == HAL_BUSY ? "BUSY" :
                address_states[i] == HAL_TIMEOUT ? "TIMEOUT" : "MISC";
    
            char msg[64];
            snprintf(msg, sizeof(msg), ",\"0x%02X\":\"%s\"", i, state_str);
            strncat(response_output_buf, msg, remaining_space);
            buf_len = strlen(response_output_buf);
        }
    }
    
    const size_t remaining_space = response_output_buf_len - buf_len - 1;
    if (remaining_space > 0) {
        strncat(response_output_buf, "}", remaining_space);
    }    
    return 0;
}
