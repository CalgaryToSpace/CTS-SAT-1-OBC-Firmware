#ifndef INCLUDE_GUARD_CONFIG_TELECOMMAND_DEFS_H
#define INCLUDE_GUARD_CONFIG_TELECOMMAND_DEFS_H

#include <stdint.h>
#include "telecommand_exec/telecommand_types.h"


uint8_t TCMDEXEC_config_set_int_var(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_config_set_str_var(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_config_get_int_var_json(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_config_get_str_var_json(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_config_get_all_vars_jsonl(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);


#endif // INCLUDE_GUARD_CONFIG_TELECOMMAND_DEFS_H
