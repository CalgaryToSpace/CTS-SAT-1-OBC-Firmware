
#ifndef __INCLUDE_GUARD__GPS_TELECOMMAND_DEFINITIONS_H
#define __INCLUDE_GUARD__GPS_TELECOMMAND_DEFINITIONS_H

#include "main.h"
#include "telecommands/telecommand_types.h"

uint8_t TCMDEXEC_gps_set_enabled_pps_tracking(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel, 
            char *response_output_buf, uint16_t response_output_buf_len);

#endif // __INCLUDE_GUARD__GPS_TELECOMMAND_DEFINITIONS_H
