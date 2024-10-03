
#ifndef __INCLUDE_GUARD__FLASH_TELECOMMAND_DEFS_H__
#define __INCLUDE_GUARD__FLASH_TELECOMMAND_DEFS_H__

#include <stdint.h>
#include "telecommands/telecommand_definitions.h"

/*----------------------------- CONFIG VARIABLES ----------------------------- */
// NAND Flash Memory Datasheet https://www.farnell.com/datasheets/3151163.pdf (pg.7)
// Each page is divided into a 2048-byte data storage region, and a 128 bytes spare area (2176 bytes total).
#define MAX_NUM_BYTES 2048

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

uint8_t TCMDEXEC_flash_reset(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);
                        
#endif /* __INCLUDE_GUARD__FLASH_TELECOMMAND_DEFS_H__ */
