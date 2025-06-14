#ifndef INLUCDE_GUARD__UART_ERROR_TRACKING_TELECOMMANDS_H
#define INLUCDE_GUARD__UART_ERROR_TRACKING_TELECOMMANDS_H

#include <stdint.h>
#include "telecommand_exec/telecommand_definitions.h"

uint8_t TCMDEXEC_uart_get_errors_json(const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
);

#endif // INLUCDE_GUARD__UART_ERROR_TRACKING_TELECOMMANDS_H