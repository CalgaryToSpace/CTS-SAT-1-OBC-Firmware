#ifndef __INCLUDE_GUARD_STM32_INTERNAL_FLASH_TELECOMMAND_DEFS_H
#define __INCLUDE_GUARD_STM32_INTERNAL_FLASH_TELECOMMAND_DEFS_H

#include "telecommands/telecommand_types.h"
uint8_t TCMDEXEC_flash_bank_write(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                  char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_flash_bank_read(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                 char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_flash_bank_erase(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                  char *response_output_buf, uint16_t response_output_buf_len);

#endif /* __INCLUDE_GUARD_STM32_INTERNAL_FLASH_TELECOMMAND_DEFS_H */