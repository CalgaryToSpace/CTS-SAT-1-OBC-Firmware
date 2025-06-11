
#ifndef INCLUDE_GUARD__TESTING_TELECOMMAND_DEFINITIONS_H
#define INCLUDE_GUARD__TESTING_TELECOMMAND_DEFINITIONS_H

#include "telecommand_exec/telecommand_types.h"
#include <stdint.h>

uint8_t TCMDEXEC_echo_back_args(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_echo_back_uint32_args(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_run_all_unit_tests(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_demo_blocking_delay(const char *args_str,char *response_output_buf, uint16_t response_output_buf_len);

#endif // INCLUDE_GUARD__TESTING_TELECOMMAND_DEFINITIONS_H
