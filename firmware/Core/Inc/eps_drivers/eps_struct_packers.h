
#ifndef INCLUDE_GUARD__EPS_STRUCT_PACKERS_H__
#define INCLUDE_GUARD__EPS_STRUCT_PACKERS_H__

#include <stdint.h>

#include "eps_drivers/eps_types.h"


void EPS_pack_eps_result_system_status(const uint8_t rx_buf[], EPS_struct_system_status_t *result_dest);
void EPS_pack_eps_result_pdu_overcurrent_fault_state(const uint8_t rx_buf[], EPS_struct_pdu_overcurrent_fault_state_t *result_dest);

void EPS_pack_eps_result_pdu_housekeeping_data_eng(const uint8_t rx_buf[], EPS_struct_pdu_housekeeping_data_eng_t *result_dest);
void EPS_pack_eps_result_pbu_housekeeping_data_eng(const uint8_t rx_buf[], EPS_struct_pbu_housekeeping_data_eng_t *result_dest);
void EPS_pack_eps_result_pcu_housekeeping_data_eng(const uint8_t rx_buf[], EPS_struct_pcu_housekeeping_data_eng_t *result_dest);
void EPS_pack_eps_result_piu_housekeeping_data_eng(const uint8_t rx_buf[], EPS_struct_piu_housekeeping_data_eng_t *result_dest);

#endif /* INCLUDE_GUARD__EPS_STRUCT_PACKERS_H__ */
