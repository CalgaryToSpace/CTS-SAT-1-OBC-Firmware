#ifndef __INCLUDE_GUARD__TEMPERATURE_SENSOR_TELECOMMAND_DEFS_H__
#define __INCLUDE_GUARD__TEMPERATURE_SENSOR_TELECOMMAND_DEFS_H__

#include "telecommands/telecommand_definitions.h"


uint8_t TCMDEXEC_obc_read_temperature(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_obc_set_temperature_precision(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);


#endif