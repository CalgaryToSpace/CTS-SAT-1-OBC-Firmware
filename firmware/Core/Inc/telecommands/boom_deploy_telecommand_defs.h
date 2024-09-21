#ifndef INCLUDE_GUARD__BOOM_DEPLOY_DRIVERS_H
#define INCLUDE_GUARD__BOOM_DEPLOY_DRIVERS_H

#include <stdint.h>

uint8_t TCMDEXEC_boom_deploy_timed(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
);


#endif /* INCLUDE_GUARD__BOOM_DEPLOY_DRIVERS_H */
