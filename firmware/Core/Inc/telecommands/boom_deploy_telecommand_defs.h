#ifndef INCLUDE_GUARD__BOOM_DEPLOY_TELECOMMAND_DEFS_H
#define INCLUDE_GUARD__BOOM_DEPLOY_TELECOMMAND_DEFS_H

#include <stdint.h>

uint8_t TCMDEXEC_boom_deploy_timed(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
);


uint8_t TCMDEXEC_boom_self_check(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
);

#endif /* INCLUDE_GUARD__BOOM_DEPLOY_TELECOMMAND_DEFS_H */
