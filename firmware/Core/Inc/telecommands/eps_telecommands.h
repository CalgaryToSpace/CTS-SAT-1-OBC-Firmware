#ifndef __INCLUDE_GUARD__EPS_TELECOMMANDS_H__
#define __INCLUDE_GUARD__EPS_TELECOMMANDS_H__

#include <stdint.h>
#include "telecommands/telecommand_definitions.h"


uint8_t TCMDEXEC_eps_watchdog(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_eps_get_system_status_json(const uint8_t *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);


#endif /* __INCLUDE_GUARD__EPS_TELECOMMANDS_H__ */

