#include "debug_tools/debug_i2c.h"
#include "log/log.h"

#include <stdint.h>
#include <inttypes.h>

// TODO telecommand to allow scanning the bus from the ground station

/// @brief Scans an I2C bus for available devices
void DEBUG_i2c_scan(I2C_HandleTypeDef *hi2c) {
    char *bus_name = "UNKNOWN";
    if (hi2c->Instance == I2C1) {
        bus_name = "I2C1";
    }
    else if (hi2c->Instance == I2C2) {
        bus_name = "I2C2";
    }
    else if (hi2c->Instance == I2C2) {
        bus_name = "I2C3";
    }
    else if (hi2c->Instance == I2C2) {
        bus_name = "I2C4";
    }

	LOG_message(LOG_SYSTEM_OBC, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "Starting I2C scan on bus %s", bus_name);

    uint8_t number_of_devices = 0;
    const uint32_t i2c_trials = 3;
    const uint32_t i2c_timeout = 5;
    // Does not look for extended addresses
	for (uint8_t i = 0; i < 128; i++) {
		if (HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(i<<1), i2c_trials, i2c_timeout) == HAL_OK) {
			// We got an ack
            LOG_message(LOG_SYSTEM_OBC, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "%2x: ready", i);
            number_of_devices++;
		}
    }
    LOG_message(LOG_SYSTEM_OBC, LOG_SEVERITY_NORMAL, LOG_CHANNEL_ALL, "Found %d available I2C %s", number_of_devices, number_of_devices == 1 ? "device" : "devices");

    return;
}
