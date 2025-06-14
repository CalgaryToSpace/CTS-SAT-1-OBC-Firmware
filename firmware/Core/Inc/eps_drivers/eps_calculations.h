#ifndef INCLUDE_GUARD__EPS_CALCULATIONS
#define INCLUDE_GUARD__EPS_CALCULATIONS

#include "eps_drivers/eps_types.h"

#include <stdint.h>

float EPS_convert_battery_voltage_to_percent(EPS_battery_pack_datatype_eng_t battery);

int32_t EPS_calculate_total_fault_count(EPS_struct_pdu_overcurrent_fault_state_t *fault_state);

uint8_t EPS_calculate_overcurrent_difference(const EPS_struct_pdu_overcurrent_fault_state_t *before, const EPS_struct_pdu_overcurrent_fault_state_t *after, EPS_struct_pdu_overcurrent_fault_comparison_t *result);

#endif // INCLUDE_GUARD__EPS_CALCULATIONS
