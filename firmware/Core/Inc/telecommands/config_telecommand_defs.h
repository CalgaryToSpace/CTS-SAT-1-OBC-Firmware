#ifndef INCLUDE_GUARD_CONFIG_TELECOMMAND_DEFS_H
#define INCLUDE_GUARD_CONFIG_TELECOMMAND_DEFS_H

#include <stdint.h>
#include "telecommands/telecommand_types.h"


uint8_t TCMDEXEC_set_int_config_variable(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_set_str_config_variable(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_get_integer_configuration_variable(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_get_string_configuration_variable(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_get_all_configuration_variables(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);


#endif // INCLUDE_GUARD_CONFIG_TELECOMMAND_DEFS_H
