#include "uart_handler/uart_handler.h"
#include "debug_tools/debug_uart.h"
#include "mpi/mpi_command_handling.h"
#include "main.h"
#include "log/log.h"

// Name the UART interfaces
UART_HandleTypeDef *UART_telecommand_port_handle = &hlpuart1;
UART_HandleTypeDef *UART_mpi_port_handle = &huart1;
UART_HandleTypeDef *UART_gps_port_handle = &huart3;
UART_HandleTypeDef *UART_camera_port_handle = &huart4;
UART_HandleTypeDef *UART_eps_port_handle = &huart5;

// UART telecommand buffer
const uint16_t UART_telecommand_buffer_len = 256;           // extern
volatile uint8_t UART_telecommand_buffer[256];              // extern       // TODO: confirm that this volatile means that the contents are volatile but the pointer is not
volatile uint16_t UART_telecommand_buffer_write_idx = 0;    // extern
volatile uint32_t UART_telecommand_last_write_time_ms = 0;  // extern
volatile uint8_t UART_telecommand_buffer_last_rx_byte = 0;  // not an extern

// UART MPI buffer
// TODO: Update buffer sizes to accommodate for incoming science data. Currently only configured for config commands
const uint16_t UART_mpi_buffer_len = 256;                   // extern       //Note: Max possible MPI response buffer size allocated to 50 bytes (Considering for the telecommand echo response, NOT science data.
volatile uint8_t UART_mpi_buffer[256];                      // extern
volatile uint8_t UART_mpi_last_rx_byte = 0;                 // extern
volatile uint32_t UART_mpi_last_write_time_ms = 0;          // extern
volatile uint16_t UART_mpi_buffer_write_idx = 0;            // extern

// UART CAMERA buffer
// TODO: Configure with peripheral required specifications
const uint16_t UART_camera_buffer_len = 1024;               // extern       // TODO: Set based on expected size requirements for reception
volatile uint8_t UART_camera_buffer[1024];                  // extern       // TODO: confirm that this volatile means that the contents are volatile but the pointer is not
volatile uint16_t UART_camera_buffer_write_idx = 0;         // extern
volatile uint32_t UART_camera_last_write_time_ms = 0;       // extern
volatile uint8_t UART_camera_is_expecting_data = 0;         // extern       // TODO: Set to 1 when a command is sent, and we're awaiting a response
volatile uint8_t UART_camera_buffer_last_rx_byte = 0;       // extern

// UART EPS buffer
const uint16_t UART_eps_buffer_len = 310;                   // extern       // Note: 286 bytes max response, plus a bit for safety and tags is expected
volatile uint8_t UART_eps_buffer[310];                      // extern
volatile uint16_t UART_eps_buffer_write_idx = 0;            // extern
volatile uint32_t UART_eps_last_write_time_ms = 0;          // extern
volatile uint8_t UART_eps_is_expecting_data = 0;            // extern       // Note: set to 1 when a command is sent, and we're awaiting a response
volatile uint8_t UART_eps_buffer_last_rx_byte = 0;          // not an extern

// UART MPI cmd response buffer
const uint16_t UART_mpi_rx_buffer_len = 50; // extern
volatile uint8_t UART_mpi_rx_buffer[50]; // extern
volatile uint8_t UART_mpi_rx_last_byte = 0; // extern
volatile uint32_t UART_mpi_rx_last_byte_write_time_ms = 0; // extern
volatile uint16_t UART_mpi_rx_buffer_write_idx = 0; // extern

// UART GPS buffer
const uint16_t UART_gps_buffer_len = 512; // extern
volatile uint8_t UART_gps_buffer[512]; // extern
volatile uint16_t UART_gps_buffer_write_idx = 0; // extern
volatile uint32_t UART_gps_last_write_time_ms = 0; // extern
volatile uint8_t UART_gps_buffer_last_rx_byte = 0; // extern
volatile uint8_t UART_gps_uart_interrupt_enabled = 0; //extern

// UART MPI science data buffer (WILL NEED IN THE FUTURE)
// const uint16_t UART_mpi_data_rx_buffer_len = 8192; // extern 
// volatile uint8_t UART_mpi_data_rx_buffer[8192]; // extern
// const uint16_t UART_mpi_data_buffer_len = 80000; // extern
// volatile uint8_t UART_mpi_data_buffer[80000]; // extern

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    // This ISR function gets called every time a byte is received on the UART.

    if (huart->Instance == UART_telecommand_port_handle->Instance) {
        // THIS IS THE DEBUG/TELECOMMAND UART
        // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> Telecommand\n");
        
        // Add the byte to the buffer.
        if (UART_telecommand_buffer_write_idx >= UART_telecommand_buffer_len) {
            DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> UART telecommand buffer is full\n");
            
            // shift all bytes left by 1
            for (uint16_t i = 1; i < UART_telecommand_buffer_len; i++) {
                UART_telecommand_buffer[i - 1] = UART_telecommand_buffer[i];
            }

            // reset to a valid index
            UART_telecommand_buffer_write_idx = UART_telecommand_buffer_len - 1;
        }

        // Add the received byte to the buffer
        UART_telecommand_buffer[UART_telecommand_buffer_write_idx++] = UART_telecommand_buffer_last_rx_byte;
        UART_telecommand_last_write_time_ms = HAL_GetTick();

        // Restart reception for next byte
        HAL_UART_Receive_IT(UART_telecommand_port_handle, (uint8_t*) &UART_telecommand_buffer_last_rx_byte, 1);
    }

    else if (huart->Instance == UART_mpi_port_handle->Instance) {
        // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> MPI Data\n");

        if (MPI_current_uart_rx_mode == MPI_RX_MODE_COMMAND_MODE) {
            // Check if buffer is full
            if (UART_mpi_buffer_write_idx >= UART_mpi_buffer_len) {
                // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> UART mpi response buffer is full\n");

                // Shift all bytes left by 1
                for(uint16_t i = 1; i < UART_mpi_buffer_len; i++) {
                    UART_mpi_buffer[i - 1] = UART_mpi_buffer[i];
                }

                // Reset to a valid index
                UART_mpi_buffer_write_idx = UART_mpi_buffer_len - 1;
            }

            // Add the received byte to the buffer
            UART_mpi_buffer[UART_mpi_buffer_write_idx++] = UART_mpi_last_rx_byte;
            UART_mpi_last_write_time_ms = HAL_GetTick();
        }
        else {
            DEBUG_uart_print_str("Unhandled MPI Mode\n"); //TODO: HANDLE other MPI MODES
        }
    }

    
    // TODO: Implement function to utilize this DMA reception callback for the CAMERA
    else if (huart->Instance == UART_camera_port_handle->Instance) {
        // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> CAMERA Data\n");

        if (! UART_camera_is_expecting_data) {
            // Not expecting data, ignore this noise.
            return;
        }

        // Check if buffer is full.
        if (UART_camera_buffer_write_idx >= UART_camera_buffer_len) {
            // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> UART response buffer is full\n");

            // Shift all bytes left by 1
            for(uint16_t i = 1; i < UART_camera_buffer_len; i++) {
                UART_camera_buffer[i - 1] = UART_camera_buffer[i];
            }

            // Reset to a valid index
            UART_camera_buffer_write_idx = UART_camera_buffer_len - 1;
        }

        // FIXME: Actually deal with the data in here, if necessary.

        UART_camera_last_write_time_ms = HAL_GetTick();
    }           

    else if (huart->Instance == UART_eps_port_handle->Instance) {
        // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> EPS Data\n");

        if (! UART_eps_is_expecting_data) {
            // Not expecting data, ignore this noise.
            // Intentionally re-enable the interrupt though, because the EPS is quite important,
            // and is deemed reliable over UART. Not too worried about spurious interrupts.
            HAL_UART_Receive_IT(UART_eps_port_handle, (uint8_t*) &UART_eps_buffer_last_rx_byte, 1);
            return;
        }

        if (UART_eps_buffer_write_idx >= UART_eps_buffer_len) {
            // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> UART EPS buffer is full\n");

            HAL_UART_Receive_IT(UART_eps_port_handle, (uint8_t*) &UART_eps_buffer_last_rx_byte, 1);
            // Exit, with everything the way it is (stop appending)
            return;
        }

        // Add the byte to the buffer
        UART_eps_buffer[UART_eps_buffer_write_idx++] = UART_eps_buffer_last_rx_byte;
        UART_eps_last_write_time_ms = HAL_GetTick();

        // Restart reception for next byte
        HAL_UART_Receive_IT(UART_eps_port_handle, (uint8_t*) &UART_eps_buffer_last_rx_byte, 1);
    }

    else if (huart->Instance == UART_gps_port_handle->Instance) {
        // Note: If the GPS is not enabled, the interrupt is not re-enabled via HAL_UART_Receive_IT().
        // This is a safety feature against the GPS spamming null bytes, which can lock up the system.
        if (UART_gps_uart_interrupt_enabled == 1) {

            // Add the byte to the buffer
            if (UART_gps_buffer_write_idx >= UART_gps_buffer_len) {
                DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> UART gps buffer is full\n");
                
                // Shift all bytes left by 1
                for (uint16_t i = 1; i < UART_gps_buffer_len; i++) {
                    UART_gps_buffer[i - 1] = UART_gps_buffer[i];
                }

                // Reset to a valid index
                UART_gps_buffer_write_idx = UART_gps_buffer_len - 1;
            }
            UART_gps_buffer[UART_gps_buffer_write_idx++] = UART_gps_buffer_last_rx_byte;
            UART_gps_last_write_time_ms = HAL_GetTick();

            HAL_UART_Receive_IT(UART_gps_port_handle, (uint8_t*) &UART_gps_buffer_last_rx_byte, 1);
        }
        
    }

    else {
        // FIXME: add the rest (camera, MPI, maybe others)
        DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> unknown UART instance\n"); // FIXME: remove
    }
}



/// @brief Sets the UART interrupt state (enabled/disabled)
/// @param new_enabled 1: enable interrupt; 0: disable interrupt
/// @note This function must be called very carefully. This type of GPS is known to, in the wrong
///       mode, spam null bytes, which can lock up the entire system. Thus, the interrupt is disabled
///       by default, and must be enabled explicitly by the GPS telecommands.
void GPS_set_uart_interrupt_state(uint8_t new_enabled) {
    if (new_enabled == 1)
    {
        UART_gps_uart_interrupt_enabled = 1;
        HAL_UART_Receive_IT(UART_gps_port_handle, (uint8_t*) &UART_gps_buffer_last_rx_byte, 1);
    }
    else {
        UART_gps_uart_interrupt_enabled = 0;
    }
}

// TODO: Probably need to remove the LOG_message() calls below. Instead, set a fault flag.

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    // Reception Error callback for MPI UART port
    if (huart->Instance == UART_mpi_port_handle->Instance) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "MPI UART Reception Error: %lu", huart->ErrorCode
        );

        HAL_UART_Receive_DMA(UART_mpi_port_handle, (uint8_t*)&UART_mpi_last_rx_byte, 1);
    }

    // Reception Error callback for GPS UART port
    if (huart->Instance == UART_gps_port_handle->Instance) {
        LOG_message(
            LOG_SYSTEM_GPS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "GPS UART Reception Error: %lu", huart->ErrorCode
        );

        if (UART_gps_uart_interrupt_enabled == 1) {
            HAL_UART_Receive_IT(UART_gps_port_handle, (uint8_t*)&UART_gps_buffer_last_rx_byte, 1);
        }
    }

    // Reception Error callback for CAMERA UART port
    if (huart->Instance == UART_camera_port_handle->Instance) {
        LOG_message(
            LOG_SYSTEM_BOOM, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "CAMERA UART Reception Error: %lu", huart->ErrorCode
        ); // TODO: CAMERA is not registered as a system in the logger yet, Telecommand system used instead

        // Do not re-enable the interrupt here. Afraid of negative feedback loop.
    }

    // Reception Error callback for EPS UART port
    if (huart->Instance == UART_eps_port_handle->Instance) {
        LOG_message(
            LOG_SYSTEM_EPS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "EPS UART Reception Error: %lu", huart->ErrorCode
        );

        // We trust the EPS. Always re-enable the interrupt.
        HAL_UART_Receive_IT(UART_eps_port_handle, (uint8_t*)&UART_eps_buffer_last_rx_byte, 1);
    }
}

void UART_init_uart_handlers(void) {
    // Enable the UART interrupt
    HAL_UART_Receive_IT(UART_telecommand_port_handle, (uint8_t*) &UART_telecommand_buffer_last_rx_byte, 1);
    HAL_UART_Receive_IT(UART_eps_port_handle, (uint8_t*) &UART_eps_buffer_last_rx_byte, 1);

    // GPS is not initialized as always-listening. It is enabled by the GPS telecommands.
    // Reason: The GPS has a mode where it spams null bytes, which can lock up the entire system.
    // Thus, its interrupt is disabled by default.

    // TODO: Verify these when peripheral implementations are added
}
