
#ifndef __INCLUDE_GUARD__EPS_COMMANDS_H__
#define __INCLUDE_GUARD__EPS_COMMANDS_H__

#include "eps_drivers/eps_types.h"

#include <stdint.h>

uint8_t EPS_CMD_system_reset();
uint8_t EPS_CMD_no_operation();
uint8_t EPS_CMD_cancel_oper();
uint8_t EPS_CMD_watchdog();
uint8_t EPS_CMD_output_bus_group_on(uint16_t CH_BF, uint16_t CH_EXT_BF);
uint8_t EPS_CMD_output_bus_group_off(uint16_t CH_BF, uint16_t CH_EXT_BF);
uint8_t EPS_CMD_output_bus_group_state(uint16_t CH_BF, uint16_t CH_EXT_BF);
uint8_t EPS_CMD_output_bus_channel_on(uint8_t CH_IDX);
uint8_t EPS_CMD_output_bus_channel_off(uint8_t CH_IDX);
uint8_t EPS_CMD_switch_to_nominal_mode();
uint8_t EPS_CMD_switch_to_safety_mode();
uint8_t EPS_CMD_get_system_status(EPS_result_system_status_t* result_dest);
uint8_t EPS_CMD_get_pdu_overcurrent_fault_state(EPS_result_pdu_overcurrent_fault_state_t* result_dest);
uint8_t EPS_CMD_get_pbu_abf_placed_state(EPS_result_pbu_abf_placed_state_t* result_dest);
uint8_t EPS_CMD_get_pdu_housekeeping_data_eng(EPS_result_pdu_housekeeping_data_eng_t* result_dest);
uint8_t EPS_CMD_get_pdu_housekeeping_data_run_avg(EPS_result_pdu_housekeeping_data_eng_t* result_dest);
uint8_t EPS_CMD_get_pbu_housekeeping_data_eng(EPS_result_pbu_housekeeping_data_eng_t* result_dest);
uint8_t EPS_CMD_get_pbu_housekeeping_data_run_avg(EPS_result_pbu_housekeeping_data_eng_t* result_dest);
uint8_t EPS_CMD_get_pcu_housekeeping_data_eng(EPS_result_pcu_housekeeping_data_eng_t* result_dest);
uint8_t EPS_CMD_get_pcu_housekeeping_data_run_avg(EPS_result_pcu_housekeeping_data_eng_t* result_dest);
uint8_t EPS_CMD_get_configuration_parameter(uint16_t parameter_id, uint8_t parameter_value_dest[]);
uint8_t EPS_CMD_set_configuration_parameter(uint16_t parameter_id, uint8_t new_parameter_value);
uint8_t EPS_CMD_reset_configuration_parameter(uint16_t parameter_id);
uint8_t EPS_CMD_reset_configuration();
uint8_t EPS_CMD_load_configuration();
uint8_t EPS_CMD_save_configuration();
uint8_t EPS_CMD_get_piu_housekeeping_data_eng(EPS_result_piu_housekeeping_data_eng_t* result_dest);
uint8_t EPS_CMD_get_piu_housekeeping_data_run_avg(EPS_result_piu_housekeeping_data_eng_t* result_dest);
uint8_t EPS_CMD_correct_time(int32_t time_correction);
uint8_t EPS_CMD_zero_reset_cause_counters();

#endif /* __INCLUDE_GUARD__EPS_COMMANDS_H__ */
