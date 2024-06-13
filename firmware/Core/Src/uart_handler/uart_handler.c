#include "uart_handler/uart_handler.h"
#include "debug_tools/debug_uart.h"
#include "mpi/mpi_command_handling.h"
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

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
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
            HAL_UART_Receive_IT(UART_eps_port_handle, (uint8_t*) &UART_eps_buffer_last_rx_byte, 1);
            return;
        }

        if (UART_eps_buffer_write_idx >= UART_eps_buffer_len) {
            DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> UART EPS buffer is full\n");
            HAL_UART_Receive_IT(UART_eps_port_handle, (uint8_t*) &UART_eps_buffer_last_rx_byte, 1);
            // exit, with everything the way it is (stop appending)
            return;
        }

        UART_eps_buffer[UART_eps_buffer_write_idx++] = UART_eps_buffer_last_rx_byte;
        UART_eps_last_write_time_ms = HAL_GetTick();
        HAL_UART_Receive_IT(UART_eps_port_handle, (uint8_t*) &UART_eps_buffer_last_rx_byte, 1);
    }
    else if (huart->Instance == USART1) {
        if (MPI_current_uart_rx_mode == MPI_RX_MODE_COMMAND_MODE) {
            // Check if buffer is full
            if (UART_mpi_rx_buffer_write_idx >= UART_mpi_rx_buffer_len) {
                // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> UART mpi response buffer is full\n");

                // Shift all bytes left by 1
                for(uint16_t i = 1; i < UART_mpi_rx_buffer_len; i++) {
                    UART_mpi_rx_buffer[i - 1] = UART_mpi_rx_buffer[i];
                }

                // Reset to a valid index
                UART_mpi_rx_buffer_write_idx = UART_mpi_rx_buffer_len - 1;
            }

            // Add a byte to the buffer
            UART_mpi_rx_buffer[UART_mpi_rx_buffer_write_idx++] = UART_mpi_rx_last_byte;
            UART_mpi_rx_last_byte_write_time_ms = HAL_GetTick();
        }
        else {
            DEBUG_uart_print_str("Unhandled MPI Mode\n"); //TODO: HANDLE other MPI MODES
        }
    }
    else {
        // FIXME: add the rest (camera, MPI, maybe others)
        DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> unknown UART instance\n"); // FIXME: remove
    }
}

void UART_init_uart_handlers(void) {
    // enable the UART interrupt
    HAL_UART_Receive_IT(UART_telecommand_port_handle, (uint8_t*) &UART_telecommand_buffer_last_rx_byte, 1);
    HAL_UART_Receive_IT(UART_eps_port_handle, (uint8_t*) &UART_eps_buffer_last_rx_byte, 1);

    // TODO: add the rest
}