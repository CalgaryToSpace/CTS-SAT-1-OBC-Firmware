#ifndef INCLUDE_GUARD__COMMS_TELECOMMAND_DEFS_H
#define INCLUDE_GUARD__COMMS_TELECOMMAND_DEFS_H

#include "telecommand_exec/telecommand_definitions.h"

#include <stdint.h>

uint8_t TCMDEXEC_comms_dipole_switch_set_state(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_comms_bulk_file_downlink_start(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
);
uint8_t TCMDEXEC_comms_bulk_file_downlink_pause(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
);
uint8_t TCMDEXEC_comms_bulk_file_downlink_resume(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
);

#endif // INCLUDE_GUARD__COMMS_TELECOMMAND_DEFS_H
