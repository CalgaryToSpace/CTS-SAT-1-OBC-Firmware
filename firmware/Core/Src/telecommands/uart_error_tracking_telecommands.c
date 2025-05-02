#include "telecommands/uart_error_tracking_telecommands.h"
#include "uart_handler/uart_error_tracking.h"
#include <stdio.h>

uint8_t TCMDEXEC_uart_get_errors_json(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel, char *response_output_buf, uint16_t response_output_buf_len)
{

    const uint8_t ret = UART_get_errors_json(response_output_buf, response_output_buf_len);
    if (ret != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error retrieving UART error tracking data: %d", ret);
        return 1;
    }
    return 0;
}
