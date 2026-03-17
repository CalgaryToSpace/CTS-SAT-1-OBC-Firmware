#ifndef INCLUDE_GUARD__EPS_POWER_MANAGEMENT_H__
#define INCLUDE_GUARD__EPS_POWER_MANAGEMENT_H__

#include "eps_drivers/eps_types.h"

extern uint16_t EPS_current_mA_threshhold[32];

uint8_t EPS_monitor_and_disable_overcurrent_channels();

void EPS_channel_management(const EPS_struct_pdu_housekeeping_data_eng_t *EPS_pdu_housekeeping_data_eng);

void EPS_CMD_power_management_set_current_threshold(uint8_t channel, uint16_t threshold);

#endif
