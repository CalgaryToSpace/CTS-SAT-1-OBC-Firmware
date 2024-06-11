#include "uart_handler/uart_handler.h"
#include "debug_tools/debug_uart.h"
#include "main.h"

// Name the UART interfaces
UART_HandleTypeDef *UART_telecommand_port_handle = &hlpuart1;
UART_HandleTypeDef *UART_eps_port_handle = &huart5; // TODO: update this

// UART telecommand buffer
const uint16_t UART_telecommand_buffer_len = 256; // extern
volatile uint8_t UART_telecommand_buffer[256]; // extern // TODO: confirm that this volatile means that the contents are volatile but the pointer is not
volatile uint16_t UART_telecommand_buffer_write_idx = 0; // extern
volatile uint32_t UART_telecommand_last_write_time_ms = 0; // extern
volatile uint8_t UART_telecommand_buffer_last_rx_byte = 0; // not an extern

// UART EPS buffer
const uint16_t UART_eps_buffer_len = 300; // extern // 286 bytes max response, plus a bit for safety and tags
volatile uint8_t UART_eps_buffer[300]; // extern
volatile uint16_t UART_eps_buffer_write_idx = 0; // extern
volatile uint32_t UART_eps_last_write_time_ms = 0; // extern
volatile uint8_t UART_eps_is_expecting_data = 0; // extern; set to 1 when a command is sent, and we're awaiting a response
volatile uint8_t UART_eps_buffer_last_rx_byte = 0; // not an extern

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // This ISR function gets called every time a byte is received on the UART.

    if (huart->Instance == UART_telecommand_port_handle->Instance) {
        // THIS IS THE DEBUG/TELECOMMAND UART
        // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> Telecommand\n");
        
        // add the byte to the buffer
        if (UART_telecommand_buffer_write_idx >= UART_telecommand_buffer_len) {
            DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> UART telecommand buffer is full\n");
            
            // shift all bytes left by 1
            for (uint16_t i = 1; i < UART_telecommand_buffer_len; i++) {
                UART_telecommand_buffer[i - 1] = UART_telecommand_buffer[i];
            }

            // reset to a valid index
            UART_telecommand_buffer_write_idx = UART_telecommand_buffer_len - 1;
        }
        UART_telecommand_buffer[UART_telecommand_buffer_write_idx++] = UART_telecommand_buffer_last_rx_byte;
        UART_telecommand_last_write_time_ms = HAL_GetTick();
        HAL_UART_Receive_IT(UART_telecommand_port_handle, (uint8_t*) &UART_telecommand_buffer_last_rx_byte, 1);
    }
    else if (huart->Instance == UART_eps_port_handle->Instance) {
        // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> EPS Data\n");

        if (! UART_eps_is_expecting_data) {
            // not expecting data, ignore this noise
            return;
        }

        if (UART_eps_buffer_write_idx >= UART_eps_buffer_len) {
            DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> UART EPS buffer is full\n");
            // exit, with everything the way it is (stop appending)
            return;
        }

        UART_eps_buffer[UART_eps_buffer_write_idx++] = UART_eps_buffer_last_rx_byte;
        UART_eps_last_write_time_ms = HAL_GetTick();
        HAL_UART_Receive_IT(UART_eps_port_handle, (uint8_t*) &UART_eps_buffer_last_rx_byte, 1);
    }
    else {
        // FIXME: add the rest (camera, MPI, maybe others)
        DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> unknown UART instance\n"); // FIXME: remove
    }
}


void UART_init_uart_handlers(void)
{
    // enable the UART interrupt
    HAL_UART_Receive_IT(UART_telecommand_port_handle, (uint8_t*) &UART_telecommand_buffer_last_rx_byte, 1);
    HAL_UART_Receive_IT(UART_eps_port_handle, (uint8_t*) &UART_eps_buffer_last_rx_byte, 1);

    // TODO: add the rest
}
