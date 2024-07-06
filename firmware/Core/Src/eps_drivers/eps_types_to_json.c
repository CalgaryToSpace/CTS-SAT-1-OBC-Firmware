
#include "eps_drivers/eps_types_to_json.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>


/*
 ***** To create these eps_<type>_TO_json(...) functions, the following ChatGPT prompt was used: ***** 

=================================================================================================
Write a C code function with a signature like: 
uint8_t eps_<type>_TO_json(const eps_<type>_t *data, char json_output_str[], uint16_t json_output_str_len).

This function should convert the following struct type to a JSON string. The JSON string should be
written to the json_output_str buffer. The length of the JSON string should not exceed the
json_output_str_len. The JSON string should be null-terminated. Use sprintf to write to the buffer.
Do not include whitespaces in the JSON output. The function should return 1 on error.

Assume that for any unknown types, a similar _TO_json function exists and can be used.

typedef struct {
	int16_t voltage_raw;
	int16_t current_raw;
	int16_t power_raw;
} EPS_vpid_raw_t;

For the long array, use array_name[0], array_name[1], array_name[2], etc. verbosely instead of using iteration.

=================================================================================================

Note that modification are sometimes required, including:
0. In the prompt, rx_buf[5] is sometimes rx_buf[6].
1. Byte order check to be like the example given.
2. Function name.
3. Start/end conditions.
4. Update any multiplier, because LLMs are bad at guessing in advance how many bytes-per-loop are used.

*/

// TODO: determine how long each of these are, add to docs, add checks to each function at the start

uint8_t EPS_vpid_raw_TO_json(const EPS_vpid_raw_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 10) {
        return 1; // Error: Invalid input
    }

    int snprintf_ret = snprintf(
        json_output_str, json_output_str_len,
        "{\"voltage_raw\":%d,\"current_raw\":%d,\"power_raw\":%d}",
        data->voltage_raw, data->current_raw, data->power_raw);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    return 0; // Success
}

uint8_t EPS_vpid_eng_TO_json(const EPS_vpid_eng_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 10) {
        return 1; // Error: Invalid input
    }

    int snprintf_ret = snprintf(
        json_output_str, json_output_str_len,
        "{\"voltage_mV\":%d,\"current_mA\":%d,\"power_cW\":%d}",
        data->voltage_mV, data->current_mA, data->power_cW);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    return 0; // Success
}

uint8_t EPS_battery_pack_datatype_raw_TO_json(const EPS_battery_pack_datatype_raw_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 10) {
        return 1; // Error: Invalid input
    }

    char vip_bp_input_raw_json[100];
    const uint8_t json_ret_code = EPS_vpid_raw_TO_json(&(data->vip_bp_input_raw), vip_bp_input_raw_json, 100);
    if (json_ret_code != 0) {
        return json_ret_code; // Error: subfunction error
    }

    int snprintf_ret = snprintf(
        json_output_str, json_output_str_len,
        "{\"vip_bp_input_raw\":%s,\"bp_status_bitfield\":%d,\"cell_voltage_each_cell_raw\":[%d,%d,%d,%d],\"battery_temperature_each_sensor_raw\":[%d,%d,%d]}",
        vip_bp_input_raw_json,
        data->bp_status_bitfield,
        data->cell_voltage_each_cell_raw[0],
        data->cell_voltage_each_cell_raw[1],
        data->cell_voltage_each_cell_raw[2],
        data->cell_voltage_each_cell_raw[3],
        data->battery_temperature_each_sensor_raw[0],
        data->battery_temperature_each_sensor_raw[1],
        data->battery_temperature_each_sensor_raw[2]);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    return 0; // Success
}

uint8_t EPS_battery_pack_datatype_eng_TO_json(const EPS_battery_pack_datatype_eng_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 10) {
        return 1; // Error: Invalid input
    }

    char vip_bp_input_json[100];
    const uint8_t json_ret_code = EPS_vpid_eng_TO_json(&(data->vip_bp_input), vip_bp_input_json, 100);
    if (json_ret_code != 0) {
        return json_ret_code + 32; // Error: subfunction error
    }

    int snprintf_ret = snprintf(
        json_output_str, json_output_str_len,
        "{\"vip_bp_input\":%s,\"bp_status_bitfield\":%d,\"cell_voltage_each_cell_eng\":[%d,%d,%d,%d],\"battery_temperature_each_sensor_eng\":[%d,%d,%d]}",
        vip_bp_input_json,
        data->bp_status_bitfield,
        data->cell_voltage_each_cell_mV[0],
        data->cell_voltage_each_cell_mV[1],
        data->cell_voltage_each_cell_mV[2],
        data->cell_voltage_each_cell_mV[3],
        data->battery_temperature_each_sensor_cC[0],
        data->battery_temperature_each_sensor_cC[1],
        data->battery_temperature_each_sensor_cC[2]);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    return 0; // Success
}

uint8_t EPS_conditioning_channel_datatype_raw_TO_json(const EPS_conditioning_channel_datatype_raw_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 10) {
        return 1; // Error: Invalid input
    }

    char vip_cc_output_raw_json[100];
    const uint8_t json_ret_code = EPS_vpid_raw_TO_json(&(data->vip_cc_output_raw), vip_cc_output_raw_json, 100);
    if (json_ret_code != 0) {
        return json_ret_code; // Error: subfunction error
    }

    int snprintf_ret = snprintf(
        json_output_str, json_output_str_len,
        "{\"vip_cc_output_raw\":%s,\"volt_in_mppt_raw\":%d,\"curr_in_mppt_raw\":%d,\"volt_ou_mppt_raw\":%d,\"curr_ou_mppt_raw\":%d}",
        vip_cc_output_raw_json,
        data->volt_in_mppt_raw,
        data->curr_in_mppt_raw,
        data->volt_ou_mppt_raw,
        data->curr_ou_mppt_raw);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    return 0; // Success
}

uint8_t EPS_conditioning_channel_datatype_eng_TO_json(const EPS_conditioning_channel_datatype_eng_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 10) {
        return 1; // Error: Invalid input
    }

    char vip_cc_output_json[100];
    const uint8_t json_ret_code = EPS_vpid_eng_TO_json(&(data->vip_cc_output), vip_cc_output_json, 100);
    if (json_ret_code != 0) {
        return json_ret_code; // Error: subfunction error
    }

    int snprintf_ret = snprintf(
        json_output_str, json_output_str_len,
        "{\"vip_cc_output\":%s,\"volt_in_mppt_mV\":%d,\"curr_in_mppt_mA\":%d,\"volt_ou_mppt_mV\":%d,\"curr_ou_mppt_mA\":%d}",
        vip_cc_output_json,
        data->volt_in_mppt_mV,
        data->curr_in_mppt_mA,
        data->volt_ou_mppt_mV,
        data->curr_ou_mppt_mA);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    return 0; // Success
}

uint8_t EPS_conditioning_channel_short_datatype_raw_TO_json(const EPS_conditioning_channel_short_datatype_raw_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 10) {
        return 1; // Error: Invalid input
    }

    int snprintf_ret = snprintf(
        json_output_str, json_output_str_len,
        "{\"volt_in_mppt_raw\":%d,\"curr_in_mppt_raw\":%d,\"volt_ou_mppt_raw\":%d,\"curr_ou_mppt_raw\":%d}",
        data->volt_in_mppt_raw,
        data->curr_in_mppt_raw,
        data->volt_ou_mppt_raw,
        data->curr_ou_mppt_raw);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    return 0; // Success
}

uint8_t EPS_conditioning_channel_short_datatype_eng_TO_json(const EPS_conditioning_channel_short_datatype_eng_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 10) {
        return 1; // Error: Invalid input
    }

    int snprintf_ret = snprintf(
        json_output_str, json_output_str_len,
        "{\"volt_in_mppt_mV\":%d,\"curr_in_mppt_mA\":%d,\"volt_ou_mppt_mV\":%d,\"curr_ou_mppt_mA\":%d}",
        data->volt_in_mppt_mV,
        data->curr_in_mppt_mA,
        data->volt_ou_mppt_mV,
        data->curr_ou_mppt_mA);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    return 0; // Success
}


uint8_t EPS_result_system_status_TO_json(const EPS_result_system_status_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 10) {
        return 1; // Error: Invalid input
    }

    // TODO: add mode_str and reset_cause_str keys which decode the enum values to strings
    int snprintf_ret = snprintf(
        json_output_str, json_output_str_len,
        "{\"mode\":%d,\"config_changed_since_boot\":%d,\"reset_cause\":%d,\"uptime_sec\":%lu,\"error_code\":%u,\"rst_cnt_pwron\":%u,\"rst_cnt_wdg\":%u,\"rst_cnt_cmd\":%u,\"rst_cnt_mcu\":%u,\"rst_cnt_emlopo\":%u,\"time_since_prev_cmd_sec\":%u,\"unix_time_sec\":%lu,\"calendar_years_since_2000\":%u,\"calendar_month\":%u,\"calendar_day\":%u,\"calendar_hour\":%u,\"calendar_minute\":%u,\"calendar_second\":%u}",
        data->mode, data->config_changed_since_boot, data->reset_cause, data->uptime_sec,
        data->error_code, data->rst_cnt_pwron, data->rst_cnt_wdg, data->rst_cnt_cmd,
        data->rst_cnt_mcu, data->rst_cnt_emlopo, data->time_since_prev_cmd_sec,
        data->unix_time_sec, data->calendar_years_since_2000, data->calendar_month,
        data->calendar_day, data->calendar_hour, data->calendar_minute, data->calendar_second);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    return 0; // Success
}


uint8_t EPS_result_pdu_overcurrent_fault_state_TO_json(const EPS_result_pdu_overcurrent_fault_state_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 10) {
        return 1; // Error: Invalid input
    }

    // Write JSON string to buffer
    int snprintf_ret = snprintf(
        json_output_str, json_output_str_len,
        "{\"stat_ch_on_bitfield\":%u,\"stat_ch_ext_on_bitfield\":%u,\"stat_ch_overcurrent_fault_bitfield\":%u,\"stat_ch_ext_overcurrent_fault_bitfield\":%u,\"overcurrent_fault_count_each_channel\":[%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u]}",
        data->stat_ch_on_bitfield,
        data->stat_ch_ext_on_bitfield,
        data->stat_ch_overcurrent_fault_bitfield,
        data->stat_ch_ext_overcurrent_fault_bitfield,
        data->overcurrent_fault_count_each_channel[0], data->overcurrent_fault_count_each_channel[1],
        data->overcurrent_fault_count_each_channel[2], data->overcurrent_fault_count_each_channel[3],
        data->overcurrent_fault_count_each_channel[4], data->overcurrent_fault_count_each_channel[5],
        data->overcurrent_fault_count_each_channel[6], data->overcurrent_fault_count_each_channel[7],
        data->overcurrent_fault_count_each_channel[8], data->overcurrent_fault_count_each_channel[9],
        data->overcurrent_fault_count_each_channel[10], data->overcurrent_fault_count_each_channel[11],
        data->overcurrent_fault_count_each_channel[12], data->overcurrent_fault_count_each_channel[13],
        data->overcurrent_fault_count_each_channel[14], data->overcurrent_fault_count_each_channel[15],
        data->overcurrent_fault_count_each_channel[16], data->overcurrent_fault_count_each_channel[17],
        data->overcurrent_fault_count_each_channel[18], data->overcurrent_fault_count_each_channel[19],
        data->overcurrent_fault_count_each_channel[20], data->overcurrent_fault_count_each_channel[21],
        data->overcurrent_fault_count_each_channel[22], data->overcurrent_fault_count_each_channel[23],
        data->overcurrent_fault_count_each_channel[24], data->overcurrent_fault_count_each_channel[25],
        data->overcurrent_fault_count_each_channel[26], data->overcurrent_fault_count_each_channel[27],
        data->overcurrent_fault_count_each_channel[28], data->overcurrent_fault_count_each_channel[29],
        data->overcurrent_fault_count_each_channel[30], data->overcurrent_fault_count_each_channel[31]);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    return 0; // Success
}

uint8_t EPS_result_pbu_abf_placed_state_TO_json(const EPS_result_pbu_abf_placed_state_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 10) {
        return 1; // Error: Invalid input
    }

    // Write JSON string to buffer
    int snprintf_ret = snprintf(
        json_output_str, json_output_str_len,
        "{\"abf_placed_0\":%u,\"abf_placed_1\":%u,\"abf_placed_0_str\":%s,\"abf_placed_1_str\":%s}",
        data->abf_placed_0,
        data->abf_placed_1,
        (data->abf_placed_0 == EPS_ABF_PIN_APPLIED) ? "\"APPLIED\"" : "\"NOT_APPLIED\"",
        (data->abf_placed_1 == EPS_ABF_PIN_APPLIED) ? "\"APPLIED\"" : "\"NOT_APPLIED\"");

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    return 0; // Success
}

// Ebube's Addition ----------------------------
uint8_t EPS_result_pdu_housekeeping_data_raw_TO_json(const EPS_result_pdu_housekeeping_data_raw_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 10) {
        return 1; // Error: Invalid input
    }

    // Write JSON string to buffer
    int snprintf_ret = snprintf(
        json_output_str, json_output_str_len,
        "{\"voltage_internal_board_supply_raw\":%u,\"temperature_mcu_raw\":%u,\"vip_total_input_raw\":%s,\"stat_ch_on_bitfield\":%u,\"stat_ch_ext_on_bitfield\":%u, \"stat_ch_overcurrent_fault_bitfield\":%u,\"stat_ch_ext_overcurrent_fault_bitfield\":%u, \"vip_each_voltage_domain_raw\":[%s,%s,%s,%s,%s,%s,%s], \"vip_each_channel_raw\":[%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s]}",
        data->voltage_internal_board_supply_raw,
        data->temperature_mcu_raw,
        data->vip_total_input_raw,
        data->stat_ch_on_bitfield,
        data->stat_ch_ext_on_bitfield,
        data->stat_ch_overcurrent_fault_bitfield,
        data->stat_ch_ext_overcurrent_fault_bitfield,
        data->vip_each_voltage_domain_raw[0], data->vip_each_voltage_domain_raw[1],
        data->vip_each_voltage_domain_raw[2], data->vip_each_voltage_domain_raw[3],
        data->vip_each_voltage_domain_raw[4], data->vip_each_voltage_domain_raw[5],
        data->vip_each_channel_raw[0],        data->vip_each_channel_raw[1],
        data->vip_each_channel_raw[2],        data->vip_each_channel_raw[3],
        data->vip_each_channel_raw[4],        data->vip_each_channel_raw[5],
        data->vip_each_channel_raw[6],        data->vip_each_channel_raw[7],
        data->vip_each_channel_raw[8],        data->vip_each_channel_raw[9],
        data->vip_each_channel_raw[10],        data->vip_each_channel_raw[11],
        data->vip_each_channel_raw[12],        data->vip_each_channel_raw[13],
        data->vip_each_channel_raw[14],        data->vip_each_channel_raw[15],
        data->vip_each_channel_raw[16],        data->vip_each_channel_raw[17],
        data->vip_each_channel_raw[18],        data->vip_each_channel_raw[19],
        data->vip_each_channel_raw[20],        data->vip_each_channel_raw[21],
        data->vip_each_channel_raw[22],        data->vip_each_channel_raw[23],
        data->vip_each_channel_raw[24],        data->vip_each_channel_raw[25],
        data->vip_each_channel_raw[26],        data->vip_each_channel_raw[27],
        data->vip_each_channel_raw[28],        data->vip_each_channel_raw[29],
        data->vip_each_channel_raw[30],        data->vip_each_channel_raw[31]);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    return 0; // Success
}

uint8_t EPS_result_pdu_housekeeping_data_eng_TO_json(const EPS_result_pdu_housekeeping_data_eng_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 10) {
        return 1; // Error: Invalid input
    }

    // Write JSON string to buffer
    int snprintf_ret = snprintf(
        json_output_str, json_output_str_len,
        "{\"voltage_internal_board_supply_mV\":%u,\"temperature_mcu_cC\":%u,\"vip_total_input\":%s,\"stat_ch_on_bitfield\":%u,\"stat_ch_ext_bitfield\":%u,\"stat_ch_overcurrent_fault_bitfield\":%u,\"stat_ch_ext_overcurrent_fault_bitfield\":%u, \"vip_each_voltage_domain\":[%s,%s,%s,%s,%s,%s,%s], \"vip_each_channel\":[%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s]}",
        data->voltage_internal_board_supply_mV,
        data->temperature_mcu_cC,
        data->vip_total_input,
        data->stat_ch_on_bitfield,
        data->stat_ch_ext_on_bitfield,
        data->stat_ch_overcurrent_fault_bitfield,
        data->stat_ch_ext_overcurrent_fault_bitfield,
        data->vip_each_voltage_domain[0], data->vip_each_voltage_domain[1],
        data->vip_each_voltage_domain[2], data->vip_each_voltage_domain[3],
        data->vip_each_voltage_domain[4], data->vip_each_voltage_domain[5],
        data->vip_each_channel[0],        data->vip_each_channel[1],
        data->vip_each_channel[2],        data->vip_each_channel[3],
        data->vip_each_channel[4],        data->vip_each_channel[5],
        data->vip_each_channel[6],        data->vip_each_channel[7],
        data->vip_each_channel[8],        data->vip_each_channel[9],
        data->vip_each_channel[10],        data->vip_each_channel[11],
        data->vip_each_channel[12],        data->vip_each_channel[13],
        data->vip_each_channel[14],        data->vip_each_channel[15],
        data->vip_each_channel[16],        data->vip_each_channel[17],
        data->vip_each_channel[18],        data->vip_each_channel[19],
        data->vip_each_channel[20],        data->vip_each_channel[21],
        data->vip_each_channel[22],        data->vip_each_channel[23],
        data->vip_each_channel[24],        data->vip_each_channel[25],
        data->vip_each_channel[26],        data->vip_each_channel[27],
        data->vip_each_channel[28],        data->vip_each_channel[29],
        data->vip_each_channel[30],        data->vip_each_channel[31]);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    return 0; // Success
}

uint8_t EPS_result_pbu_housekeeping_data_raw_TO_json(const EPS_result_pbu_housekeeping_data_raw_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 10) {
        return 1; // Error: Invalid input
    }

    // Write JSON string to buffer
    int snprintf_ret = snprintf(
        json_output_str, json_output_str_len,
        "{\"voltage_internal_board_supply_raw\":%u,\"temperature_mcu_raw\":%u,\"vip_total_input_raw\":%s,\"battery_pack_status_bitfield\":%u, \"battery_pack_info_each_pack_raw\":[%s,%s,%s]}",
        data->voltage_internal_board_supply_raw,
        data->temperature_mcu_raw,
        data->vip_total_input_raw,
        data->battery_pack_status_bitfield,
        data->battery_pack_info_each_pack_raw[0], data->battery_pack_info_each_pack_raw[1],
        data->battery_pack_info_each_pack_raw[2]);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    return 0; // Success
}

uint8_t EPS_result_pbu_housekeeping_data_eng_TO_json(const EPS_result_pbu_housekeeping_data_eng_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 10) {
        return 1; // Error: Invalid input
    }

    // Write JSON string to buffer
    int snprintf_ret = snprintf(
        json_output_str, json_output_str_len,
        "{\"voltage_internal_board_supply_mV\":%u,\"temperature_mcu_cC\":%u,\"vip_total_input\":%s,\"battery_pack_status_bitfield\":%u, \"battery_pack_info_each_pack\":[%s,%s,%s]}",
        data->voltage_internal_board_supply_mV,
        data->temperature_mcu_cC,
        data->vip_total_input,
        data->battery_pack_status_bitfield,
        data->battery_pack_info_each_pack[0], data->battery_pack_info_each_pack[1],
        data->battery_pack_info_each_pack[2]);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    return 0; // Success
}

uint8_t EPS_result_pcu_housekeeping_data_raw_TO_json(const EPS_result_pcu_housekeeping_data_raw_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 10) {
        return 1; // Error: Invalid input
    }

    // Write JSON string to buffer
    int snprintf_ret = snprintf(
        json_output_str, json_output_str_len,
        "{\"voltage_internal_board_supply_raw\":%u,\"temperature_mcu_raw\":%u,\"vip_total_input_raw\":%s,\"conditioning_channel_info_each_channel_raw\":[%s,%s,%s,%s]}",
        data->voltage_internal_board_supply_raw,
        data->temperature_mcu_raw,
        data->vip_total_input_raw,
        data->conditioning_channel_info_each_channel_raw[0], data->conditioning_channel_info_each_channel_raw[1],
        data->conditioning_channel_info_each_channel_raw[2], data->conditioning_channel_info_each_channel_raw[3]);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    return 0; // Success
}

uint8_t EPS_result_pcu_housekeeping_data_eng_TO_json(const EPS_result_pcu_housekeeping_data_eng_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 10) {
        return 1; // Error: Invalid input
    }

    // Write JSON string to buffer
    int snprintf_ret = snprintf(
        json_output_str, json_output_str_len,
        "{\"voltage_internal_board_supply_mV\":%u,\"temperature_mcu_cC\":%u,\"vip_total_input\":%s,\"conditioning_channel_info_each_channel\":[%s,%s,%s,%s]}",
        data->voltage_internal_board_supply_mV,
        data->temperature_mcu_cC,
        data->vip_total_input,
        data->conditioning_channel_info_each_channel[0], data->conditioning_channel_info_each_channel[1],
        data->conditioning_channel_info_each_channel[2], data->conditioning_channel_info_each_channel[3]);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    return 0; // Success
}

uint8_t EPS_result_piu_housekeeping_data_raw_TO_json(const EPS_result_piu_housekeeping_data_raw_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 10) {
        return 1; // Error: Invalid input
    }

    // Write JSON string to buffer
    int snprintf_ret = snprintf(
        json_output_str, json_output_str_len,
        // TODO: Fix the text on this!!!!!!
        "{\"voltage_internal_board_supply_raw\":%u,\"temperature_mcu_raw\":%u,\"vip_dist_input_raw\":%s,\"vip_batt_input_raw\":%s, \"stat_ch_on_bitfield\":%u, \"stat_ch_overcurrent_fault_bitfield\":%u,\"battery_status_bitfield\":%u,\"battery_temp2_raw\":%u, \"battery_temp3_raw\":%u, \"vd0_voltage_raw\":%u, \"vd1_voltage_raw\":%u, \"vd2_voltage_raw\":%u, \"vip_each_channel_raw\":[%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s], \"conditioning_channel_info_each_channel_raw\":[%s,%s,%s,%s,%s], \"stat_ch_ext_on_bitfield\":%u, \"stat_ch_ext_overcurrent_fault_bitfield\":%u}",
        data->voltage_internal_board_supply_raw,
        data->temperature_mcu_raw,
        data->vip_dist_input_raw,
        data->vip_batt_input_raw,
        data->stat_ch_on_bitfield,
        data->stat_ch_overcurrent_fault_bitfield,
        data->battery_status_bitfield,
        data->battery_temp2_raw,
        data->battery_temp3_raw,

        data->vd0_voltage_raw,
        data->vd1_voltage_raw,
        data->vd2_voltage_raw,


        data->vip_each_channel_raw[0], data->vip_each_channel_raw[1], 
        data->vip_each_channel_raw[2], data->vip_each_channel_raw[3],
        data->vip_each_channel_raw[4], data->vip_each_channel_raw[5], 
        data->vip_each_channel_raw[6], data->vip_each_channel_raw[7],
        data->vip_each_channel_raw[8], data->vip_each_channel_raw[9], 
        data->vip_each_channel_raw[10], data->vip_each_channel_raw[11],
        data->vip_each_channel_raw[12], data->vip_each_channel_raw[13], 
        data->vip_each_channel_raw[14], data->vip_each_channel_raw[15],
        data->vip_each_channel_raw[15], data->vip_each_channel_raw[16], 
        data->vip_each_channel_raw[17], data->vip_each_channel_raw[18],
        data->vip_each_channel_raw[19], data->vip_each_channel_raw[20], 
        data->vip_each_channel_raw[21], data->vip_each_channel_raw[22],
        data->vip_each_channel_raw[23], data->vip_each_channel_raw[24], 
        data->vip_each_channel_raw[25], data->vip_each_channel_raw[26],
        data->vip_each_channel_raw[27], data->vip_each_channel_raw[28],
         data->vip_each_channel_raw[29], data->vip_each_channel_raw[30],
        data->vip_each_channel_raw[31],

        data->conditioning_channel_info_each_channel_raw[0], data->conditioning_channel_info_each_channel_raw[1],
        data->conditioning_channel_info_each_channel_raw[2], data->conditioning_channel_info_each_channel_raw[3],
        data->conditioning_channel_info_each_channel_raw[4],

        data->stat_ch_ext_on_bitfield,
        data->stat_ch_ext_overcurrent_fault_bitfield
        );


    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    return 0; // Success
}

uint8_t EPS_result_piu_housekeeping_data_eng_TO_json(const EPS_result_piu_housekeeping_data_eng_t *data, char json_output_str[], uint16_t json_output_str_len) {
    if (data == NULL || json_output_str == NULL || json_output_str_len < 10) {
        return 1; // Error: Invalid input
    }

    // Write JSON string to buffer
    int snprintf_ret = snprintf(
        json_output_str, json_output_str_len,
        "{\"voltage_internal_board_supply_mV\":%u,\"temperature_mcu_cC\":%u,\"vip_dist_input\":%s,\"vip_batt_input\":%s, \"stat_ch_on_bitfield\":%u, \"stat_ch_overcurrent_fault_bitfield\":%u,\"battery_status_bitfield\":%u,\"battery_temp2_cC\":%u, \"battery_temp3_cC\":%u, \"vd0_voltage_mV\":%u, \"vd1_voltage_mV\":%u, \"vd2_voltage_mV\":%u, \"vip_each_channel\":[%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s], \"conditioning_channel_info_each_channel\":[%s,%s,%s,%s,%s], \"stat_ch_ext_on_bitfield\":%u, \"stat_ch_ext_overcurrent_fault_bitfield\":%u}",
        data->voltage_internal_board_supply_mV,
        data->temperature_mcu_cC,
        data->vip_dist_input,
        data->vip_batt_input,
        data->stat_ch_on_bitfield,
        data->stat_ch_overcurrent_fault_bitfield,
        data->battery_status_bitfield,
        data->battery_temp2_cC,
        data->battery_temp3_cC,

        data->vd0_voltage_mV,
        data->vd1_voltage_mV,
        data->vd2_voltage_mV,


        data->vip_each_channel[0], data->vip_each_channel[1], 
        data->vip_each_channel[2], data->vip_each_channel[3],
        data->vip_each_channel[4], data->vip_each_channel[5], 
        data->vip_each_channel[6], data->vip_each_channel[7],
        data->vip_each_channel[8], data->vip_each_channel[9], 
        data->vip_each_channel[10], data->vip_each_channel[11],
        data->vip_each_channel[12], data->vip_each_channel[13], 
        data->vip_each_channel[14], data->vip_each_channel[15],
        data->vip_each_channel[15], data->vip_each_channel[16], 
        data->vip_each_channel[17], data->vip_each_channel[18],
        data->vip_each_channel[19], data->vip_each_channel[20], 
        data->vip_each_channel[21], data->vip_each_channel[22],
        data->vip_each_channel[23], data->vip_each_channel[24], 
        data->vip_each_channel[25], data->vip_each_channel[26],
        data->vip_each_channel[27], data->vip_each_channel[28],
        data->vip_each_channel[29], data->vip_each_channel[30],
        data->vip_each_channel[31],

        data->conditioning_channel_info_each_channel[0], data->conditioning_channel_info_each_channel[1],
        data->conditioning_channel_info_each_channel[2], data->conditioning_channel_info_each_channel[3],
        data->conditioning_channel_info_each_channel[4],

        data->stat_ch_ext_on_bitfield,
        data->stat_ch_ext_overcurrent_fault_bitfield
        );


    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_len) {
        return 3; // Error: string buffer too short
    }
    return 0; // Success
}






