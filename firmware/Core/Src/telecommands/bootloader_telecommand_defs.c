#include "telecommands/bootloader_telecomand_defs.h"
#include "bootloader/bootloader.h"

uint8_t TCMDEXEC_jump_to_golden_copy(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len)
{
    BOOTLOADER_Jump_To_Golden_Copy();
    return 0;
}