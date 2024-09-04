#ifndef __INCLUDE_GUARD__MPI_TELECOMMAND_DEFS_H__
#define __INCLUDE_GUARD__MPI_TELECOMMAND_DEFS_H__

#include <stdint.h>
#include "telecommands/telecommand_definitions.h"

uint8_t TCMDEXEC_mpi_send_command_hex(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len);

#endif /* __INCLUDE_GUARD__MPI_TELECOMMAND_DEFINITIONS_H__ */