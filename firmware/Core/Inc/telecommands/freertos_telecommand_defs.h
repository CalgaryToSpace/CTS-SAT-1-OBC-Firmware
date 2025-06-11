
#ifndef INCLUDE_GUARD__FREERTOS_TELECOMMAND_DEFINITIONS_H
#define INCLUDE_GUARD__FREERTOS_TELECOMMAND_DEFINITIONS_H

#include "telecommand_exec/telecommand_types.h"

#include <stdint.h>

uint8_t TCMDEXEC_freetos_list_tasks_jsonl(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_freertos_demo_stack_usage(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

#endif // INCLUDE_GUARD__FREERTOS_TELECOMMAND_DEFINITIONS_H
