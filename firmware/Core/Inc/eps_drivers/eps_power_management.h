#ifndef INCLUDE_GUARD__EPS_POWER_MANAGMENT_H__
#define INCLUDE_GUARD__EPS_POWER_MANAGMENT_H__

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "rtos_tasks/rtos_eps_tasks.h"
#include "eps_drivers/eps_types_to_json.h"
#include "eps_drivers/eps_types.h"
#include "eps_drivers/eps_commands.h"
#include "log/log.h"
#include "config/configuration.h"

extern uint16_t current_mA_threshhold[32];

uint8_t EPS_power_monitoring();

uint8_t EPS_log_pdu_json(const EPS_struct_pdu_housekeeping_data_eng_t *EPS_pdu_housekeeping_data_eng);   // Log the PDU housekeeping data in JSON format

void EPS_channel_managment(const EPS_struct_pdu_housekeeping_data_eng_t *EPS_pdu_housekeeping_data_eng);   // Monitor the power consumption of each channel and disable any channels that exceed a certain threshold

void EPS_CMD_power_managment_set_current_threshold(uint8_t channel, uint16_t threshold);   // Set the EPS power managment threshold for the current of a specific channel
#endif