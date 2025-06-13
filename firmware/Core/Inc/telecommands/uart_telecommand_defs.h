#ifndef INCLUDE_GUARD__UART_TELECOMMAND_DEFS_H__
#define INCLUDE_GUARD__UART_TELECOMMAND_DEFS_H__

#include "telecommand_exec/telecommand_definitions.h"

uint8_t TCMDEXEC_uart_send_hex_get_response_hex(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_uart_get_last_rx_times_json(
    const char *args_str,
    TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf,
    uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_uart_set_baud_rate(
    const char *args_str,
    TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf,
    uint16_t response_output_buf_len
);

#endif /* INCLUDE_GUARD__UART_TELECOMMAND_DEFS_H__*/
