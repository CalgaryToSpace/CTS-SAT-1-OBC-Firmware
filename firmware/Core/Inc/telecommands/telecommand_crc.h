#include <stdint.h>
#include "telecommand_exec/telecommand_types.h"

uint8_t TCMDEXEC_crc(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,char *response_output_buf,
                        uint16_t response_output_buf_len);
