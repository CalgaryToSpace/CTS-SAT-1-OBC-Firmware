
#include "eps_drivers/eps_types_to_json.h"
#include "eps_drivers/eps_channel_control.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>


// TODO: Determine how long each of these are, add to docs, add checks to each function at the start.


uint8_t EPS_vpid_eng_TO_json(const EPS_vpid_eng_t *data, char json_output_str[], uint16_t json_output_str_size) {
    if (data == NULL || json_output_str == NULL || json_output_str_size < 10) {
        return 1; // Error: Invalid input
    }

    int snprintf_ret = snprintf(
        json_output_str, json_output_str_size,
        "{\"mV\":%d,\"mA\":%d,\"cW\":%d}",
        data->voltage_mV, data->current_mA, data->power_cW);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_size) {
        return 3; // Error: json_output_str too short
    }
    return 0; // Success
}


uint8_t EPS_battery_pack_datatype_eng_TO_json(
    const EPS_battery_pack_datatype_eng_t *data,
    char json_output_str[],
    uint16_t json_output_str_size,
    uint8_t enable_show_unsupported_fields // 0=hide; normal option
) {
    if (data == NULL || json_output_str == NULL || json_output_str_size < 10) {
        return 1; // Error: Invalid input
    }

    char vip_bp_input_json[100];
    const uint8_t json_ret_code = EPS_vpid_eng_TO_json(&(data->vip_bp_input), vip_bp_input_json, 100);
    if (json_ret_code != 0) {
        return json_ret_code + 32; // Error: subfunction error
    }

    int snprintf_ret;
    if (enable_show_unsupported_fields) {
        snprintf_ret = snprintf(
            json_output_str, json_output_str_size,
            "{\"vip_bp_input\":%s,\"bp_status_bitfield\":%d,\"cell_voltage_each_cell_mV\":[%d,%d,%d,%d],\"battery_temperature_each_sensor_cC\":[%d,%d,%d]}",
            vip_bp_input_json,
            data->bp_status_bitfield,
            data->cell_voltage_each_cell_mV[0],
            data->cell_voltage_each_cell_mV[1],
            data->cell_voltage_each_cell_mV[2],
            data->cell_voltage_each_cell_mV[3],
            data->battery_temperature_each_sensor_cC[0],
            data->battery_temperature_each_sensor_cC[1],
            data->battery_temperature_each_sensor_cC[2]
        );
    }
    else {
        // Hide battery_temperature_each_sensor_cC[0], and remove the "cell_voltage_each_cell_mV" key entirely.
        snprintf_ret = snprintf(
            json_output_str, json_output_str_size,
            "{\"vip_bp_input\":%s,\"bp_status_bitfield\":%d,\"battery_temperature_each_sensor_cC\":[%d,%d]}",
            vip_bp_input_json,
            data->bp_status_bitfield,
            data->battery_temperature_each_sensor_cC[1],
            data->battery_temperature_each_sensor_cC[2]
        );
    }

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_size) {
        return 3; // Error: json_output_str too short
    }
    return 0; // Success
}


uint8_t EPS_conditioning_channel_datatype_eng_TO_json(const EPS_conditioning_channel_datatype_eng_t *data, char json_output_str[], uint16_t json_output_str_size) {
    if (data == NULL || json_output_str == NULL || json_output_str_size < 10) {
        return 1; // Error: Invalid input
    }

    char vip_cc_output_json[100];
    const uint8_t json_ret_code = EPS_vpid_eng_TO_json(&(data->vip_cc_output), vip_cc_output_json, 100);
    if (json_ret_code != 0) {
        return json_ret_code; // Error: subfunction error
    }

    int snprintf_ret = snprintf(
        json_output_str, json_output_str_size,
        "{\"vip_cc_output\":%s,\"volt_in_mppt_mV\":%d,\"curr_in_mppt_mA\":%d,\"volt_ou_mppt_mV\":%d,\"curr_ou_mppt_mA\":%d}",
        vip_cc_output_json,
        data->volt_in_mppt_mV,
        data->curr_in_mppt_mA,
        data->volt_ou_mppt_mV,
        data->curr_ou_mppt_mA);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_size) {
        return 3; // Error: json_output_str too short
    }
    return 0; // Success
}


uint8_t EPS_conditioning_channel_short_datatype_eng_TO_json(const EPS_conditioning_channel_short_datatype_eng_t *data, char json_output_str[], uint16_t json_output_str_size) {
    if (data == NULL || json_output_str == NULL || json_output_str_size < 10) {
        return 1; // Error: Invalid input
    }

    int snprintf_ret = snprintf(
        json_output_str, json_output_str_size,
        "{\"volt_in_mppt_mV\":%d,\"curr_in_mppt_mA\":%d,\"volt_ou_mppt_mV\":%d,\"curr_ou_mppt_mA\":%d}",
        data->volt_in_mppt_mV,
        data->curr_in_mppt_mA,
        data->volt_ou_mppt_mV,
        data->curr_ou_mppt_mA);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_size) {
        return 3; // Error: json_output_str too short
    }
    return 0; // Success
}


uint8_t EPS_struct_system_status_TO_json(const EPS_struct_system_status_t *data, char json_output_str[], uint16_t json_output_str_size) {
    if (data == NULL || json_output_str == NULL || json_output_str_size < 10) {
        return 1; // Error: Invalid input
    }

    // TODO: Add mode_str and reset_cause_str keys which decode the enum values to strings.
    const int snprintf_ret = snprintf(
        json_output_str, json_output_str_size,
        "{\"mode\":%d,\"config_changed_since_boot\":%d,\"reset_cause\":%d,\"uptime_sec\":%lu,\"error_code\":%u,\"rst_cnt_pwron\":%u,\"rst_cnt_wdg\":%u,\"rst_cnt_cmd\":%u,\"rst_cnt_mcu\":%u,\"rst_cnt_emlopo\":%u,\"time_since_prev_cmd_sec\":%u,\"unix_time_sec\":%lu,\"calendar_years_since_2000\":%u,\"calendar_month\":%u,\"calendar_day\":%u,\"calendar_hour\":%u,\"calendar_minute\":%u,\"calendar_second\":%u}",
        data->mode, data->config_changed_since_boot, data->reset_cause, data->uptime_sec,
        data->error_code, data->rst_cnt_pwron, data->rst_cnt_wdg, data->rst_cnt_cmd,
        data->rst_cnt_mcu, data->rst_cnt_emlopo, data->time_since_prev_cmd_sec,
        data->unix_time_sec, data->calendar_years_since_2000, data->calendar_month,
        data->calendar_day, data->calendar_hour, data->calendar_minute, data->calendar_second);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_size) {
        return 3; // Error: json_output_str too short
    }
    return 0; // Success
}


uint8_t EPS_struct_pdu_overcurrent_fault_state_TO_json(const EPS_struct_pdu_overcurrent_fault_state_t *data, char json_output_str[], uint16_t json_output_str_size) {
    if (data == NULL || json_output_str == NULL || json_output_str_size < 10) {
        return 1; // Error: Invalid input
    }

    // Channels 0-16 (inclusive) are present on our model.
    // Channel 17-31 are not present on our model.
    // Sum them up in a single value, which should always be 0.
    uint32_t overcurrent_fault_count_channel_17_plus = 0;
    for (uint8_t ch = 17; ch <= 31; ch++) {
        overcurrent_fault_count_channel_17_plus += data->overcurrent_fault_count_each_channel[ch];
    }

    // Write JSON string to json_output_str
    const int snprintf_ret = snprintf(
        json_output_str, json_output_str_size,
        "{\"stat_ch_on_bitfield\":%u,\"stat_ch_ext_on_bitfield\":%u,\"stat_ch_overcurrent_fault_bitfield\":%u,\"stat_ch_ext_overcurrent_fault_bitfield\":%u,\"overcurrent_fault_count_each_channel\":[%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%lu]}",
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
        data->overcurrent_fault_count_each_channel[16],
        // Put the 17-31 overcurrent count in the last slot.
        overcurrent_fault_count_channel_17_plus
    );

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_size) {
        return 3; // Error: json_output_str too short
    }
    return 0; // Success
}


uint8_t EPS_struct_pdu_overcurrent_fault_comparison_TO_json(
    const EPS_struct_pdu_overcurrent_fault_comparison_t *comparison, 
    const EPS_struct_pdu_overcurrent_fault_state_t *status_before, 
    const EPS_struct_pdu_overcurrent_fault_state_t *status_after, 
    char json_output_str[], 
    uint16_t json_output_str_size
) {
    if (!comparison || !json_output_str || json_output_str_size < 128) {
        return 1;  // Invalid input
    }

    // Buffers for holding intermediate JSON components
    char new_faults_json[512] = {0};  

    // Directly integrate the logic for getting new faults
    int fault_count = 0; 
    const int max_faults = 32; 
    char new_faults_buffer[128]; 
    new_faults_json[0] = '\0'; 

    // Loop through the faults (up to 32 channels)
    for (int i = 0; i < max_faults; i++) {
        if (comparison->channels_with_new_faults_bitfield & (1 << i)) { // Check if the fault is active for this channel
            snprintf(new_faults_buffer, sizeof(new_faults_buffer), "\"channel_%d\"", i + 1); // TODO: correct for actual channel name
            if (fault_count > 0) {
                strncat(new_faults_json, ",", json_output_str_size - strlen(new_faults_json) - 1); // Add comma if not the first item
            }
            strncat(new_faults_json, new_faults_buffer, json_output_str_size - strlen(new_faults_json) - 1);
            fault_count++;
        }
    }

    // Format the JSON output string using the comparison data
    int snprintf_ret = snprintf(json_output_str, json_output_str_size,
        "{"
        "\"power_channel_status\":\"%s\","
        "\"powered_channels_before\":%s,"
        "\"powered_channels_after\":%s,"
        "\"channels_with_new_overcurrent_faults\":[%s],"
        "\"total_fault_count_before\":%u,"
        "\"total_fault_count_after\":%u"
        "}",
        comparison->power_channel_status, 
        comparison->powered_channels_before_json, 
        comparison->powered_channels_after_json,
        new_faults_json,
        comparison->total_fault_count_before, 
        comparison->total_fault_count_after
    );

    // Check for errors in snprintf
    if (snprintf_ret < 0) {
        return 2;  // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_size) {
        return 3;  // Error: json_output_str too short
    }

    return 0;  // Success
}


uint8_t EPS_struct_pbu_abf_placed_state_TO_json(const EPS_struct_pbu_abf_placed_state_t *data, char json_output_str[], uint16_t json_output_str_size) {
    if (data == NULL || json_output_str == NULL || json_output_str_size < 10) {
        return 1; // Error: Invalid input
    }

    // Write JSON string to json_output_str
    const int snprintf_ret = snprintf(
        json_output_str, json_output_str_size,
        "{\"abf_placed_0\":%u,\"abf_placed_1\":%u,\"abf_placed_0_str\":%s,\"abf_placed_1_str\":%s}",
        data->abf_placed_0,
        data->abf_placed_1,
        (data->abf_placed_0 == EPS_ABF_PIN_APPLIED) ? "\"APPLIED\"" : "\"NOT_APPLIED\"",
        (data->abf_placed_1 == EPS_ABF_PIN_APPLIED) ? "\"APPLIED\"" : "\"NOT_APPLIED\"");

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_size) {
        return 3; // Error: json_output_str too short
    }
    return 0; // Success
}


uint8_t EPS_struct_pdu_housekeeping_data_eng_TO_json(const EPS_struct_pdu_housekeeping_data_eng_t *data, char json_output_str[], uint16_t json_output_str_size) {
    if (data == NULL || json_output_str == NULL || json_output_str_size < 10) {
        return 1; // Error: Invalid input
    }
    int ret, offset = 0;

    // Start the JSON string
    offset = snprintf(json_output_str, json_output_str_size, "{");

    // Add voltage_internal_board_supply_mV and temperature_mcu_cC
    ret = snprintf(
        json_output_str + offset, json_output_str_size - offset,
        "\"voltage_internal_board_supply_mV\":%" PRIu16 ",\"temperature_mcu_cC\":%" PRId16 ",",
        data->voltage_internal_board_supply_mV, data->temperature_mcu_cC
    );
    if (ret < 0 || ret >= (json_output_str_size - offset)) return 3;
    offset += ret;

    // Add vip_total_input as JSON
    char vip_total_input_json[128];
    ret = EPS_vpid_eng_TO_json(&data->vip_total_input, vip_total_input_json, sizeof(vip_total_input_json));
    if (ret != 0) return 4;

    ret = snprintf(json_output_str + offset, json_output_str_size - offset, "\"vip_total_input\":%s,", vip_total_input_json);
    if (ret < 0 || ret >= (json_output_str_size - offset)) return 3;
    offset += ret;

    // Add the bitfields
    ret = snprintf(
        json_output_str + offset, json_output_str_size - offset,
        "\"stat_ch_on_bitfield\":%u,\"stat_ch_ext_on_bitfield\":%u,"
        "\"stat_ch_overcurrent_fault_bitfield\":%u,\"stat_ch_ext_overcurrent_fault_bitfield\":%u,",
        data->stat_ch_on_bitfield, data->stat_ch_ext_on_bitfield,
        data->stat_ch_overcurrent_fault_bitfield, data->stat_ch_ext_overcurrent_fault_bitfield
    );
    if (ret < 0 || ret >= (json_output_str_size - offset)) return 3;
    offset += ret;

    // Add vip_each_voltage_domain array as JSON
    ret = snprintf(json_output_str + offset, json_output_str_size - offset, "\"vip_each_voltage_domain\":[");
    if (ret < 0 || ret >= (json_output_str_size - offset)) return 3;
    offset += ret;

    for (int i = 0; i < 7; ++i) {
        char vip_domain_json[128];
        ret = EPS_vpid_eng_TO_json(&data->vip_each_voltage_domain[i], vip_domain_json, sizeof(vip_domain_json));
        if (ret != 0) return 4;

        ret = snprintf(json_output_str + offset, json_output_str_size - offset, "%s%s", vip_domain_json, (i < 6) ? "," : "");
        if (ret < 0 || ret >= (json_output_str_size - offset)) return 3;
        offset += ret;
    }
    ret = snprintf(json_output_str + offset, json_output_str_size - offset, "],");
    if (ret < 0 || ret >= (json_output_str_size - offset)) return 3;
    offset += ret;

    // Add vip_each_channel array as JSON
    ret = snprintf(json_output_str + offset, json_output_str_size - offset, "\"vip_each_channel\":[");
    if (ret < 0 || ret >= (json_output_str_size - offset)) return 3;
    offset += ret;

    for (uint8_t ch = 0; ch <= EPS_MAX_ACTIVE_CHANNEL_NUMBER; ch++) {
        char vip_channel_json[128];
        ret = EPS_vpid_eng_TO_json(&data->vip_each_channel[ch], vip_channel_json, sizeof(vip_channel_json));
        if (ret != 0) return 5;

        ret = snprintf(
            json_output_str + offset,
            json_output_str_size - offset,
            "%s%s",
            vip_channel_json,
            (ch < EPS_MAX_ACTIVE_CHANNEL_NUMBER) ? "," : ""
        );
        if (ret < 0 || ret >= (json_output_str_size - offset)) return 3;
        offset += ret;
    }
    ret = snprintf(json_output_str + offset, json_output_str_size - offset, "]");
    if (ret < 0 || ret >= (json_output_str_size - offset)) return 3;
    offset += ret;

    // End the JSON string
    ret = snprintf(json_output_str + offset, json_output_str_size - offset, "}");
    if (ret < 0 || ret >= (json_output_str_size - offset)) return 3;
    offset += ret;

    // Copy to output
    if (offset >= json_output_str_size) {
        return 3; // Output json_output_str too small
    }
    return 0;
}


uint8_t EPS_struct_single_channel_data_eng_TO_json(const EPS_struct_pdu_housekeeping_data_eng_t *data, const uint8_t eps_channel, char json_output_str[], uint16_t json_output_str_size) {
    if (data == NULL || json_output_str == NULL || json_output_str_size < 10) {
        return 1; // Error: Invalid input
    }

    // Get the string channel name
    const char *eps_channel_name = EPS_channel_to_str(eps_channel);

    const EPS_vpid_eng_t *channel_data = &data->vip_each_channel[eps_channel];

    int snprintf_ret = snprintf(
        json_output_str, json_output_str_size,
        "{\"ch_num\":%d,\"ch_name\":\"%s\",\"mV\":%d,\"mA\":%d,\"cW\":%d}",
        eps_channel, eps_channel_name,channel_data->voltage_mV, channel_data->current_mA, channel_data->power_cW);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_size) {
        return 3; // Error: json_output_str too short
    }

    return 0;
}


uint8_t EPS_struct_pbu_housekeeping_data_eng_TO_json(const EPS_struct_pbu_housekeeping_data_eng_t *data, char json_output_str[], uint16_t json_output_str_size) {
    if (data == NULL || json_output_str == NULL || json_output_str_size < 10) {
        return 1; // Error: Invalid input
    }

    // Convert vip_total_input to JSON using the provided helper function
    char vip_total_input_json[100];
    uint8_t json_ret_code = EPS_vpid_eng_TO_json(
        &(data->vip_total_input),
        vip_total_input_json,
        sizeof(vip_total_input_json)
    );
    if (json_ret_code != 0) {
        return 2; // Error: vip_total_input conversion failed
    }

    // Convert battery_pack_info_each_pack[0] to JSON.
    // Note: Only the first battery pack is rendered, as our EPS only has 1 battery pack.
    char battery_pack_info_json[300];
    json_ret_code = EPS_battery_pack_datatype_eng_TO_json(
        &(data->battery_pack_info_each_pack[0]),
        battery_pack_info_json,
        sizeof(battery_pack_info_json),
        0 // Disable showing unsupported fields
    );
    if (json_ret_code != 0) {
        return 3; // Error: battery_pack_info_each_pack[0] conversion failed
    }
    

    // Format the full JSON string
    int snprintf_ret = snprintf(
        json_output_str, json_output_str_size,
        "{\"voltage_internal_board_supply_mV\":%d,"
        "\"temperature_mcu_cC\":%" PRId16 ","
        "\"vip_total_input\":%s,"
        "\"battery_pack_status_bitfield\":%d,"
        "\"battery_pack\":%s}",
        data->voltage_internal_board_supply_mV,
        data->temperature_mcu_cC,
        vip_total_input_json,
        data->battery_pack_status_bitfield,
        battery_pack_info_json
        // Note: Only the first battery pack is rendered.
    );

    // Check for snprintf errors
    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_size) {
        return 3; // Error: string buffer too short
    }

    return 0; // Success
}


uint8_t EPS_struct_pcu_housekeeping_data_eng_TO_json(const EPS_struct_pcu_housekeeping_data_eng_t *data, char json_output_str[], uint16_t json_output_str_size) {
    if (data == NULL || json_output_str == NULL || json_output_str_size < 10) {
        return 1; // Error: Invalid input
    }


    // Buffer for each channel info
    char conditioning_channel_json[4][200]; // Assume each channel's JSON will fit in 200 chars
    for (int i = 0; i < 4; i++) {
        uint8_t json_ret_code = EPS_conditioning_channel_datatype_eng_TO_json(
            &(data->conditioning_channel_info_each_channel[i]), conditioning_channel_json[i], 200);

        if (json_ret_code != 0) {
            return 2; // Error: Subfunction failed
        }
    }

    // Convert vip_total_input to JSON
    char vip_total_input_json[100];
    uint8_t json_ret_code = EPS_vpid_eng_TO_json(&(data->vip_total_input), vip_total_input_json, 100);
    if (json_ret_code != 0) {
        return 3; // Error: Subfunction failed
    }

    // Prepare the final JSON string
    const int snprintf_ret = snprintf(
        json_output_str, json_output_str_size,
        "{"
        "\"voltage_internal_board_supply_mV\":%d,"
        "\"temperature_mcu_cC\":%" PRId16 ","
        "\"vip_total_input\":%s,"
        "\"conditioning_channel_info_each_channel\":[%s,%s,%s,%s]"
        "}",
        data->voltage_internal_board_supply_mV,
        data->temperature_mcu_cC,
        vip_total_input_json,
        conditioning_channel_json[0],
        conditioning_channel_json[1],
        conditioning_channel_json[2],
        conditioning_channel_json[3]);

    if (snprintf_ret < 0) {
        return 2; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_size) {
        return 3; // Error: Output buffer too short
    }

    return 0; // Success
}


uint8_t EPS_struct_piu_housekeeping_data_eng_TO_json(
    const EPS_struct_piu_housekeeping_data_eng_t *data,
    char json_output_str[],
    uint16_t json_output_str_size
) {
    if (data == NULL || json_output_str == NULL || json_output_str_size < 10) {
        return 1; // Error: Invalid input
    }

    // Buffers for individual components (e.g., vip_dist_input and vip_batt_input)
    char vip_dist_input_json[128];
    char vip_batt_input_json[128];
    char vip_each_channel_json[128 * 16]; // Note: Could do all 32 channels, but only using 16 channels on CTS-SAT-1.
    char conditioning_channel_info_json[256 * 5];
    
    // Convert vip_dist_input and vip_batt_input
    if (EPS_vpid_eng_TO_json(&data->vip_dist_input, vip_dist_input_json, sizeof(vip_dist_input_json)) != 0) {
        return 2; // Error converting vip_dist_input
    }
    if (EPS_vpid_eng_TO_json(&data->vip_batt_input, vip_batt_input_json, sizeof(vip_batt_input_json)) != 0) {
        return 3; // Error converting vip_batt_input
    }

    // Convert vip_each_channel array
    vip_each_channel_json[0] = '\0'; // Initialize the JSON array string
    strcat(vip_each_channel_json, "[");
    for (uint8_t ch = 0; ch <= EPS_MAX_ACTIVE_CHANNEL_NUMBER; ch++) {
        char vip_channel_json[128];
        if (EPS_vpid_eng_TO_json(&data->vip_each_channel[ch], vip_channel_json, sizeof(vip_channel_json)) != 0) {
            return 4; // Error converting vip_each_channel[i]
        }
        strcat(vip_each_channel_json, vip_channel_json);
        if (ch < EPS_MAX_ACTIVE_CHANNEL_NUMBER) {
            strcat(vip_each_channel_json, ",");
        }
    }
    strcat(vip_each_channel_json, "]");

    // Convert conditioning_channel_info_each_channel array
    conditioning_channel_info_json[0] = '\0'; // Initialize the JSON array string
    strcat(conditioning_channel_info_json, "[");
    for (int i = 0; i < 5; i++) {
        char conditioning_channel_json[256];
        if (EPS_conditioning_channel_short_datatype_eng_TO_json(&data->conditioning_channel_info_each_channel[i], conditioning_channel_json, sizeof(conditioning_channel_json)) != 0) {
            return 5; // Error converting conditioning_channel_info_each_channel[i]
        }
        strcat(conditioning_channel_info_json, conditioning_channel_json);
        if (i < 4) {
            strcat(conditioning_channel_info_json, ",");
        }
    }
    strcat(conditioning_channel_info_json, "]");

    // Format the entire JSON output
    const int snprintf_ret = snprintf(
        json_output_str, json_output_str_size,
        "{"
        "\"voltage_internal_board_supply_mV\":%d,"
        "\"temperature_mcu_cC\":%" PRId16 ","
        "\"vip_dist_input\":%s,"
        "\"vip_batt_input\":%s,"
        "\"stat_ch_on_bitfield\":%d,"
        "\"stat_ch_overcurrent_fault_bitfield\":%d,"
        "\"battery_status_bitfield\":%d,"
        "\"battery_temp2_cC\":%d,"
        "\"battery_temp3_cC\":%d,"
        "\"vd012_voltage_mV\":[%d,%d,%d],"
        "\"vip_each_channel\":%s," // List-of-Dicts.
        "\"conditioning_channel_info_each_channel\":%s," // List-of-Dicts.
        "\"stat_ch_ext_on_bitfield\":%d,"
        "\"stat_ch_ext_overcurrent_fault_bitfield\":%d"
        "}",
        data->voltage_internal_board_supply_mV,
        data->temperature_mcu_cC,
        vip_dist_input_json,
        vip_batt_input_json,
        data->stat_ch_on_bitfield,
        data->stat_ch_overcurrent_fault_bitfield,
        data->battery_status_bitfield,
        data->battery_temp2_cC,
        data->battery_temp3_cC,
        data->vd0_voltage_mV,
        data->vd1_voltage_mV,
        data->vd2_voltage_mV,
        vip_each_channel_json,
        conditioning_channel_info_json,
        data->stat_ch_ext_on_bitfield,
        data->stat_ch_ext_overcurrent_fault_bitfield
    );

    if (snprintf_ret < 0) {
        return 6; // Error: snprintf encoding error
    }
    if (snprintf_ret >= json_output_str_size) {
        return 7; // Error: string buffer too short
    }

    return 0; // Success
}
