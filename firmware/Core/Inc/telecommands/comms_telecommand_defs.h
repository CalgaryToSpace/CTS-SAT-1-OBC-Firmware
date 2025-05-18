#ifndef INCLUDE_GUARD__COMMS_TELECOMMAND_DEFS_H
#define INCLUDE_GUARD__COMMS_TELECOMMAND_DEFS_H

#include "telecommand_exec/telecommand_definitions.h"

#include <stdint.h>

uint8_t TCMDEXEC_comms_set_rf_switch_control_mode(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
);


#endif // INCLUDE_GUARD__COMMS_TELECOMMAND_DEFS_H
