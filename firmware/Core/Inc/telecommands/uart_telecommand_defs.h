#ifndef INCLUDE_GUARD__UART_TELECOMMAND_DEFS_H__
#define INCLUDE_GUARD__UART_TELECOMMAND_DEFS_H__

#include "telecommands/telecommand_definitions.h"

uint8_t TCMDEXEC_uart_send_bytes_hex(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
);

#endif /* INCLUDE_GUARD__UART_TELECOMMAND_DEFS_H__*/
