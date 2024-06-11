
#include <string.h>
#include <stdio.h>

#include "debug_tools/debug_uart.h"
#include "eps_drivers/eps_debug_tools.h"
#include "eps_drivers/eps_commands.h"


void EPS_debug_uart_print_system_status(EPS_result_system_status_t* system_status) {
	char msg1[365];
	sprintf(
	    msg1,
	    "Mode: %d, Configuration Changed?: %d, Reset cause: %d, Uptime: %lu sec, Error Code: %d, rst_cnt_pwron: %u, rst_cnt_wdg: %d, rst_cnt_cmd: %d, rst_cnt_mcu: %d, rst_cnt_emlopo: %d, time_since_prev_cmd: %d sec, Unix time: %lu sec, Unix year: %d, Unix month: %d, Unix day: %d, Unix hour: %d, Unix minute: %d, Unix second: %d\n",
	    system_status->mode, system_status->config_changed_since_boot, system_status->reset_cause,
        system_status->uptime_sec, system_status->error_code,
        system_status->rst_cnt_pwron, system_status->rst_cnt_wdg, system_status->rst_cnt_cmd,
        system_status->rst_cnt_mcu, system_status->rst_cnt_emlopo,
        
        system_status->time_since_prev_cmd_sec, system_status->unix_time_sec,
        (system_status->calendar_years_since_2000 + 2000), system_status->calendar_month, system_status->calendar_day,
        system_status->calendar_hour, system_status->calendar_minute, system_status->calendar_second
	);

	DEBUG_uart_print_str(msg1);
}

void EPS_result_pdu_housekeeping_data_eng_to_json(const EPS_result_pdu_housekeeping_data_eng_t *data, char json_output_str[]) {
    // json_output_str must be >= 4096 bytes

    sprintf(json_output_str, "{\n");
    sprintf(json_output_str + strlen(json_output_str), "    voltage_internal_board_supply_mV: %u,\n", data->voltage_internal_board_supply_mV);
    sprintf(json_output_str + strlen(json_output_str), "    temperature_mcu_cC: %u,\n", data->temperature_mcu_cC);
    sprintf(json_output_str + strlen(json_output_str), "    vip_total_input: { voltage_mV: %d, current_mA: %d, power_cW: %d },\n", 
            data->vip_total_input.voltage_mV, data->vip_total_input.current_mA, data->vip_total_input.power_cW);
    sprintf(json_output_str + strlen(json_output_str), "    stat_ch_on_bitfield: %u,\n", data->stat_ch_on_bitfield);
    sprintf(json_output_str + strlen(json_output_str), "    stat_ch_ext_on_bitfield: %u,\n", data->stat_ch_ext_on_bitfield);
    sprintf(json_output_str + strlen(json_output_str), "    stat_ch_overcurrent_fault_bitfield: %u,\n", data->stat_ch_overcurrent_fault_bitfield);
    sprintf(json_output_str + strlen(json_output_str), "    stat_ch_ext_overcurrent_fault_bitfield: %u,\n", data->stat_ch_ext_overcurrent_fault_bitfield);

    sprintf(json_output_str + strlen(json_output_str), "    vip_each_voltage_domain: [\n");
    for (int i = 0; i < 7; i++) {
        sprintf(json_output_str + strlen(json_output_str), "        { voltage_mV: %d, current_mA: %d, power_cW: %d },\n", 
                data->vip_each_voltage_domain[i].voltage_mV,
                data->vip_each_voltage_domain[i].current_mA, 
                data->vip_each_voltage_domain[i].power_cW);
    }
    sprintf(json_output_str + strlen(json_output_str), "    ],\n");

    sprintf(json_output_str + strlen(json_output_str), "    vip_each_channel: [\n");
    for (int i = 0; i < 32; i++) {
        sprintf(json_output_str + strlen(json_output_str), "        { voltage_mV: %d, current_mA: %d, power_cW: %d },\n", 
                data->vip_each_channel[i].voltage_mV,
                data->vip_each_channel[i].current_mA, 
                data->vip_each_channel[i].power_cW);
    }
    sprintf(json_output_str + strlen(json_output_str), "    ],\n");
    sprintf(json_output_str + strlen(json_output_str), "    json_output_str_length_approx: %d\n", strlen(json_output_str)+40);
    sprintf(json_output_str + strlen(json_output_str), "}\n");
}
