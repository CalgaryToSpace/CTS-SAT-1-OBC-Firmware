
#include "main.h"
#include "debug_tools/debug_i2c.h"
#include "debug_tools/debug_uart.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>


void DEBUG_i2c_scan(I2C_HandleTypeDef *hi2c) {
	DEBUG_uart_print_str("Starting I2C scan...\n");

	char msg[5];
	// Go through all possible i2c addresses
	for (uint8_t i = 0; i < 128; i++) {

		if (HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(i<<1), 3, 5) == HAL_OK) {
			// We got an ack
			sprintf(msg, "%2x ", i);
			DEBUG_uart_print_str(msg);
		} else {
			DEBUG_uart_print_str("-- ");
		}

	if (i > 0 && (i + 1) % 16 == 0) DEBUG_uart_print_str("\n");

	}

	DEBUG_uart_print_str("\n");
}
