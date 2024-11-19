#ifndef __INCLUDE_GUARD__GPS_TELECOMMAND_DEFS_H
#define __INCLUDE_GUARD__GPS_TELECOMMAND_DEFS_H

#include "telecommands/telecommand_types.h"

#include <stdint.h>

uint8_t TCMDEXEC_gps_set_enabled(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                 char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_gps_send_cmd_receive_response(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                               char *response_output_buf, uint16_t response_output_buf_len);

#endif // __INCLUDE_GUARD__GPS_TELECOMMAND_DEFS_H