#ifndef INC_MPI_COMMAND_HANDLING_H_
#define INC_MPI_COMMAND_HANDLING_H_

#include <stdint.h>
#include <stddef.h>
#include "stm32l4xx_hal.h"

extern enum Mpi_Mode current_mpi_mode;
uint8_t MPI_send_telecommand_hex(const uint8_t *bytes_to_send, size_t bytes_to_send_len, uint8_t mpi_response[], size_t mpi_response_size);

#endif /* INC_MPICOMMANDHANDLING_H_ */
