
#include "eps_drivers/eps_types.h"

/*
 ***** To create these pack_eps_... functions, the following ChatGPT prompt was used: ***** 

=================================================================================================
Write a C code snippet that packs values from the array "rx_buf" into an instance of the
following struct (named "result_dest").

In cases where more than one array element is used, please write it out explicitly by
using the bitwise OR operation of the bytes left-shifted (instead of using type casting).
Here is an example: result_dest->var_example_uint16 = (rx_buf[8]) | (rx_buf[9]<<8);

Assume that the first element of the struct is at rx_buf[5].

typedef struct { ... } EPS_struct_system_status_t;
=================================================================================================

Note that modification are sometimes required, including:
0. In the prompt, rx_buf[5] is sometimes rx_buf[6].
1. Byte order check to be like the example given.
2. Function name.
3. Start/end conditions.
4. Update any multiplier, because LLMs are bad at guessing in advance how many bytes-per-loop are used.

*/

void EPS_pack_eps_result_system_status(const uint8_t rx_buf[], EPS_struct_system_status_t *result_dest) {
    result_dest->mode = rx_buf[5];
    result_dest->config_changed_since_boot = rx_buf[6];
    result_dest->reset_cause = rx_buf[7];
    result_dest->uptime_sec = (rx_buf[11] << 24) | (rx_buf[10] << 16) | (rx_buf[9] << 8) | rx_buf[8];
    result_dest->error_code = (rx_buf[13] << 8) | rx_buf[12];
    result_dest->rst_cnt_pwron = (rx_buf[15] << 8) | rx_buf[14];
    result_dest->rst_cnt_wdg = (rx_buf[17] << 8) | rx_buf[16];
    result_dest->rst_cnt_cmd = (rx_buf[19] << 8) | rx_buf[18];
    result_dest->rst_cnt_mcu = (rx_buf[21] << 8) | rx_buf[20];
    result_dest->rst_cnt_emlopo = (rx_buf[23] << 8) | rx_buf[22];
    result_dest->time_since_prev_cmd_sec = (rx_buf[25] << 8) | rx_buf[24];
    result_dest->unix_time_sec = (rx_buf[29] << 24) | (rx_buf[28] << 16) | (rx_buf[27] << 8) | rx_buf[26];
    result_dest->calendar_years_since_2000 = rx_buf[30];
    result_dest->calendar_month = rx_buf[31];
    result_dest->calendar_day = rx_buf[32];
    result_dest->calendar_hour = rx_buf[33];
    result_dest->calendar_minute = rx_buf[34];
    result_dest->calendar_second = rx_buf[35];
}

void EPS_pack_eps_result_pdu_overcurrent_fault_state(const uint8_t rx_buf[], EPS_struct_pdu_overcurrent_fault_state_t *result_dest) {
    // Note: rx_buf[5] is a reserved/ignored value
	// const uint8_t rx_len = 78;

    result_dest->stat_ch_on_bitfield = (rx_buf[7] << 8) | rx_buf[6];
    result_dest->stat_ch_ext_on_bitfield = (rx_buf[9] << 8) | rx_buf[8];
    result_dest->stat_ch_overcurrent_fault_bitfield = (rx_buf[11] << 8) | rx_buf[10];
    result_dest->stat_ch_ext_overcurrent_fault_bitfield = (rx_buf[13] << 8) | rx_buf[12];

    for (uint8_t ch_num = 0; ch_num <= 31; ch_num++) {
        result_dest->overcurrent_fault_count_each_channel[ch_num] = (rx_buf[15 + ch_num*2] << 8) | rx_buf[14 + ch_num*2];
		// Max rx_buf index is: (15 + ch_num*2) = (15 + 31*2) = 77 [< 78, so good]
    }
}



void EPS_pack_eps_result_pdu_housekeeping_data_eng(const uint8_t rx_buf[], EPS_struct_pdu_housekeeping_data_eng_t *result_dest) {
    result_dest->voltage_internal_board_supply_mV = (rx_buf[6] | (rx_buf[7] << 8));
    result_dest->temperature_mcu_cC = (rx_buf[8] | (rx_buf[9] << 8));

    // Packing vip_total_input
    result_dest->vip_total_input.voltage_mV = (rx_buf[10] | (rx_buf[11] << 8));
    result_dest->vip_total_input.current_mA = (rx_buf[12] | (rx_buf[13] << 8));
    result_dest->vip_total_input.power_cW = (rx_buf[14] | (rx_buf[15] << 8));

    result_dest->stat_ch_on_bitfield = (rx_buf[16] | (rx_buf[17] << 8));
    result_dest->stat_ch_ext_on_bitfield = (rx_buf[18] | (rx_buf[19] << 8));
    result_dest->stat_ch_overcurrent_fault_bitfield = (rx_buf[20] | (rx_buf[21] << 8));
    result_dest->stat_ch_ext_overcurrent_fault_bitfield = (rx_buf[22] | (rx_buf[23] << 8));

    for (uint8_t domain_num = 0; domain_num < 7; domain_num++) {
        result_dest->vip_each_voltage_domain[domain_num].voltage_mV = (rx_buf[24 + domain_num * 6] | (rx_buf[25 + domain_num * 6] << 8));
        result_dest->vip_each_voltage_domain[domain_num].current_mA = (rx_buf[26 + domain_num * 6] | (rx_buf[27 + domain_num * 6] << 8));
        result_dest->vip_each_voltage_domain[domain_num].power_cW = (rx_buf[28 + domain_num * 6] | (rx_buf[29 + domain_num * 6] << 8));
		// Max rx_buf index is: (29 + domain_num * 6) = (29 + 6*6) = 65
    }

    for (uint8_t ch_num = 0; ch_num < 32; ch_num++) {
        result_dest->vip_each_channel[ch_num].voltage_mV = (rx_buf[66 + ch_num * 6] | (rx_buf[67 + ch_num * 6] << 8));
        result_dest->vip_each_channel[ch_num].current_mA = (rx_buf[68 + ch_num * 6] | (rx_buf[69 + ch_num * 6] << 8));
        result_dest->vip_each_channel[ch_num].power_cW = (rx_buf[70 + ch_num * 6] | (rx_buf[71 + ch_num * 6] << 8));
		// Max rx_buf index is: (71 + ch_num * 6) = (71 + 31*6) = 257 [< 258, so good]
    }
}



void EPS_pack_eps_result_pbu_housekeeping_data_eng(const uint8_t rx_buf[], EPS_struct_pbu_housekeeping_data_eng_t *result_dest) {
	// rx_buf_len = 84
    result_dest->vip_total_input.voltage_mV = (rx_buf[6]) | (rx_buf[7] << 8);
    result_dest->vip_total_input.current_mA = (rx_buf[8]) | (rx_buf[9] << 8);
    result_dest->vip_total_input.power_cW = (rx_buf[10]) | (rx_buf[11] << 8);

    result_dest->voltage_internal_board_supply_mV = (rx_buf[12]) | (rx_buf[13] << 8);
    result_dest->temperature_mcu_cC = (rx_buf[14]) | (rx_buf[15] << 8);
    result_dest->battery_pack_status_bitfield = (rx_buf[16]) | (rx_buf[17] << 8);

    for (uint8_t bp_num = 0; bp_num < 3; bp_num++) {
        result_dest->battery_pack_info_each_pack[bp_num].vip_bp_input.voltage_mV = (rx_buf[18 + (bp_num*22)]) | (rx_buf[19 + (bp_num*22)] << 8);
        result_dest->battery_pack_info_each_pack[bp_num].vip_bp_input.current_mA = (rx_buf[20 + (bp_num*22)]) | (rx_buf[21 + (bp_num*22)] << 8);
        result_dest->battery_pack_info_each_pack[bp_num].vip_bp_input.power_cW = (rx_buf[22 + (bp_num*22)]) | (rx_buf[23 + (bp_num*22)] << 8);

        result_dest->battery_pack_info_each_pack[bp_num].bp_status_bitfield = (rx_buf[24 + (bp_num*22)]) | (rx_buf[25 + (bp_num*22)] << 8);

        // Pack data for cell_voltage_each_cell_mV
        for (uint8_t cell_num = 0; cell_num < 4; cell_num++) {
            result_dest->battery_pack_info_each_pack[bp_num].cell_voltage_each_cell_mV[cell_num] =
						(rx_buf[26 + (cell_num*2) + (bp_num*22)]) | (rx_buf[27 + (cell_num*2) + (bp_num*22)] << 8);
        }

        // Pack data for battery_temperature_each_sensor_cC
        for (uint8_t sensor_num = 0; sensor_num < 3; sensor_num++) {
            result_dest->battery_pack_info_each_pack[bp_num].battery_temperature_each_sensor_cC[sensor_num] =
						(rx_buf[34 + (sensor_num*2) + (bp_num*22)]) | (rx_buf[35 + (sensor_num*2) + (bp_num*22)] << 8);
        }

		// Max rx_buf index is: (35 + (sensor_num*2) + (bp_num*22)) = (35 + 2*2 + 2*22) = 83 [< 84, so good]
    }
}


void EPS_pack_eps_result_pcu_housekeeping_data_eng(const uint8_t rx_buf[], EPS_struct_pcu_housekeeping_data_eng_t *result_dest) {
	// rx_buf_len = 72
	result_dest->voltage_internal_board_supply_mV = (rx_buf[6]) | (rx_buf[7] << 8);
    result_dest->temperature_mcu_cC = (rx_buf[8]) | (rx_buf[9] << 8);

    result_dest->vip_total_input.voltage_mV = (rx_buf[10]) | (rx_buf[11] << 8);
    result_dest->vip_total_input.current_mA = (rx_buf[12]) | (rx_buf[13] << 8);
    result_dest->vip_total_input.power_cW = (rx_buf[14]) | (rx_buf[15] << 8);

    for (uint8_t ch_num = 0; ch_num < 4; ch_num++) {
        result_dest->conditioning_channel_info_each_channel[ch_num].vip_cc_output.voltage_mV = (rx_buf[16 + ch_num * 14]) | (rx_buf[17 + ch_num * 14] << 8);
        result_dest->conditioning_channel_info_each_channel[ch_num].vip_cc_output.current_mA = (rx_buf[18 + ch_num * 14]) | (rx_buf[19 + ch_num * 14] << 8);
        result_dest->conditioning_channel_info_each_channel[ch_num].vip_cc_output.power_cW = (rx_buf[20 + ch_num * 14]) | (rx_buf[21 + ch_num * 14] << 8);

        result_dest->conditioning_channel_info_each_channel[ch_num].volt_in_mppt_mV = (rx_buf[22 + ch_num * 14]) | (rx_buf[23 + ch_num * 14] << 8);
        result_dest->conditioning_channel_info_each_channel[ch_num].curr_in_mppt_mA = (rx_buf[24 + ch_num * 14]) | (rx_buf[25 + ch_num * 14] << 8);
        result_dest->conditioning_channel_info_each_channel[ch_num].volt_ou_mppt_mV = (rx_buf[26 + ch_num * 14]) | (rx_buf[27 + ch_num * 14] << 8);
        result_dest->conditioning_channel_info_each_channel[ch_num].curr_ou_mppt_mA = (rx_buf[28 + ch_num * 14]) | (rx_buf[29 + ch_num * 14] << 8);

		// Max rx_buf index is: (29 + ch_num * 14) = (29 + 3*14) = 71 [< 72, so good]
    }
}


void EPS_pack_eps_result_piu_housekeeping_data_eng(const uint8_t rx_buf[], EPS_struct_piu_housekeeping_data_eng_t *result_dest) {
	// rx_len = 274
    result_dest->voltage_internal_board_supply_mV = rx_buf[6] | (rx_buf[7] << 8);
    result_dest->temperature_mcu_cC = rx_buf[8] | (rx_buf[9] << 8);

    result_dest->vip_dist_input.voltage_mV = rx_buf[10] | (rx_buf[11] << 8);
    result_dest->vip_dist_input.current_mA = rx_buf[12] | (rx_buf[13] << 8);
    result_dest->vip_dist_input.power_cW = rx_buf[14] | (rx_buf[15] << 8);

    result_dest->vip_batt_input.voltage_mV = rx_buf[16] | (rx_buf[17] << 8);
    result_dest->vip_batt_input.current_mA = rx_buf[18] | (rx_buf[19] << 8);
    result_dest->vip_batt_input.power_cW = rx_buf[20] | (rx_buf[21] << 8);

    result_dest->stat_ch_on_bitfield = rx_buf[22] | (rx_buf[23] << 8);
    result_dest->stat_ch_overcurrent_fault_bitfield = rx_buf[24] | (rx_buf[25] << 8);
    result_dest->battery_status_bitfield = rx_buf[26] | (rx_buf[27] << 8);
    result_dest->battery_temp2_cC = rx_buf[28] | (rx_buf[29] << 8);
    result_dest->battery_temp3_cC = rx_buf[30] | (rx_buf[31] << 8);

    result_dest->vd0_voltage_mV = rx_buf[32] | (rx_buf[33] << 8);
    result_dest->vd1_voltage_mV = rx_buf[34] | (rx_buf[35] << 8);
    result_dest->vd2_voltage_mV = rx_buf[36] | (rx_buf[37] << 8);

	// EVERYTHING BELOW THIS LINE IS NOT IN BYTE ORDER

	// VIP_CH[0] to VIP_CH[8]
	for (uint8_t ch_num = 0; ch_num <= 8; ch_num++) {
		result_dest->vip_each_channel[ch_num].voltage_mV = (rx_buf[38 + ch_num*6]) | (rx_buf[39 + ch_num*6] << 8);
		result_dest->vip_each_channel[ch_num].current_mA = (rx_buf[40 + ch_num*6]) | (rx_buf[41 + ch_num*6] << 8);
		result_dest->vip_each_channel[ch_num].power_cW = (rx_buf[42 + ch_num*6]) | (rx_buf[43 + ch_num*6] << 8);
		// Max rx_buf index here is: (43 + ch_num*6) = (43 + 8*6) = 91
	}

	// NOTE: cc channels go CC1, CC2, CC3 in the Software ICD.
	//   We are changing such that CC1 is at conditioning_channel_info_each_channel[0].
	for (uint8_t cc_num = 0; cc_num <= 2; cc_num++) {
		result_dest->conditioning_channel_info_each_channel[cc_num].volt_in_mppt_mV = (rx_buf[92 + cc_num*8]) | (rx_buf[93 + cc_num*8] << 8);
		result_dest->conditioning_channel_info_each_channel[cc_num].curr_in_mppt_mA = (rx_buf[94 + cc_num*8]) | (rx_buf[95 + cc_num*8] << 8);
		result_dest->conditioning_channel_info_each_channel[cc_num].volt_ou_mppt_mV = (rx_buf[96 + cc_num*8]) | (rx_buf[97 + cc_num*8] << 8);
		result_dest->conditioning_channel_info_each_channel[cc_num].curr_ou_mppt_mA = (rx_buf[98 + cc_num*8]) | (rx_buf[99 + cc_num*8] << 8);
		// Max rx_buf index here is: (99 + cc_num*8) = (99 + 2*8) = 115
	}

	// VIP_CH[9] to VIP_CH[15]
	for (uint8_t ch_num = 9; ch_num <= 15; ch_num++) {
		const uint8_t loop_num = ch_num - 9;
		result_dest->vip_each_channel[ch_num].voltage_mV = (rx_buf[116 + loop_num*6]) | (rx_buf[117 + loop_num*6] << 8);
		result_dest->vip_each_channel[ch_num].current_mA = (rx_buf[118 + loop_num*6]) | (rx_buf[119 + loop_num*6] << 8);
		result_dest->vip_each_channel[ch_num].power_cW = (rx_buf[120 + loop_num*6]) | (rx_buf[121 + loop_num*6] << 8);
		// Max rx_buf index here is: (121 + loop_num*6) = (121 + (15-9)*6) = 157
	}

	// CC4 (cc_num=3, loop_num=0), CC5 (cc_num=4, loop_num=1)
	for (uint8_t cc_num = 3; cc_num <= 4; cc_num++) {
		const uint8_t loop_num = cc_num - 3;
		result_dest->conditioning_channel_info_each_channel[cc_num].volt_in_mppt_mV = (rx_buf[158 + loop_num*8]) | (rx_buf[159 + loop_num*8] << 8);
		result_dest->conditioning_channel_info_each_channel[cc_num].curr_in_mppt_mA = (rx_buf[160 + loop_num*8]) | (rx_buf[161 + loop_num*8] << 8);
		result_dest->conditioning_channel_info_each_channel[cc_num].volt_ou_mppt_mV = (rx_buf[162 + loop_num*8]) | (rx_buf[163 + loop_num*8] << 8);
		result_dest->conditioning_channel_info_each_channel[cc_num].curr_ou_mppt_mA = (rx_buf[164 + loop_num*8]) | (rx_buf[165 + loop_num*8] << 8);
		// Max rx_buf index here is: (165 + loop_num*8) = (165 + (1)*8) = 173
	}

    result_dest->stat_ch_on_bitfield = rx_buf[174] | (rx_buf[175] << 8);
    result_dest->stat_ch_overcurrent_fault_bitfield = rx_buf[176] | (rx_buf[177] << 8);
	
	// VIP_CH[16] to VIP_CH[31]
    // FIXME: Disable this block probably.
	for (uint8_t ch_num = 16; ch_num <= 31; ch_num++) {
		const uint8_t loop_num = ch_num - 16;
		result_dest->vip_each_channel[ch_num].voltage_mV = (rx_buf[178 + loop_num*6]) | (rx_buf[179 + loop_num*6] << 8);
		result_dest->vip_each_channel[ch_num].current_mA = (rx_buf[180 + loop_num*6]) | (rx_buf[181 + loop_num*6] << 8);
		result_dest->vip_each_channel[ch_num].power_cW = (rx_buf[182 + loop_num*6]) | (rx_buf[183 + loop_num*6] << 8);
		// Max rx_buf index is: (183 + loop_num*6) = (183 + (31-16)*6) = 273
	}
}
