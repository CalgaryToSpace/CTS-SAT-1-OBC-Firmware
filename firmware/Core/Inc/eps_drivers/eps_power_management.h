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

extern EPS_struct_pdu_housekeeping_data_eng_t prev_EPS_pdu_housekeeping_data_eng;

uint8_t EPS_power_monitoring();

uint8_t EPS_log_pdu_json(const EPS_struct_pdu_housekeeping_data_eng_t *EPS_pdu_housekeeping_data_eng);   // Log the PDU housekeeping data in JSON format

void EPS_channel_managment(const EPS_struct_pdu_housekeeping_data_eng_t *EPS_pdu_housekeeping_data_eng, const EPS_struct_pdu_housekeeping_data_eng_t *prev_EPS_pdu_housekeeping_data_eng);   // Monitor the power consumption of each channel and disable any channels that exceed a certain threshold

void EPS_power_managment_thresholds(uint32_t power_cW, uint32_t voltage_mV, uint32_t current_mA); // Set the thresholds for the power monitoring

#endif