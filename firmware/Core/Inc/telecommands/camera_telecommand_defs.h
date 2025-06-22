#ifndef INCLUDE_GUARD__CAMERA_TELECOMMAND_DEFS_H
#define INCLUDE_GUARD__CAMERA_TELECOMMAND_DEFS_H

#include <stdint.h>
#include "telecommand_exec/telecommand_types.h"


uint8_t TCMDEXEC_camera_setup(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
);
    
uint8_t TCMDEXEC_camera_test(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_camera_change_baud_rate(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_camera_capture(const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
);


#endif // INCLUDE_GUARD__CAMERA_TELECOMMAND_DEFS_H
