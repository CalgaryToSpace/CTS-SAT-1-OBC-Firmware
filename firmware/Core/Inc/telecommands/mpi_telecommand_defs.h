#ifndef INCLUDE_GUARD__MPI_TELECOMMAND_DEFS_H__
#define INCLUDE_GUARD__MPI_TELECOMMAND_DEFS_H__

#include <stdint.h>
#include "telecommand_exec/telecommand_definitions.h"

uint8_t TCMDEXEC_mpi_send_command_get_response_hex(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_mpi_demo_tx_to_mpi(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_mpi_demo_set_transceiver_mode(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_mpi_enable_active_mode(const char *args_str, 
    char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_mpi_disable_active_mode(const char *args_str, 
    char *response_output_buf, uint16_t response_output_buf_len);

#endif /* INCLUDE_GUARD__MPI_TELECOMMAND_DEFINITIONS_H__ */