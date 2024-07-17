#ifndef __INCLUDE_GUARD__I2C_TELECOMMAND_DEFS_H__
#define __INCLUDE_GUARD__I2C_TELECOMMAND_DEFS_H__

#include <stdint.h>
#include "telecommands/telecommand_definitions.h"



uint8_t TCMDEXEC_scan_i2c_bus(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

#endif /* __INCLUDE_GUARD__I2C_TELECOMMAND_DEFS_H__*/