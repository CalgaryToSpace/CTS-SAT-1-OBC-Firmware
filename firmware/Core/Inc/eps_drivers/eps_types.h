
#ifndef __INCLUDE_GUARD__EPS_TYPES_H__
#define __INCLUDE_GUARD__EPS_TYPES_H__

#include <stdint.h>

typedef enum {
	// TODO: add the 5V MPI channel
	EPS_CHANNEL_VBATT_STACK             = 0, // CH0
	EPS_CHANNEL_5V_STACK                = 1, // CH1
	EPS_CHANNEL_5V_CH2_UNUSED           = 2, // CH2
	EPS_CHANNEL_5V_CH3_UNUSED           = 3, // CH3
	EPS_CHANNEL_5V_CH4_UNUSED           = 4, // CH4
	EPS_CHANNEL_3V3_STACK               = 5, // CH5
	EPS_CHANNEL_3V3_CAMERA              = 6, // CH6
	EPS_CHANNEL_3V3_UHF_ANTENNA_DEPLOY  = 7, // CH7
	EPS_CHANNEL_3V3_LORA_MODULES        = 8, // CH8
	EPS_CHANNEL_VBATT_CH9_UNUSED        = 9, // CH9
	EPS_CHANNEL_VBATT_CH10_UNUSED       = 10, // CH10
	EPS_CHANNEL_VBATT_CH11_UNUSED       = 11, // CH11
	EPS_CHANNEL_12V_MPI                 = 12, // CH12
	EPS_CHANNEL_12V_BOOM                = 13, // CH13
	EPS_CHANNEL_3V3_CH14_UNUSED         = 14, // CH14
	EPS_CHANNEL_3V3_CH15_UNUSED         = 15, // CH15
	EPS_CHANNEL_28V6_CH16_UNUSED        = 16, // CH16
} EPS_CHANNEL_enum_t;

// "Complex Datatype: VIPD" (raw)
typedef struct {
	int16_t voltage_raw;
	int16_t current_raw;
	int16_t power_raw;
} eps_vpid_raw_t;

// "Complex Datatype: VIPD" (eng)
typedef struct {
	int16_t voltage_mV;
	int16_t current_mA;
	int16_t power_cW; // centiWatts (x 10^-2 watts)
} eps_vpid_eng_t;

// "Complex Datatype: BPD" (raw)
typedef struct {
	eps_vpid_raw_t vip_bp_input_raw;
	uint16_t bp_status_bitfield; // See Table 3-18 for bitfield definitions
	uint16_t cell_voltage_each_cell_raw[4];
    uint16_t battery_temperature_each_sensor_raw[3];
} eps_battery_pack_datatype_raw_t;

// "Complex Datatype: BPD (Battery Pack Datatype)" (eng)
typedef struct {
    eps_vpid_eng_t vip_bp_input;
	uint16_t bp_status_bitfield; // See Table 3-18 for bitfield definitions
    int16_t cell_voltage_each_cell_mV[4];
    int16_t battery_temperature_each_sensor_cC[3]; // centiCelsius = 1e-2 degrees C
} eps_battery_pack_datatype_eng_t;

// "Complex Datatype: CCD (Conditioning Channel Datatype)" (raw)
// The conditioning channel refers to the MPPT system.
typedef struct {
	eps_vpid_raw_t vip_cc_output_raw;
    uint16_t volt_in_mppt_raw;
    uint16_t curr_in_mppt_raw;
    uint16_t volt_ou_mppt_raw;
    uint16_t curr_ou_mppt_raw;
} eps_conditioning_channel_datatype_raw_t;

// "Complex Datatype: CCD (Conditioning Channel Datatype)" (eng)
// The conditioning channel refers to the MPPT system.
typedef struct {
	eps_vpid_eng_t vip_cc_output;
    int16_t volt_in_mppt_mV;
    int16_t curr_in_mppt_mA;
    int16_t volt_ou_mppt_mV;
    int16_t curr_ou_mppt_mA;
} eps_conditioning_channel_datatype_eng_t;

// "Complex Datatype: CCSD (Conditioning Channel Short Datatype)" (raw)
// The conditioning channel refers to the MPPT system.
typedef struct {
    uint16_t volt_in_mppt_raw;
    uint16_t curr_in_mppt_raw;
    uint16_t volt_ou_mppt_raw;
    uint16_t curr_ou_mppt_raw;
} eps_conditioning_channel_short_datatype_raw_t;

// "Complex Datatype: CCSD (Conditioning Channel Short Datatype)" (eng)
// The conditioning channel refers to the MPPT system.
typedef struct {
    int16_t volt_in_mppt_mV;
    int16_t curr_in_mppt_mA;
    int16_t volt_ou_mppt_mV;
    int16_t curr_ou_mppt_mA;
} eps_conditioning_channel_short_datatype_eng_t;

// Command Response: 0x40: Get System Status
typedef struct {
	uint8_t mode; // 0=startup, 1=nominal, 2=safety, 3=emergency_low_power
	uint8_t config_changed_since_boot; // 0=not_altered, 1=changed_since_boot
	uint8_t reset_cause; // 0=power_on, 1=watchdog, 2=commanded, 3=control_system_reset, 4=emergency_low_power
	uint32_t uptime_sec;
	uint16_t error_code;
	uint16_t rst_cnt_pwron;
	uint16_t rst_cnt_wdg;
	uint16_t rst_cnt_cmd;
	uint16_t rst_cnt_mcu;
	uint16_t rst_cnt_emlopo; // reset count due to Emergency Low Power
	uint16_t time_since_prev_cmd_sec;
	uint32_t unix_time_sec; // seconds since 1970-01-01
	uint8_t calendar_years_since_2000; // real year = 2000 + unix_years_since_2000
	uint8_t calendar_month;
	uint8_t calendar_day;
	uint8_t calendar_hour;
	uint8_t calendar_minute;
	uint8_t calendar_second;
} eps_result_system_status_t;

// Command Response: 0x42: Get Overcurrent Fault State
typedef struct {
    // Note: rx_buf[5] is a reserved/ignored value
	uint16_t stat_ch_on_bitfield;
	uint16_t stat_ch_ext_on_bitfield; // channels not present on our model
	uint16_t stat_ch_overcurrent_fault_bitfield;
	uint16_t stat_ch_ext_overcurrent_fault_bitfield; // channels not present on our model
    uint16_t overcurrent_fault_count_each_channel[32]; // only channels 0-15 are present on our model
} eps_result_pdu_overcurrent_fault_state_t;

// Command Response: 0x44: Get PBU ABF Placed State
typedef enum {
    EPS_ABF_PIN_APPLIED = 0,
    EPS_ABF_PIN_NOT_APPLIED = 0xAB
} EPS_ABF_PIN_PLACED_enum_t;
typedef struct {
	EPS_ABF_PIN_PLACED_enum_t abf_placed_0;
	EPS_ABF_PIN_PLACED_enum_t abf_placed_1;
}  eps_result_pbu_abf_placed_state_t;


// Command Response: 0x50: Get PDU (Distribution Unit) Housekeeping Data (Raw)
typedef struct {
	uint16_t voltage_internal_board_supply_raw;
	uint16_t temperature_mcu_raw;

	eps_vpid_raw_t vip_total_input_raw;
    
    uint16_t stat_ch_on_bitfield;
	uint16_t stat_ch_ext_on_bitfield; // channels not present on our model
	uint16_t stat_ch_overcurrent_fault_bitfield;
	uint16_t stat_ch_ext_overcurrent_fault_bitfield; // channels not present on our model

    eps_vpid_raw_t vip_each_voltage_domain_raw[7];
    eps_vpid_raw_t vip_each_channel_raw[32];
} eps_result_pdu_housekeeping_data_raw_t;

// Command Response: 0x52: Get PDU (Distribution Unit) Housekeeping Data (Eng)
// Command Response: 0x54: Get PDU (Distribution Unit) Housekeeping Data (Running Average, Eng)
typedef struct {
	uint16_t voltage_internal_board_supply_mV;
	uint16_t temperature_mcu_cC; // centiCelsius = 1e-2 degrees C

	eps_vpid_eng_t vip_total_input;
    
    uint16_t stat_ch_on_bitfield;
	uint16_t stat_ch_ext_on_bitfield; // channels not present on our model
	uint16_t stat_ch_overcurrent_fault_bitfield;
	uint16_t stat_ch_ext_overcurrent_fault_bitfield; // channels not present on our model

    eps_vpid_eng_t vip_each_voltage_domain[7];
    eps_vpid_eng_t vip_each_channel[32];
} eps_result_pdu_housekeeping_data_eng_t;

// Command Response: 0x60: Get PBU (Battery Unit) Housekeeping Data (Raw)
typedef struct {
	uint16_t voltage_internal_board_supply_raw;
	uint16_t temperature_mcu_raw;
	eps_vpid_raw_t vip_total_input_raw;
    uint16_t battery_pack_status_bitfield; // Table 3-18: Battery Pack Status

    eps_battery_pack_datatype_raw_t battery_pack_info_each_pack_raw[3];
} eps_result_pbu_housekeeping_data_raw_t;

// Command Response: 0x62: Get PBU (Battery Unit) Housekeeping Data (Eng)
// Command Response: 0x64: Get PBU (Battery Unit) Housekeeping Data (Running Average, Eng)
typedef struct {
	uint16_t voltage_internal_board_supply_mV;
	uint16_t temperature_mcu_cC; // centiCelsius = 1e-2 degrees C
	eps_vpid_eng_t vip_total_input;
    uint16_t battery_pack_status_bitfield; // Table 3-18: Battery Pack Status

    eps_battery_pack_datatype_eng_t battery_pack_info_each_pack[3];
} eps_result_pbu_housekeeping_data_eng_t;

// Command Response: 0x70: Get PCU (Conditioning Unit) Housekeeping Data (Raw)
typedef struct {
	uint16_t voltage_internal_board_supply_raw;
	uint16_t temperature_mcu_raw;
	eps_vpid_raw_t vip_total_input_raw;

    eps_conditioning_channel_datatype_raw_t conditioning_channel_info_each_channel_raw[4];
} eps_result_pcu_housekeeping_data_raw_t;

// Command Response: 0x72: Get PCU (Conditioning Unit) Housekeeping Data (Eng)
// Command Response: 0x74: Get PCU (Conditioning Unit) Housekeeping Data (Running Average, Eng)
typedef struct {
    uint16_t voltage_internal_board_supply_mV;
    uint16_t temperature_mcu_cC; // centiCelsius = 1e-2 degrees C
    eps_vpid_eng_t vip_total_input;

    eps_conditioning_channel_datatype_eng_t conditioning_channel_info_each_channel[4];
} eps_result_pcu_housekeeping_data_eng_t;

// Command Response: 0xA0: Get PDU (Distribution Unit) Housekeeping Data (Raw)
typedef struct {
	uint16_t voltage_internal_board_supply_raw;
	uint16_t temperature_mcu_raw;

    eps_vpid_raw_t vip_dist_input_raw;
    eps_vpid_raw_t vip_batt_input_raw;
    uint16_t stat_ch_on_bitfield;
    uint16_t stat_ch_overcurrent_fault_bitfield;
    uint16_t battery_status_bitfield; // Table 3-18: Battery Pack Status
    uint16_t battery_temp2_raw;
    uint16_t battery_temp3_raw;

    uint16_t vd0_voltage_raw;
    uint16_t vd1_voltage_raw;
    uint16_t vd2_voltage_raw;

    // Note: elements below this line are not in byte order they are received in

    eps_vpid_raw_t vip_each_channel_raw[32];
    eps_conditioning_channel_short_datatype_raw_t conditioning_channel_info_each_channel_raw[5];

    uint16_t stat_ch_ext_on_bitfield;
    uint16_t stat_ch_ext_overcurrent_fault_bitfield;
} eps_result_piu_housekeeping_data_raw_t;


// Command Response: 0xA2: Get PDU (Distribution Unit) Housekeeping Data (Eng)
// Command Response: 0xA4: Get PDU (Distribution Unit) Housekeeping Data (Running Average, Eng)
typedef struct {
	uint16_t voltage_internal_board_supply_mV;
	uint16_t temperature_mcu_cC; // centiCelsius = 1e-2 degrees C

    eps_vpid_eng_t vip_dist_input;
    eps_vpid_eng_t vip_batt_input;
    uint16_t stat_ch_on_bitfield;
    uint16_t stat_ch_overcurrent_fault_bitfield;
    uint16_t battery_status_bitfield; // Table 3-18: Battery Pack Status
    uint16_t battery_temp2_cC;
    uint16_t battery_temp3_cC;

    uint16_t vd0_voltage_mV;
    uint16_t vd1_voltage_mV;
    uint16_t vd2_voltage_mV;

    // Note: elements below this line are not in byte order they are received in

    eps_vpid_eng_t vip_each_channel[32];
    eps_conditioning_channel_short_datatype_eng_t conditioning_channel_info_each_channel[5];

    uint16_t stat_ch_ext_on_bitfield;
    uint16_t stat_ch_ext_overcurrent_fault_bitfield;
} eps_result_piu_housekeeping_data_eng_t;

#endif /* __INCLUDE_GUARD__EPS_TYPES_H__ */
