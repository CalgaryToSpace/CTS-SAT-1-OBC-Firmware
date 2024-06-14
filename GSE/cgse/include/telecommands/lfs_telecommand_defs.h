
#ifndef __INCLUDE_GUARD__LFS_TELECOMMAND_DEFS_H__
#define __INCLUDE_GUARD__LFS_TELECOMMAND_DEFS_H__

#include <stdint.h>
#include "telecommands/telecommand_definitions.h"

uint8_t TCMDEXEC_fs_format_storage(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) { return 0; } 

uint8_t TCMDEXEC_fs_mount(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) { return 0; }

uint8_t TCMDEXEC_fs_unmount(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) { return 0; }

uint8_t TCMDEXEC_fs_write_file(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) { return 0; }
                        
uint8_t TCMDEXEC_fs_read_file(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) { return 0; }

uint8_t TCMDEXEC_fs_demo_write_then_read(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) { return 0; }

#endif /* __INCLUDE_GUARD__LFS_TELECOMMAND_DEFS_H__ */
