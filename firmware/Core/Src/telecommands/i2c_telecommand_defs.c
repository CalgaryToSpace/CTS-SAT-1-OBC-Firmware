#include "i2c_telecommand_defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

uint8_t TCMDEXEC_scan_i2c_bus(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    I2C_HandleTypeDef* hi2c;
    uint8_t bus_to_scan = atoi(args_str);

    // Figure out which bus we want to scan
    switch(bus_to_scan){
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
    HAL_StatusTypeDef i2c_device_status;

	// Go through all possible i2c addresses
	for (uint8_t i = 0; i < 128; i++) {

        i2c_device_status = HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(i<<1), 3, 5);

        // calculate remaining space in buffer. -1 for null terminator
        size_t remaining_space = response_output_buf_len - strlen(response_output_buf) - 1;

		if (i2c_device_status == HAL_OK) {
			// O for OK
            snprintf(msg, sizeof(msg), " 0x%02x ", i);
		} 
        else if (i2c_device_status == HAL_ERROR) {
            // E for Error
            snprintf(msg, sizeof(msg), "  _E_ ");
        }
        else if (i2c_device_status == HAL_TIMEOUT) {
            // T for timeout
            snprintf(msg, sizeof(msg), "  _T_ ");
		}
        else if (i2c_device_status == HAL_BUSY) {
            // B for busy
            snprintf(msg, sizeof(msg), "  _B_ ");
		}
        else {
            // M for Misc. error
            snprintf(msg, sizeof(msg), "  _M_ ");
        }

        // Add newline ever 16 addresses
        if ((i + 1) % 16 == 0 && i > 0) {
            snprintf(&msg[strlen(msg)], sizeof(msg) - strlen(msg), "\n");
        }
        strncat(response_output_buf, msg, remaining_space);
	}
    return 0;
}