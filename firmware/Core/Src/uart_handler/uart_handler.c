#include "uart_handler/uart_handler.h"
#include "debug_tools/debug_uart.h"
#include "mpi/mpi_command_handling.h"
#include "uart_handler/uart_error_tracking.h"
#include "camera/camera_capture.h"
#include "log/log.h"

#include "mpi/mpi_transceiver.h"
#include "main.h"

#include <string.h>

// Name the UART interfaces
UART_HandleTypeDef *UART_telecommand_port_handle = &hlpuart1;
UART_HandleTypeDef *UART_mpi_port_handle = &huart1;
UART_HandleTypeDef *UART_ax100_port_handle = &huart2;
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

// UART AX100 buffer
volatile uint16_t UART_ax100_buffer_write_idx = 0;          // extern
volatile uint32_t UART_ax100_last_write_time_ms = 0;        // extern
volatile uint8_t UART_ax100_buffer_last_rx_byte = 0;       // extern

// UART CAMERA buffer
// TODO: Configure with peripheral required specifications
const uint16_t UART_camera_dma_buffer_len = CAM_BYTES_TO_RECEIVE_PER_HALF_CALLBACK*2; // extern       // TODO: Set based on expected size requirements for reception
const uint16_t UART_camera_dma_buffer_len_half = CAM_BYTES_TO_RECEIVE_PER_HALF_CALLBACK; // extern       // TODO: Set based on expected size requirements for reception
volatile uint8_t UART_camera_dma_buffer[CAM_BYTES_TO_RECEIVE_PER_HALF_CALLBACK*2];   // extern       
volatile uint8_t UART_camera_pending_fs_write_half_1_buf[CAM_BYTES_TO_RECEIVE_PER_HALF_CALLBACK];   // extern       // half-size buffer for writing to LFS in half/cplt callback
volatile uint8_t UART_camera_pending_fs_write_half_2_buf[CAM_BYTES_TO_RECEIVE_PER_HALF_CALLBACK];   // extern       // half-size buffer for writing to LFS in half/cplt callback
volatile uint32_t UART_camera_last_write_time_ms = 0;       // extern
volatile CAMERA_uart_write_state_enum_t CAMERA_uart_half_1_state = CAMERA_UART_WRITE_STATE_IDLE; // extern
volatile CAMERA_uart_write_state_enum_t CAMERA_uart_half_2_state = CAMERA_UART_WRITE_STATE_IDLE; // extern

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

// UART MPI science data buffer
const uint16_t UART_mpi_data_rx_buffer_len = 40960; // extern 
volatile uint8_t UART_mpi_data_rx_buffer[40960]; // extern
const uint16_t MPI_active_data_median_buffer_len = 20480;
volatile uint8_t MPI_active_data_median_buffer[20480];


#define KISS_FEND  0xC0
#define KISS_FESC  0xDB
#define KISS_TFEND 0xDC
#define KISS_TFESC 0xDD


volatile AX100_kiss_frame_struct_t UART_AX100_kiss_frame_queue[AX100_MAX_KISS_FRAMES_IN_RX_QUEUE];
volatile uint8_t UART_AX100_kiss_frame_queue_head = 0;
volatile uint8_t UART_AX100_kiss_frame_queue_tail = 0;


/// Indicates whether we are currently inside a KISS frame.
/// Set to 1 after receiving KISS_FEND, and reset on frame completion or error.
static uint8_t kiss_in_frame = 0;

/// Indicates whether the previous byte was KISS_FESC, meaning the next byte
/// should be interpreted as an escaped control character (TFEND or TFESC).
static uint8_t kiss_escaped = 0;

/// Temporary buffer to hold the decoded contents of the current KISS frame.
/// Reset when a frame is completed or an error occurs (e.g. overflow or bad escape).
static uint8_t kiss_decode_buf[AX100_MAX_KISS_FRAME_SIZE_BYTES];

/// Current write index into kiss_decode_buf, tracking how many decoded bytes
/// have been accumulated in the current frame.
static uint16_t kiss_decode_len = 0;

static inline uint8_t kiss_queue_is_full(void) {
    return ((UART_AX100_kiss_frame_queue_head + 1) % AX100_MAX_KISS_FRAMES_IN_RX_QUEUE) == UART_AX100_kiss_frame_queue_tail;
}

static inline void kiss_enqueue_frame(const uint8_t *data, uint16_t len) {
    if (kiss_queue_is_full()) {
        // Tracking error
        UART_error_ax100_error_info.handler_buffer_full_error_count++;
        // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> KISS frame queue is full\n");
        return;
    }

    AX100_kiss_frame_struct_t *dst = (AX100_kiss_frame_struct_t*)&UART_AX100_kiss_frame_queue[UART_AX100_kiss_frame_queue_head];
    dst->len = len > AX100_MAX_KISS_FRAME_SIZE_BYTES ? AX100_MAX_KISS_FRAME_SIZE_BYTES : len;
    memcpy(dst->data, data, dst->len);
    UART_AX100_kiss_frame_queue_head = (UART_AX100_kiss_frame_queue_head + 1) % AX100_MAX_KISS_FRAMES_IN_RX_QUEUE;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    // This ISR function gets called every time a byte is received on the UART.

    if (huart->Instance == UART_telecommand_port_handle->Instance) {
        // THIS IS THE DEBUG/TELECOMMAND UART
        // DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> Telecommand\n");
        
        // Add the byte to the buffer.
        if (UART_telecommand_buffer_write_idx >= UART_telecommand_buffer_len) {
            // Tracking error
            UART_error_telecommand_error_info.handler_buffer_full_error_count++;
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
        UART_mpi_last_write_time_ms = HAL_GetTick();

        if (MPI_current_uart_rx_mode == MPI_RX_MODE_COMMAND_MODE) {
            // Command mode is blocking. Nothing to do here.
        }
        else if (MPI_current_uart_rx_mode == MPI_RX_MODE_SENSING_MODE) {
            // Store the second half into another buffer
            if (MPI_buffer_state == MPI_MEMORY_WRITE_STATUS_READY) {
                for(uint16_t i = (UART_mpi_data_rx_buffer_len/2); i < UART_mpi_data_rx_buffer_len; i++) {
                    MPI_active_data_median_buffer[i-(UART_mpi_data_rx_buffer_len/2)] = UART_mpi_data_rx_buffer[i];
                    UART_mpi_data_rx_buffer[i] = 0x00;
                }
                MPI_buffer_state = MPI_MEMORY_WRITE_STATUS_PENDING;
                // DEBUG_uart_print_str("COMPLETE - Received 4096 Bytes!\n");

            } else {
                DEBUG_uart_print_str("COMPLETE - *Assumed* 4096 Bytes are being lost!\n");
                MPI_science_data_bytes_lost += MPI_active_data_median_buffer_len;
            }
        }
        else {
            DEBUG_uart_print_str("Unhandled MPI Mode\n"); //TODO: HANDLE other MPI MODES
            DEBUG_uart_print_str("COMPLETE - Receiving some sort of MPI Data!\n");
        }

        UART_mpi_last_write_time_ms = HAL_GetTick();
    }

    else if (huart->Instance == UART_ax100_port_handle->Instance) {
        uint8_t rx = UART_ax100_buffer_last_rx_byte;
    
        if (rx == KISS_FEND) {
            if (kiss_in_frame && kiss_decode_len > 0) {
                // Queue completed frame
                kiss_enqueue_frame(kiss_decode_buf, kiss_decode_len);
            }
    
            // Start new frame
            kiss_in_frame = 1;
            kiss_escaped = 0;
            kiss_decode_len = 0;
        } else if (kiss_in_frame) {
            if (kiss_escaped) {
                if (rx == KISS_TFEND) {
                    rx = KISS_FEND;
                } else if (rx == KISS_TFESC) {
                    rx = KISS_FESC;
                } else {
                    // Invalid escape sequence - abort frame
                    kiss_in_frame = 0;
                    kiss_decode_len = 0;
                }
                kiss_escaped = 0;
            } else if (rx == KISS_FESC) {
                kiss_escaped = 1;
            } else {
                if (kiss_decode_len < AX100_MAX_KISS_FRAME_SIZE_BYTES) {
                    kiss_decode_buf[kiss_decode_len++] = rx;
                } else {
                    // Frame too long - discard
                    kiss_in_frame = 0;
                    kiss_decode_len = 0;
                    kiss_escaped = 0;
                }
            }
        }

        UART_ax100_last_write_time_ms = HAL_GetTick();
    
        HAL_UART_Receive_IT(UART_ax100_port_handle, (uint8_t*) &UART_ax100_buffer_last_rx_byte, 1);
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
            // Tracking error
            UART_error_eps_error_info.handler_buffer_full_error_count++;
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
                // Tracking error
                UART_error_gps_error_info.handler_buffer_full_error_count++;
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

    else if (huart->Instance == UART_camera_port_handle->Instance) {
        if (CAMERA_uart_half_2_state == CAMERA_UART_WRITE_STATE_HALF_FILLED_WAITING_FS_WRITE) {
            // Error: Data coming in too fast. Previous half not written yet.
            UART_error_camera_error_info.handler_buffer_full_error_count++;
            DEBUG_uart_print_str("Cam Full ISR() -> Data too fast\n");
        }

        // Volatile-safe memcpy.
        for (uint16_t i = UART_camera_dma_buffer_len_half; i < UART_camera_dma_buffer_len; i++) {
            UART_camera_pending_fs_write_half_2_buf[i-UART_camera_dma_buffer_len_half] = UART_camera_dma_buffer[i];

            // Clear the DMA buffer so that the len of the final read can be detected easily.
            UART_camera_dma_buffer[i] = 0;
        }

        UART_camera_last_write_time_ms = HAL_GetTick();
        CAMERA_uart_half_2_state = CAMERA_UART_WRITE_STATE_HALF_FILLED_WAITING_FS_WRITE;
        CAMERA_uart_half_1_state = CAMERA_UART_WRITE_STATE_HALF_FILLING;
    }

    else {
        // FIXME: add the rest (camera, MPI, maybe others)
        DEBUG_uart_print_str("HAL_UART_RxCpltCallback() -> unknown UART instance\n"); // FIXME: remove
    }
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart) {
    // DEBUG_uart_print_str("half call back\n");
    if (huart->Instance == UART_camera_port_handle->Instance) {
        if (CAMERA_uart_half_1_state == CAMERA_UART_WRITE_STATE_HALF_FILLED_WAITING_FS_WRITE) {
            // Error: Data coming in too fast. Previous half not written yet.
            UART_error_camera_error_info.handler_buffer_full_error_count++;
            DEBUG_uart_print_str("Cam Half ISR -> Data too fast\n");
        }

        for (uint16_t i = 0; i < UART_camera_dma_buffer_len_half; i++) {
            UART_camera_pending_fs_write_half_1_buf[i] = UART_camera_dma_buffer[i];

            // Clear the DMA buffer so that the len of the final read can be detected easily.
            UART_camera_dma_buffer[i] = 0;
        }
        UART_camera_last_write_time_ms = HAL_GetTick();

        CAMERA_uart_half_1_state = CAMERA_UART_WRITE_STATE_HALF_FILLED_WAITING_FS_WRITE;
        CAMERA_uart_half_2_state = CAMERA_UART_WRITE_STATE_HALF_FILLING;
    }

    else if (huart->Instance == UART_mpi_port_handle->Instance) {
        // DEBUG_uart_print_str("Half callback being called!");

        if (MPI_current_uart_rx_mode == MPI_RX_MODE_SENSING_MODE) {
            if (MPI_buffer_state == MPI_MEMORY_WRITE_STATUS_READY) {
                for (uint16_t i = 0; i < UART_mpi_data_rx_buffer_len/2; i++) {
                    MPI_active_data_median_buffer[i] = UART_mpi_data_rx_buffer[i];
                    UART_mpi_data_rx_buffer[i] = 0x00;
                }
                MPI_buffer_state = MPI_MEMORY_WRITE_STATUS_PENDING;
            }
            else {
                UART_error_mpi_error_info.handler_buffer_full_error_count++;
                MPI_science_data_bytes_lost += MPI_active_data_median_buffer_len;

                DEBUG_uart_print_str("MPI Half ISR - Data too fast!\n");
            }
        }
        else {
            DEBUG_uart_print_str("MPI Half ISR - Received MPI Data, rx_mode != SENSING though!\n");
        }
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

/// @brief Sets the UART interrupt state (enabled/disabled)
/// @param new_enabled 1: command sent, expecting data; 0: not expecting data
uint8_t CAMERA_set_expecting_data(uint8_t new_enabled) {
    if (new_enabled == 1) {
        CAMERA_uart_half_1_state = CAMERA_UART_WRITE_STATE_HALF_FILLING;
        
		const HAL_StatusTypeDef receive_status = HAL_UART_Receive_DMA(
            UART_camera_port_handle,(uint8_t*) &UART_camera_dma_buffer, UART_camera_dma_buffer_len
        );

        if (receive_status != HAL_OK) {
			return 3; // Error code: Failed UART reception
		}
        return 0;
    }

    HAL_UART_DMAStop(UART_camera_port_handle);
    return 0;
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    // Docs for error codes: https://community.st.com/t5/stm32-mcus-products/identifying-and-solving-uart-error/td-p/135754
    const uint32_t error_code = huart->ErrorCode;
    const uint32_t up_time_ms = HAL_GetTick();
    // if the error code is no error or it has been less than 100 ms since start up, ignore
    if ((error_code == HAL_UART_ERROR_NONE) || up_time_ms < 100) {
        return;
    }

    UART_track_error_from_isr(huart->Instance, error_code);

    // Reception Error callback for MPI UART port
    if (huart->Instance == UART_mpi_port_handle->Instance) {
        HAL_UART_Receive_DMA(UART_mpi_port_handle, (uint8_t*)&UART_mpi_last_rx_byte, 1);
    }

    // Reception Error callback for AX100 UART port
    if (huart->Instance == UART_ax100_port_handle->Instance) {
        HAL_UART_Receive_IT(UART_ax100_port_handle, (uint8_t*)&UART_ax100_buffer_last_rx_byte, 1);
    }

    // Reception Error callback for GPS UART port
    if (huart->Instance == UART_gps_port_handle->Instance) {
        if (UART_gps_uart_interrupt_enabled == 1) {
            HAL_UART_Receive_IT(UART_gps_port_handle, (uint8_t*)&UART_gps_buffer_last_rx_byte, 1);
        }
    }

    // Reception Error callback for CAMERA UART port
    if (huart->Instance == UART_camera_port_handle->Instance) {
        // Do not re-enable the interrupt here. Afraid of negative feedback loop.
    }

    // Reception Error callback for EPS UART port
    if (huart->Instance == UART_eps_port_handle->Instance) {
        // We trust the EPS. Always re-enable the interrupt.
        HAL_UART_Receive_IT(UART_eps_port_handle, (uint8_t*)&UART_eps_buffer_last_rx_byte, 1);
    }
}

void UART_init_uart_handlers(void) {
    // Enable the UART interrupt
    HAL_UART_Receive_IT(UART_telecommand_port_handle, (uint8_t*) &UART_telecommand_buffer_last_rx_byte, 1);
    HAL_UART_Receive_IT(UART_eps_port_handle, (uint8_t*) &UART_eps_buffer_last_rx_byte, 1);
    HAL_UART_Receive_IT(UART_ax100_port_handle, (uint8_t*) &UART_ax100_buffer_last_rx_byte, 1);

    // GPS is not initialized as always-listening. It is enabled by the GPS telecommands.
    // Reason: The GPS has a mode where it spams null bytes, which can lock up the entire system.
    // Thus, its interrupt is disabled by default.

    // TODO: Verify these when peripheral implementations are added
}