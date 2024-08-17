#ifndef INCLUDE_GUARD__EPS_TELECOMMANDS_H__
#define INCLUDE_GUARD__EPS_TELECOMMANDS_H__

#include <stdint.h>
#include "telecommands/telecommand_definitions.h"


uint8_t TCMDEXEC_eps_watchdog(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_eps_system_reset(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_eps_no_operation(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_eps_cancel_operation(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_eps_switch_to_mode(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_eps_set_channel_enabled(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_eps_get_system_status_json(
    const char *args_str,
    TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
);


#endif /* INCLUDE_GUARD__EPS_TELECOMMANDS_H__ */

