#ifndef INC_MPI_COMMAND_HANDLING_H_
#define INC_MPI_COMMAND_HANDLING_H_

#include <stdint.h>
#include <stddef.h>

#include "mpi/mpi_types.h"
#include "littlefs/lfs.h"

static const uint8_t MPI_COMMAND_SUCCESS_RESPONSE_VALUE = 0xFE; // 0xFE = 254

/// @brief Current mode under which the MPI is being operated.
extern volatile MPI_rx_mode_t MPI_current_uart_rx_mode;

extern uint8_t MPI_science_data_file_is_open;
extern uint32_t MPI_science_data_bytes_lost;
extern lfs_file_t MPI_science_data_file_pointer;
extern uint32_t MPI_recording_start_uptime_ms;

typedef enum {
    MPI_REASON_FOR_STOPPING_TEMPERATURE_EXCEEDED,
    MPI_REASON_FOR_STOPPING_TELECOMMAND,
    MPI_REASON_FOR_STOPPING_MAX_TIME_EXCEEDED // TODO: Use this https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/488
} MPI_reason_for_stopping_active_mode;

uint8_t MPI_send_command_get_response(
    const uint8_t *bytes_to_send, const size_t bytes_to_send_len, 
    uint8_t *rx_buffer, const size_t rx_buffer_max_size, 
    uint16_t *rx_buffer_len
);

uint8_t MPI_enable_active_mode(const char output_file_path[]);
uint8_t MPI_disable_active_mode(MPI_reason_for_stopping_active_mode reason_for_stopping);
uint8_t MPI_validate_command_response(
    const uint8_t command_code, uint8_t *rx_buffer, const uint16_t rx_buffer_len
);

char *MPI_reason_for_stopping_active_mode_enum_to_str(MPI_reason_for_stopping_active_mode reason);

#endif /* INC_MPI_COMMAND_HANDLING_H_ */
