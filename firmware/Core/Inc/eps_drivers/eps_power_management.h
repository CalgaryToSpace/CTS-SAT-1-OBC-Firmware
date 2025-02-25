#ifndef INCLUDE_GUARD__EPS_POWER_MANAGEMENT_H__
#define INCLUDE_GUARD__EPS_POWER_MANAGEMENT_H__

#include "eps_drivers/eps_types.h"

uint8_t EPS_power_monitoring();

uint8_t EPS_log_pdu_json(const EPS_struct_pdu_housekeeping_data_eng_t *EPS_pdu_housekeeping_data_eng);   // Log the PDU housekeeping data in JSON format

void EPS_channel_management(const EPS_struct_pdu_housekeeping_data_eng_t *EPS_pdu_housekeeping_data_eng);   // Monitor the power consumption of each channel and disable any channels that exceed a certain threshold

void EPS_CMD_power_management_set_current_threshold(uint8_t channel, uint16_t threshold);   // Set the EPS power management threshold for the current of a specific channel
#endif