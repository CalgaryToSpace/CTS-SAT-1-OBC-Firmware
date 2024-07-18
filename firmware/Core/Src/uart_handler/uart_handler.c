#include "uart_handler/uart_handler.h"
#include "debug_tools/debug_uart.h"
#include "log/log.h"
#include "main.h"

// UART telecommand buffer
const uint16_t UART_telecommand_buffer_len = 256; // extern
volatile uint8_t UART_telecommand_buffer[256]; // extern // TODO: confirm that this volatile means that the contents are volatile but the pointer is not
volatile uint16_t UART_telecommand_buffer_write_idx = 0; // extern
volatile uint32_t UART_telecommand_last_write_time_ms = 0; // extern
volatile uint8_t UART_telecommand_buffer_last_rx_byte = 0; // extern


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // This ISR function gets called every time a byte is received on a UART.

    // TODO: Replace numbered UART interfaces with semantic names, like MPIUART, etc.
    if (huart->Instance == LPUART1) {
        //LOG_message(LOG_SYSTEM_UMBILICAL_UART, LOG_SEVERITY_DEBUG, LOG_CHANNEL_ALL, "HAL_UART_RxCpltCallback() -> LPUART1");
        
        // add the byte to the buffer
        if (UART_telecommand_buffer_write_idx >= UART_telecommand_buffer_len) {
            LOG_message(LOG_SYSTEM_UMBILICAL_UART, LOG_SEVERITY_WARNING, LOG_CHANNEL_ALL, "HAL_UART_RxCpltCallback() -> UART telecommand buffer is full");
            
            // shift all bytes left by 1
            for (uint16_t i = 1; i < UART_telecommand_buffer_len; i++) {
                UART_telecommand_buffer[i - 1] = UART_telecommand_buffer[i];
            }

            // reset to a valid index
            UART_telecommand_buffer_write_idx = UART_telecommand_buffer_len - 1;
        }
        UART_telecommand_buffer[UART_telecommand_buffer_write_idx++] = UART_telecommand_buffer_last_rx_byte;
        UART_telecommand_last_write_time_ms = HAL_GetTick();
        HAL_UART_Receive_IT(&hlpuart1, (uint8_t*) &UART_telecommand_buffer_last_rx_byte, 1);
    }
    // FIXME: add the rest
    else {
        LOG_message(LOG_SYSTEM_UNKNOWN, LOG_SEVERITY_CRITICAL, LOG_CHANNEL_ALL, "HAL_UART_RxCpltCallback() -> unknown UART instance"); 
    }
}


void UART_init_uart_handlers(void)
{
    // enable the UART interrupt
    HAL_UART_Receive_IT(&hlpuart1, (uint8_t*) &UART_telecommand_buffer_last_rx_byte, 1);

    // TODO: add the rest
}
