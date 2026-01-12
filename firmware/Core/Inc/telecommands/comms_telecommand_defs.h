#ifndef INCLUDE_GUARD__COMMS_TELECOMMAND_DEFS_H
#define INCLUDE_GUARD__COMMS_TELECOMMAND_DEFS_H

#include "telecommand_exec/telecommand_definitions.h"

#include <stdint.h>

uint8_t TCMDEXEC_comms_set_rf_switch_control_mode(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_comms_get_rf_switch_info(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
);

// MARK: Bulk Downlink
uint8_t TCMDEXEC_comms_bulk_file_downlink_start(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
);
uint8_t TCMDEXEC_comms_bulk_file_downlink_pause(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
);
uint8_t TCMDEXEC_comms_bulk_file_downlink_resume(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
);


// MARK: Bulk Uplink
uint8_t TCMDEXEC_comms_bulk_uplink_open_file(
    const char *args_str,
    char *response_output_buf,
    uint16_t response_output_buf_len
);
uint8_t TCMDEXEC_comms_bulk_uplink_seek(
    const char *args_str,
    char *response_output_buf,
    uint16_t response_output_buf_len
);
uint8_t TCMDEXEC_comms_bulk_uplink_write_bytes_hex(
    const char *args_str,
    char *response_output_buf,
    uint16_t response_output_buf_len
);
uint8_t TCMDEXEC_bulkup16(
    const char *args_str,
    char *response_output_buf,
    uint16_t response_output_buf_len
);
uint8_t TCMDEXEC_comms_bulk_uplink_write_bytes_base64(
    const char *args_str,
    char *response_output_buf,
    uint16_t response_output_buf_len
);
uint8_t TCMDEXEC_bulkup64(
    const char *args_str,
    char *response_output_buf,
    uint16_t response_output_buf_len
);
uint8_t TCMDEXEC_comms_bulk_uplink_close_file(
    const char *args_str,
    char *response_output_buf,
    uint16_t response_output_buf_len
);

#endif // INCLUDE_GUARD__COMMS_TELECOMMAND_DEFS_H
