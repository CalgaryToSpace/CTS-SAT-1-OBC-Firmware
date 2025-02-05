#ifndef INCLUDE_GUARD__AGENDA_TELECOMMAND_DEFINITIONS_H
#define INCLUDE_GUARD__AGENDA_TELECOMMAND_DEFINITIONS_H

#include "telecommands/telecommand_types.h"

#include <stdint.h>

uint8_t TCMDEXEC_agenda_delete_all(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_agenda_delete_by_tssent(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_agenda_fetch_jsonl(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_agenda_delete_by_name(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

#endif // INCLUDE_GUARD__AGENDA_TELECOMMAND_DEFINITIONS_H