#ifndef INCLUDE_GUARD__GNSS_TELECOMMAND_DEFS_H
#define INCLUDE_GUARD__GNSS_TELECOMMAND_DEFS_H

#include <stdint.h>

uint8_t TCMDEXEC_gnss_send_cmd_ascii(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_gnss_enable_firehose_storage_mode(
    const char *args_str, char *response_output_buf, uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_gnss_disable_firehose_storage_mode(
    const char *args_str, char *response_output_buf, uint16_t response_output_buf_len
);

#endif // INCLUDE_GUARD__GNSS_TELECOMMAND_DEFS_H
