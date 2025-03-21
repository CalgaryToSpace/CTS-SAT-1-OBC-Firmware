#ifndef INCLUDE_GUARD_ANT_INTERNAL_DRIVERS_H__
#define INCLUDE_GUARD_ANT_INTERNAL_DRIVERS_H__

#include <stdint.h>

#define ANT_DEFAULT_RX_LEN_MIN 5 // for commands with no response params, 5 bytes are returned

static const uint16_t ANT_ADDR_A = 0x31; // I2C address of mcu A on ant deploy system
static const uint16_t ANT_ADDR_B = 0x32; // I2C address of mcu B on ant deploy system

extern uint64_t last_response_ms;

/// @brief the antenna deployment module has two different i2c connections and two different
/// microcontrollers. The values of this enum represent a combination of the i2c line to
/// use in transmission and the microcontroller to transmit to.
enum ANT_i2c_bus_mcu {
    ANT_I2C_BUS_A_MCU_A,
    ANT_I2C_BUS_B_MCU_B
};
uint8_t ANT_send_cmd(enum ANT_i2c_bus_mcu i2c_bus_mcu, uint8_t cmd_buf[], uint8_t cmd_len);
uint8_t ANT_get_response(enum ANT_i2c_bus_mcu i2c_bus_mcu, uint8_t rx_buf[], uint16_t rx_len);

#endif /* INCLUDE_GUARD_ANT_INTERNAL_DRIVERS_H__ */