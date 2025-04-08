#ifndef INCLUDE_GUARD__I2C_TELECOMMAND_DEFS_H__
#define INCLUDE_GUARD__I2C_TELECOMMAND_DEFS_H__

#include <stdint.h>
#include "telecommand_exec/telecommand_definitions.h"

uint8_t TCMDEXEC_scan_i2c_bus_verbose(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_scan_i2c_bus(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
);

#endif /* INCLUDE_GUARD__I2C_TELECOMMAND_DEFS_H__*/