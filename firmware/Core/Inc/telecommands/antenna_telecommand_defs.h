#ifndef __INCLUDE_GUARD_ANTENNA_TELECOMMAND_DEFS_H__
#define __INCLUDE_GUARD_ANTENNA_TELECOMMAND_DEFS_H__

#include <stdint.h>
#include "telecommands/telecommand_definitions.h"

uint8_t TCMDEXEC_ant_arm_antenna_system(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ant_deploy_antenna1(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ant_measure_temp(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

#endif /* __INCLUDE_GUARD_ANTENNA_TELECOMMAND_DEFS_H__ */