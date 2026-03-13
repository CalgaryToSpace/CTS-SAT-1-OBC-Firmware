
#ifndef INCLUDE_GUARD__TIMEKEEPING_TELECOMMAND_DEFS_H__
#define INCLUDE_GUARD__TIMEKEEPING_TELECOMMAND_DEFS_H__

#include <stdint.h>
#include "telecommand_exec/telecommand_definitions.h"

uint8_t TCMDEXEC_get_system_time(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_set_system_time(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_correct_system_time(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_set_eps_time_based_on_obc_time(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_set_obc_time_based_on_eps_time(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_set_obc_time_based_on_gnss_time(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_set_obc_time_based_on_gnss_pps(const char *args_str,
                        char *response_output_buf, uint16_t response_output_buf_len);

#endif /* INCLUDE_GUARD__FLASH_TELECOMMAND_DEFS_H__ */
