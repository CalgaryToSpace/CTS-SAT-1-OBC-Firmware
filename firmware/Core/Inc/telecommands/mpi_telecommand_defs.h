#ifndef INCLUDE_GUARD__MPI_TELECOMMAND_DEFS_H__
#define INCLUDE_GUARD__MPI_TELECOMMAND_DEFS_H__

#include <stdint.h>
#include "telecommand_exec/telecommand_definitions.h"

uint8_t TCMDEXEC_mpi_send_command_hex(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_mpi_demo_tx_to_mpi(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
);


#endif /* INCLUDE_GUARD__MPI_TELECOMMAND_DEFINITIONS_H__ */