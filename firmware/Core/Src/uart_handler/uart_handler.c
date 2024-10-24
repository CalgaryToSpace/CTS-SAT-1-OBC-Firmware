#include "uart_handler/uart_handler.h"
#include "debug_tools/debug_uart.h"
#include "mpi/mpi_command_handling.h"
#include "main.h"
#include "log/log.h"

// Name the UART interfaces
UART_HandleTypeDef *UART_telecommand_port_handle = &hlpuart1;
UART_HandleTypeDef *UART_mpi_port_handle = &huart1;
UART_HandleTypeDef *UART_lora_port_handle = &huart2;
UART_HandleTypeDef *UART_gps_port_handle = &huart3;
UART_HandleTypeDef *UART_camera_port_handle = &huart4;
UART_HandleTypeDef *UART_eps_port_handle = &huart5;

// UART telecommand buffer
const uint16_t UART_telecommand_buffer_len = 256;           // extern
volatile uint8_t UART_telecommand_buffer[256];              // extern       //TODO: confirm that this volatile means that the contents are volatile but the pointer is not
volatile uint16_t UART_telecommand_buffer_write_idx = 0;    // extern
volatile uint32_t UART_telecommand_last_write_time_ms = 0;  // extern
volatile uint8_t UART_telecommand_buffer_last_rx_byte = 0;  // not an extern

// UART MPI buffer
// TODO: Update buffer sizes to accomodate for incoming science data. Currently only configured for config commands
const uint16_t UART_mpi_buffer_len = 5120;                  // extern       //Note: Max possible MPI response buffer size allocated to 50 bytes (Considering for the telecommand echo response, NOT science data.
volatile uint8_t UART_mpi_buffer[5120];                     // extern
volatile uint8_t UART_mpi_last_rx_byte = 0;                 // extern
volatile uint32_t UART_mpi_last_write_time_ms = 0;          // extern
volatile uint16_t UART_mpi_buffer_write_idx = 0;            // extern

// UART LORA buffer                                 
//TODO: Configure with peripheral required specifications
const uint16_t UART_lora_buffer_len = 5120;                 // extern
volatile uint8_t UART_lora_buffer[5120];                    // extern
volatile uint16_t UART_lora_buffer_write_idx = 0;           // extern
volatile uint32_t UART_lora_last_write_time_ms = 0;         // extern
volatile uint8_t UART_lora_is_expecting_data = 0;           // extern;      //TODO: Set to 1 when a command is sent, and we're awaiting a response
volatile uint8_t UART_lora_buffer_last_rx_byte = 0;         // not an extern

// UART GPS buffer
// TODO: Configure with peripheral required specifications
const uint16_t UART_gps_buffer_len = 5120;                  // extern
volatile uint8_t UART_gps_buffer[5120];                     // extern
volatile uint16_t UART_gps_buffer_write_idx = 0;            // extern
volatile uint32_t UART_gps_last_write_time_ms = 0;          // extern
volatile uint8_t UART_gps_buffer_last_rx_byte = 0;          // not an extern

// UART CAMERA buffer
// TODO: Configure with peripheral required specifications
const uint16_t UART_camera_buffer_len = 5120;               // extern
volatile uint8_t UART_camera_buffer[5120];                  // extern       //TODO: confirm that this volatile means that the contents are volatile but the pointer is not
volatile uint16_t UART_camera_buffer_write_idx = 0;         // extern
volatile uint32_t UART_camera_last_write_time_ms = 0;       // extern
volatile uint8_t UART_camera_is_expecting_data = 0;         // extern       //TODO: Set to 1 when a command is sent, and we're awaiting a response
volatile uint8_t UART_camera_buffer_last_rx_byte = 0;       // extern

// UART EPS buffer
const uint16_t UART_eps_buffer_len = 5120;                  // extern       //Note: 286 bytes max response, plus a bit for safety and tags is expected
volatile uint8_t UART_eps_buffer[5120];                     // extern
volatile uint16_t UART_eps_buffer_write_idx = 0;            // extern
volatile uint32_t UART_eps_last_write_time_ms = 0;          // extern
volatile uint8_t UART_eps_is_expecting_data = 0;            // extern       //Note: set to 1 when a command is sent, and we're awaiting a response
volatile uint8_t UART_eps_buffer_last_rx_byte = 0;          // not an extern


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

    // TODO: Verify implementation with peripheral connected. Currently configured to follow interrupt based receive
    else if(huart->Instance == UART_lora_port_handle->Instance){
        // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> LORA Data\n");

        if (! UART_lora_is_expecting_data) {
            // not expecting data, ignore this noise
            HAL_UART_Receive_IT(UART_lora_port_handle, (uint8_t*) &UART_lora_buffer_last_rx_byte, 1);
            return;
        }

        // Add the byte to the buffer
        if (UART_lora_buffer_write_idx >= UART_lora_buffer_len) {
            // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> UART response buffer is full\n");
            
            // Shift all bytes left by 1
            for (uint16_t i = 1; i < UART_lora_buffer_len; i++) {
                UART_lora_buffer[i - 1] = UART_lora_buffer[i];
            }

            // Reset to a valid index
            UART_lora_buffer_write_idx = UART_lora_buffer_len - 1;
        }

        // Add the received byte to the buffer
        UART_lora_buffer[UART_lora_buffer_write_idx++] = UART_lora_buffer_last_rx_byte;
        UART_lora_last_write_time_ms = HAL_GetTick();

        // Restart reception for next byte
        HAL_UART_Receive_IT(UART_lora_port_handle, (uint8_t*) &UART_lora_buffer_last_rx_byte, 1);
    }        

    // TODO: Verify implementation with peripheral connected. Currently configured to follow interrupt based receive
    else if(huart->Instance == UART_gps_port_handle->Instance){
        // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> GPS Data\n");
        
        // Add the byte to the buffer
        if (UART_gps_buffer_write_idx >= UART_gps_buffer_len) {
            // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> UART response buffer is full\n");
            
            // Shift all bytes left by 1
            for (uint16_t i = 1; i < UART_gps_buffer_len; i++) {
                UART_gps_buffer[i - 1] = UART_gps_buffer[i];
            }

            // Reset to a valid index
            UART_gps_buffer_write_idx = UART_gps_buffer_len - 1;
        }

        // Add a byte to the buffer
        UART_gps_buffer[UART_gps_buffer_write_idx++] = UART_gps_buffer_last_rx_byte;
        UART_gps_last_write_time_ms = HAL_GetTick();

        // Restart reception for next byte
        HAL_UART_Receive_IT(UART_gps_port_handle, (uint8_t*) &UART_gps_buffer_last_rx_byte, 1);
    }

    // TODO: Implement function to utilize this DMA reception callback for the CAMERA
    else if (huart->Instance == UART_camera_port_handle->Instance) {
        // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> CAMERA Data\n");

        if (! UART_camera_is_expecting_data) {
            // not expecting data, ignore this noise
            HAL_UART_Receive_IT(UART_camera_port_handle, (uint8_t*) &UART_camera_buffer_last_rx_byte, 1);
            return;
        }

        // Check if buffer is full
        if (UART_camera_buffer_write_idx >= UART_camera_buffer_len) {
            // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> UART response buffer is full\n");

            // Shift all bytes left by 1
            for(uint16_t i = 1; i < UART_camera_buffer_len; i++) {
                UART_camera_buffer[i - 1] = UART_camera_buffer[i];
            }

            // Reset to a valid index
            UART_camera_buffer_write_idx = UART_camera_buffer_len - 1;
        }

        // Add a byte to the buffer
        UART_camera_buffer[UART_camera_buffer_write_idx++] = UART_camera_buffer_last_rx_byte;
        UART_camera_last_write_time_ms = HAL_GetTick();
    }           

    else if (huart->Instance == UART_eps_port_handle->Instance) {
        // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> EPS Data\n");

        if (! UART_eps_is_expecting_data) {
            // Not expecting data, ignore this noise
            HAL_UART_Receive_IT(UART_eps_port_handle, (uint8_t*) &UART_eps_buffer_last_rx_byte, 1);
            return;
        }

        if (UART_eps_buffer_write_idx >= UART_eps_buffer_len) {
            // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> UART EPS buffer is full\n"); //TODO: Remove this later

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

    else {
        // FIXME: add the rest (camera, MPI, maybe others)
        DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> unknown UART instance\n"); // FIXME: remove
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {

    // Reception Error callback for MPI UART port
    if (huart->Instance == UART_mpi_port_handle->Instance) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "MPI UART Reception Error: %d", huart->ErrorCode
        );

        HAL_UART_Receive_DMA(UART_mpi_port_handle, (uint8_t*)&UART_mpi_last_rx_byte, 1);
    }

    // Reception Error callback for LORA UART port
    if (huart->Instance == UART_lora_port_handle->Instance) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "LORA UART Reception Error: %d", huart->ErrorCode
        ); // TODO: LORA is not registered as a system in the logger yet

        HAL_UART_Receive_IT(UART_lora_port_handle, (uint8_t*)&UART_lora_buffer_last_rx_byte, 1);
    }

    // Reception Error callback for GPS UART port
    if (huart->Instance == UART_gps_port_handle->Instance) {
        LOG_message(
            LOG_SYSTEM_GPS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "GPS UART Reception Error: %d", huart->ErrorCode
        );

        HAL_UART_Receive_IT(UART_gps_port_handle, (uint8_t*)&UART_gps_buffer_last_rx_byte, 1);
    }

    // Reception Error callback for CAMERA UART port
    if (huart->Instance == UART_camera_port_handle->Instance) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "CAMERA UART Reception Error: %d", huart->ErrorCode
        ); // TODO: CAMERA is not registered as a system in the logger yet, Telecommand system used instead

        HAL_UART_Receive_DMA(UART_camera_port_handle, (uint8_t*)&UART_camera_buffer_last_rx_byte, 1);
    }

    // Reception Error callback for EPS UART port
    if (huart->Instance == UART_eps_port_handle->Instance) {
        LOG_message(
            LOG_SYSTEM_EPS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "EPS UART Reception Error: %d", huart->ErrorCode
        );

        HAL_UART_Receive_IT(UART_eps_port_handle, (uint8_t*)&UART_eps_buffer_last_rx_byte, 1);
    }
}

void UART_init_uart_handlers(void) {
    // Enable the UART interrupt
    HAL_UART_Receive_IT(UART_telecommand_port_handle, (uint8_t*) &UART_telecommand_buffer_last_rx_byte, 1);
    HAL_UART_Receive_IT(UART_lora_port_handle, (uint8_t*) &UART_lora_buffer_last_rx_byte, 1);
    HAL_UART_Receive_IT(UART_gps_port_handle, (uint8_t*) &UART_gps_buffer_last_rx_byte, 1);
    HAL_UART_Receive_IT(UART_eps_port_handle, (uint8_t*) &UART_eps_buffer_last_rx_byte, 1);
    // TODO: Verify these when peripheral implementations are added
}