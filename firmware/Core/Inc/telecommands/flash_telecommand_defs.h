
#ifndef __INCLUDE_GUARD__FLASH_TELECOMMAND_DEFS_H__
#define __INCLUDE_GUARD__FLASH_TELECOMMAND_DEFS_H__

#include <stdint.h>
#include "telecommands/telecommand_definitions.h"

uint8_t TCMDEXEC_flash_activate_each_cs(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_flash_each_is_reachable(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);
                        
uint8_t TCMDEXEC_flash_read_hex(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_flash_write_hex(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_flash_erase(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_flash_benchmark_erase_write_read(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);
                        
#endif /* __INCLUDE_GUARD__FLASH_TELECOMMAND_DEFS_H__ */
