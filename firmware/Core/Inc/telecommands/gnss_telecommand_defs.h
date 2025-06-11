#ifndef __INCLUDE_GUARD__GNSS_TELECOMMAND_DEFS_H
#define __INCLUDE_GUARD__GNSS_TELECOMMAND_DEFS_H

#include "telecommand_exec/telecommand_types.h"

#include <stdint.h>

uint8_t TCMDEXEC_gnss_send_cmd_ascii(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

#endif // __INCLUDE_GUARD__GNSS_TELECOMMAND_DEFS_H