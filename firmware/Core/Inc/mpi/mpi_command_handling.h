#ifndef INC_MPI_COMMAND_HANDLING_H_
#define INC_MPI_COMMAND_HANDLING_H_

#include <stdint.h>
#include <stddef.h>

#include "mpi/mpi_types.h"

/// @brief Current mode under which the MPI is being operated.
extern volatile MPI_rx_mode_t MPI_current_uart_rx_mode;

uint8_t MPI_send_command_get_response(
    const uint8_t *bytes_to_send, const size_t bytes_to_send_len, uint8_t *MPI_rx_buffer,
    const size_t MPI_rx_buffer_max_size, uint16_t *MPI_rx_buffer_len
);

uint8_t MPI_validate_command_response(
    const uint8_t *MPI_tx_buffer, uint8_t *MPI_rx_buffer, const uint16_t MPI_tx_buffer_size
);

#endif /* INC_MPI_COMMAND_HANDLING_H_ */
