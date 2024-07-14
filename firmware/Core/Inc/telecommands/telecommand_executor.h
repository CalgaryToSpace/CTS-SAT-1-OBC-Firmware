
#ifndef INCLUDE_GUARD__TELECOMMAND_EXECUTOR_H
#define INCLUDE_GUARD__TELECOMMAND_EXECUTOR_H

#include <stdint.h>
#include "telecommands/telecommand_types.h"

uint8_t TCMD_execute_parsed_telecommand_now(const uint16_t tcmd_idx, const char args_str_no_parens[],
    TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_size
);

#endif // INCLUDE_GUARD__TELECOMMAND_EXECUTOR_H
