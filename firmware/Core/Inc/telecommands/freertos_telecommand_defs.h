
#ifndef __INCLUDE_GUARD__FREERTOS_TELECOMMAND_DEFINITIONS_H
#define __INCLUDE_GUARD__FREERTOS_TELECOMMAND_DEFINITIONS_H

#include "telecommands/telecommand_types.h"

#include <stdint.h>

uint8_t TCMDEXEC_freetos_list_tasks_jsonl(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

#endif // __INCLUDE_GUARD__FREERTOS_TELECOMMAND_DEFINITIONS_H
