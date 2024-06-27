
#ifndef __INCLUDE_GUARD__LFS_TELECOMMAND_DEFS_H__
#define __INCLUDE_GUARD__LFS_TELECOMMAND_DEFS_H__

#include <stdint.h>
#include "telecommands/telecommand_definitions.h"

uint8_t TCMDEXEC_fs_format_storage(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) { return 0; } 

uint8_t TCMDEXEC_fs_mount(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) { return 0; }

uint8_t TCMDEXEC_fs_unmount(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) { return 0; }

uint8_t TCMDEXEC_fs_write_file(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) { return 0; }
                        
uint8_t TCMDEXEC_fs_read_file(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) { return 0; }

uint8_t TCMDEXEC_fs_delete_file(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) { return 0; }

uint8_t TCMDEXEC_fs_ls_dir(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) { return 0; }

uint8_t TCMDEXEC_fs_demo_write_then_read(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) { return 0; }

#endif /* __INCLUDE_GUARD__LFS_TELECOMMAND_DEFS_H__ */
