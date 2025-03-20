#ifndef INCLUDE_GUARD__SYSTEM_TELECOMMAND_DEFINITIONS_H
#define INCLUDE_GUARD__SYSTEM_TELECOMMAND_DEFINITIONS_H

#include <stdint.h>
#include "telecommand_exec/telecommand_types.h"

uint8_t TCMDEXEC_hello_world(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_core_system_stats(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_available_telecommands(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_reboot(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_system_self_check_failures_as_json(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
);

#endif /* INCLUDE_GUARD__SYSTEM_TELECOMMAND_DEFINITIONS_H */
