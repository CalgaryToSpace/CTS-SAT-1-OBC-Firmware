#ifndef INCLUDE_GUARD_CAMERA_TELECOMMAND_DEF_H
#define INCLUDE_GUARD_CAMERA_TELECOMMAND_DEF_H

#include <stdint.h>
#include "telecommands/telecommand_types.h"


uint8_t TCMDEXEC_camera_capture(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);


#endif // INCLUDE_GUARD_CAMERA_TELECOMMAND_DEF_H