#ifndef INCLUDE_GUARD__CTS_TELECOMMAND_DEFS_H
#define INCLUDE_GUARD__CTS_TELECOMMAND_DEFS_H

#include <stdint.h>
#include "telecommands/telecommand_types.h"

uint8_t TCMDEXEC_csp_demo_1(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
);


#endif // INCLUDE_GUARD__CTS_TELECOMMAND_DEFS_H
