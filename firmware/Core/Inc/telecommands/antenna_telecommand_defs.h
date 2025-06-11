#ifndef INCLUDE_GUARD_ANTENNA_TELECOMMAND_DEFS_H__
#define INCLUDE_GUARD_ANTENNA_TELECOMMAND_DEFS_H__

#include <stdint.h>
#include "telecommand_exec/telecommand_definitions.h"

uint8_t TCMDEXEC_ant_reset(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ant_arm_antenna_system(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ant_disarm_antenna_system(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);


uint8_t TCMDEXEC_ant_deploy_antenna(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ant_start_automated_antenna_deployment(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ant_deploy_antenna_with_override(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ant_cancel_deployment_system_activation(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);


uint8_t TCMDEXEC_ant_measure_temp(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ant_report_deployment_status(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ant_report_antenna_deployment_activation_count(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ant_report_antenna_deployment_activation_time(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

#endif /* INCLUDE_GUARD_ANTENNA_TELECOMMAND_DEFS_H__ */