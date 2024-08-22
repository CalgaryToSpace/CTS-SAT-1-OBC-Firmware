#include "csp/drivers/usart.h"
#include "csp/arch/csp_time.h"

#include "log/log.h"
#include "transforms/arrays.h"

#include "main.h"

/**
   Write data on open UART.

   @param[in] fd file descriptor.
   @param[in] data data to write.
   @param[in] data_length length of \a data.
   @return number of bytes written on success, a negative value on failure.
*/
int csp_usart_write(csp_usart_fd_t fd, const void * data, size_t data_length) {
    char data_hex_str[200];
    GEN_byte_array_to_hex_str(data, data_length, data_hex_str, sizeof(data_hex_str));

    LOG_message(
        LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "csp_usart_write: fd: %d, data: %s, length: %d bytes",
        fd, data,
        data_hex_str,
        data_length
    );

    const HAL_StatusTypeDef hal_tx_result = HAL_UART_Transmit(
        &hlpuart1, (uint8_t *)data, data_length,
        1000 // Timeout // TODO: set lower
    );

    if (hal_tx_result != HAL_OK) {
        return -1;
    }

    return 0;
}


/**
   Return uptime in seconds.
   The function uses csp_get_s() for relative time. First time the function is called (by csp_init()), it saves an offset
   in case the platform doesn't start from 0, e.g. Linux.
   @return uptime in seconds.
*/
uint32_t csp_get_uptime_s(void) {
    return HAL_GetTick() / 1000;

}
