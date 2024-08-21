#ifndef INC_MPI_COMMAND_HANDLING_H_
#define INC_MPI_COMMAND_HANDLING_H_

#include <stdint.h>
#include <stddef.h>
#include "mpi/mpi_types.h"

extern MPI_rx_mode_t current_mpi_mode;        // Current mode under which the MPI is being operated

// Transmit and Receive timeout configurations
// const uint32_t tx_timeout_duration_ms;
// const uint32_t rx_timeout_duration_ms;

uint8_t MPI_send_telecommand_get_response(const uint8_t *bytes_to_send, const size_t bytes_to_send_len, uint8_t *MPI_rx_buffer, size_t MPI_rx_buffer_max_size, uint16_t *MPI_rx_buffer_len);
uint8_t MPI_validate_telecommand_response(const uint8_t *MPI_tx_buffer, uint8_t *MPI_rx_buffer, uint8_t const MPI_tx_buffer_size);

#endif /* INC_MPICOMMANDHANDLING_H_ */
