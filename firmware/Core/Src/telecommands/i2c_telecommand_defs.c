#include "i2c_telecommand_defs.h"

#include <stdio.h>
#include <stdlib.h>

uint8_t TCMDEXEC_scan_i2c_device(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    snprintf(response_output_buf, response_output_buf_len, "Starting I2C scan...\n");

    I2C_HandleTypeDef* hi2c;
    int bus_to_scan = atoi(args_str);

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
            return 0;
    }


	char msg[5];
	// Go through all possible i2c addresses
	for (uint8_t i = 0; i < 128; i++) {

		if (HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(i<<1), 3, 5) == HAL_OK) {
			// We got an ack
			sprintf(msg, "%2x ", i);
			snprintf(response_output_buf, response_output_buf_len, "%s", msg);
		} else {
			snprintf(response_output_buf, response_output_buf_len, "-- ");
		}

	    if (i > 0 && (i + 1) % 16 == 0) snprintf(response_output_buf, response_output_buf_len, "\n");

	}

    return 0;
}