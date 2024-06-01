
#ifndef __INCLUDE_GUARD__FLASH_TELECOMMAND_DEFS_H__
#define __INCLUDE_GUARD__FLASH_TELECOMMAND_DEFS_H__

#include <stdint.h>
#include "telecommands/telecommand_definitions.h"

uint8_t TCMDEXEC_flash_activate_each_cs(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);
                        
uint8_t TCMDEXEC_flash_read_as_hex(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

#endif /* __INCLUDE_GUARD__FLASH_TELECOMMAND_DEFS_H__ */
