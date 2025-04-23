#ifndef INCLUDE_GUARD__EPS_CALCULATIONS
#define INCLUDE_GUARD__EPS_CALCULATIONS

#include "eps_drivers/eps_types.h"

#include <stdint.h>

float EPS_convert_battery_voltage_to_percent(EPS_battery_pack_datatype_eng_t battery);

int32_t EPS_calculate_total_fault_count(EPS_struct_pdu_overcurrent_fault_state_t *fault_state);

#endif // INCLUDE_GUARD__EPS_CALCULATIONS
