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

extern EPS_struct_pdu_housekeeping_data_eng_t prev_EPS_pdu_housekeeping_data_eng;

extern uint16_t power_cW_threshhold;
extern uint16_t voltage_mV_threshhold;
extern uint16_t current_mA_threshhold;

extern uint32_t disableable_channels;

uint8_t EPS_power_monitoring();

uint8_t EPS_log_pdu_json(const EPS_struct_pdu_housekeeping_data_eng_t *EPS_pdu_housekeeping_data_eng);   // Log the PDU housekeeping data in JSON format

void EPS_channel_managment(const EPS_struct_pdu_housekeeping_data_eng_t *EPS_pdu_housekeeping_data_eng, const EPS_struct_pdu_housekeeping_data_eng_t *prev_EPS_pdu_housekeeping_data_eng);   // Monitor the power consumption of each channel and disable any channels that exceed a certain threshold

void EPS_power_managment_thresholds(uint16_t power_cW, uint16_t voltage_mV, uint16_t current_mA); // Set the thresholds for the power monitoring

void EPS_change_disableable_channels(EPS_CHANNEL_enum_t channel); // Change the disableable state of a channel

void set_eps_monitoring_timer(uint64_t timer); // Set the timer for power monitoring

uint8_t set_watchdog_timer(uint64_t timer); // Set the timer for the watchdog

#endif