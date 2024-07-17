#ifndef INC_MPI_COMMAND_HANDLING_H_
#define INC_MPI_COMMAND_HANDLING_H_
#include "main.h"
#include "mpi/mpi_types.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/**
 * @brief Sends commandcode + params (If ANY) to the MPI as bytes
 * @param bytes_to_send MPI command code and paramerters needed to be sent to the MPI
 * @param bytes_to_send_length Number of bytes being sent to the MPI
 * @param mpi_command_response Echo from the
 * @return 0 - Success, 1 - MPI side failure, 2 - Transmit Error, 3 - Receive Error
 */
uint8_t MPI_send_telecommand_hex(const uint8_t *bytes_to_send, size_t bytes_to_send_len, uint8_t *mpi_cmd_response, size_t mpi_cmd_response_len);

#endif /* INC_MPICOMMANDHANDLING_H_ */
