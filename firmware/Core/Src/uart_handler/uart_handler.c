#include "uart_handler/uart_handler.h"
#include "debug_tools/debug_uart.h"
#include "mpi/mpi_command_handling.h"
#include "main.h"

// UART telecommand buffer
const uint16_t UART_telecommand_buffer_len = 256; // extern
volatile uint8_t UART_telecommand_buffer[256]; // extern // TODO: confirm that this volatile means that the contents are volatile but the pointer is not
volatile uint16_t UART_telecommand_buffer_write_idx = 0; // extern
volatile uint32_t UART_telecommand_last_write_time_ms = 0; // extern
volatile uint8_t UART_telecommand_buffer_last_rx_byte = 0; // extern

// UART MPI cmd response buffer
const uint16_t UART_mpi_rx_buffer_len = 50; // extern
volatile uint8_t UART_mpi_rx_buffer[50]; // extern
volatile uint8_t UART_mpi_rx_last_byte = 0; // extern
volatile uint32_t UART_mpi_rx_last_byte_write_time_ms = 0; // extern
volatile uint16_t UART_mpi_rx_buffer_write_idx = 0; // extern

// UART MPI science data buffer (WILL NEED IN THE FUTURE)
// const uint16_t UART_mpi_data_rx_buffer_len = 8192; // extern 
// volatile uint8_t UART_mpi_data_rx_buffer[8192]; // extern
// const uint16_t UART_mpi_data_buffer_len = 80000; // extern
// volatile uint8_t UART_mpi_data_buffer[80000]; //extern

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // This ISR function gets called every time a byte is received on the UART.

    if (huart->Instance == LPUART1) {
        // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> LPUART1\n");
        
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
        HAL_UART_Receive_IT(&hlpuart1, (uint8_t*) &UART_telecommand_buffer_last_rx_byte, 1);
    }
    else if (huart->Instance == USART1){

        // TODO: Deal with overflow
        if(current_mpi_mode == COMMAND_MODE) {
            // Check if buffer is full
            if(UART_mpi_rx_buffer_write_idx >= UART_mpi_rx_buffer_len){
                DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> UART mpi response buffer is full\n");

                // Shift all bytes left by 1
                for(uint16_t i = 1; i < UART_mpi_rx_buffer_len; i++){
                    UART_mpi_rx_buffer[i - 1] = UART_mpi_rx_buffer[i];
                }

                // Reset to a valid index
                UART_mpi_rx_buffer_write_idx = UART_mpi_rx_buffer_len - 1;
            }

            // Add a byte to the buffer
            UART_mpi_rx_buffer[UART_mpi_rx_buffer_write_idx++] = UART_mpi_rx_last_byte;
            UART_mpi_rx_last_byte_write_time_ms = HAL_GetTick();
        }
        else{
            DEBUG_uart_print_str("Unhandled MPI Mode\n"); //FIXME: HANDLE THIS MPI MODE
        }
    }
    else {
        // FIXME: add the rest
        DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> unknown UART instance\n"); // FIXME: remove
    }
}


void UART_init_uart_handlers(void)
{
    // enable the UART interrupt
    HAL_UART_Receive_IT(&hlpuart1, (uint8_t*) &UART_telecommand_buffer_last_rx_byte, 1);

    // TODO: add the rest
}