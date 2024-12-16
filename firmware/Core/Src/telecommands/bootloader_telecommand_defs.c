#include "stdio.h"

#include "telecommands/bootloader_telecomand_defs.h"
#include "bootloader/bootloader.h"


/// @brief Jump to golden copy of firmware
/// @param response_output_buf Only prints if something goes wrong
/// @return Only returns if something goes wrong
uint8_t TCMDEXEC_jump_to_golden_copy(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len)
{
    const uint8_t jump_failed = BOOTLOADER_Jump_To_Golden_Copy();
    if (jump_failed) {
        snprintf(response_output_buf, response_output_buf_len, "Something went wrong when trying to jump to golden copy. Error code: %u", jump_failed);    
        return 1;
    }
    // should never reach this point
    return 0;
}