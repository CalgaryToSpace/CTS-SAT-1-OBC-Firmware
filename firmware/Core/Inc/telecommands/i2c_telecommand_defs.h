#include <stdint.h>
#include "telecommands/telecommand_definitions.h"
#include "main.h"


uint8_t TCMDEXEC_scan_i2c_device(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);