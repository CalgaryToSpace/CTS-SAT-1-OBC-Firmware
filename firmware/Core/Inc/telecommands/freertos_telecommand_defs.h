
#ifndef __INCLUDE_GUARD__FREERTOS_TELECOMMAND_DEFINITIONS_H
#define __INCLUDE_GUARD__FREERTOS_TELECOMMAND_DEFINITIONS_H

#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>
#include "telecommands/telecommand_types.h"

const char* freertos_current_state_enum_to_str(eTaskState state);

uint8_t TCMDEXEC_freertos_get_metadata(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

#endif // __INCLUDE_GUARD__FREERTOS_TELECOMMAND_DEFINITIONS_H
