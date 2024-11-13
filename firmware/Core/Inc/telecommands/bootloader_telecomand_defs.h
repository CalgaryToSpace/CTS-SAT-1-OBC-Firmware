#ifndef __INCLUDE_GUARD_BOOTLOADER_TELECOMMAND_DEFS_H__
#define __INCLUDE_GUARD_BOOTLOADER_TELECOMMAND_DEFS_H__

#include "telecommands/telecommand_types.h"

uint8_t TCMDEXEC_jump_to_golden_copy(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len);

#endif // __INCLUDE_GUARD_BOOTLOADER_TELECOMMAND_DEFS_H__