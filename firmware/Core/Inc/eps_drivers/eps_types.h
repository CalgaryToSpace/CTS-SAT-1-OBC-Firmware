
#ifndef INCLUDE_GUARD__EPS_TYPES_H__
#define INCLUDE_GUARD__EPS_TYPES_H__

#include <stdint.h>

/// @brief The number of EPS channels
static const uint8_t EPS_ACTIVE_CHANNEL_COUNT = 17;

/// @brief The highest EPS channel number, starting from 0. 16 means there are 17 channels total.
static const uint8_t EPS_MAX_ACTIVE_CHANNEL_NUMBER = 16;

/// @brief The number of EPS channels, including those not really present on our model.
static const uint8_t EPS_TOTAL_CHANNEL_COUNT = 32;

#pragma pack(push, 1)

typedef enum {
    EPS_CHANNEL_VBATT_STACK             = 0, // CH0 (Pin H2-45 and H2-46)
    EPS_CHANNEL_5V_STACK                = 1, // CH1 (Pin H2-25 and H2-26)
    EPS_CHANNEL_5V_CH2_UNUSED           = 2, // CH2 (Pin H1-47)
    EPS_CHANNEL_5V_CH3_UNUSED           = 3, // CH3 (Pin H1-49)
    EPS_CHANNEL_5V_MPI                  = 4, // CH4 (Pin H1-51)
    EPS_CHANNEL_3V3_STACK               = 5, // CH5 (Pin H2-27 and H2-28)
    EPS_CHANNEL_3V3_CAMERA              = 6, // CH6 (Pin H1-48)
    EPS_CHANNEL_3V3_UHF_ANTENNA_DEPLOY  = 7, // CH7 (Pin H1-50)
    EPS_CHANNEL_3V3_GNSS                = 8, // CH8 (Pin H1-52)
    EPS_CHANNEL_VBATT_CH9_UNUSED        = 9, // CH9
    EPS_CHANNEL_VBATT_CH10_UNUSED       = 10, // CH10
    EPS_CHANNEL_VBATT_CH11_UNUSED       = 11, // CH11
    EPS_CHANNEL_12V_MPI                 = 12, // CH12 (Harness)
    EPS_CHANNEL_12V_BOOM                = 13, // CH13 (Harness)
    EPS_CHANNEL_3V3_CH14_UNUSED         = 14, // CH14
    EPS_CHANNEL_3V3_CH15_UNUSED         = 15, // CH15
    EPS_CHANNEL_28V6_CH16_UNUSED        = 16, // CH16
    EPS_CHANNEL_UNKNOWN                 = 255, // Unknown Channel, parsing failed, etc.
} EPS_CHANNEL_enum_t;


// "Complex Datatype: VIPD" (eng)
typedef struct {
    int16_t voltage_mV;
    int16_t current_mA;
    int16_t power_cW; // centiWatts (x 10^-2 watts)
} EPS_vpid_eng_t;

// "Complex Datatype: BPD (Battery Pack Datatype)" (eng)
typedef struct {
    EPS_vpid_eng_t vip_bp_input;
    uint16_t bp_status_bitfield; // See Table 3-18 for bitfield definitions
    int16_t cell_voltage_each_cell_mV[4];
    int16_t battery_temperature_each_sensor_cC[3]; // centiCelsius = 1e-2 degrees C
} EPS_battery_pack_datatype_eng_t;


// "Complex Datatype: CCD (Conditioning Channel Datatype)" (eng)
// The conditioning channel refers to the MPPT system.
typedef struct {
    EPS_vpid_eng_t vip_cc_output;
    int16_t volt_in_mppt_mV;
    int16_t curr_in_mppt_mA;
    int16_t volt_ou_mppt_mV;
    int16_t curr_ou_mppt_mA;
} EPS_conditioning_channel_datatype_eng_t;

// "Complex Datatype: CCSD (Conditioning Channel Short Datatype)" (eng)
// The conditioning channel refers to the MPPT system.
typedef struct {
    int16_t volt_in_mppt_mV;
    int16_t curr_in_mppt_mA;
    int16_t volt_ou_mppt_mV;
    int16_t curr_ou_mppt_mA;
} EPS_conditioning_channel_short_datatype_eng_t;

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
} EPS_struct_system_status_t;

// Command Response: 0x42: Get Overcurrent Fault State
typedef struct {
    // Note: rx_buf[5] is a reserved/ignored value
    uint16_t stat_ch_on_bitfield;
    uint16_t stat_ch_ext_on_bitfield; // Channels not present on our model.
    uint16_t stat_ch_overcurrent_fault_bitfield;
    uint16_t stat_ch_ext_overcurrent_fault_bitfield; // Channels not present on our model.
    // Only channels 0-16 (17 ch total) are present on our model.
    uint16_t overcurrent_fault_count_each_channel[32];
} EPS_struct_pdu_overcurrent_fault_state_t;

// Command Response: 0x44: Get PBU ABF Placed State
typedef enum {
    EPS_ABF_PIN_NOT_APPLIED = 0x00,
    EPS_ABF_PIN_APPLIED = 0xAB,
} EPS_ABF_PIN_PLACED_enum_t;
typedef struct {
    EPS_ABF_PIN_PLACED_enum_t abf_placed_0;
    EPS_ABF_PIN_PLACED_enum_t abf_placed_1;
}  EPS_struct_pbu_abf_placed_state_t;

// Command Response: 0x52: Get PDU (Distribution Unit) Housekeeping Data (Eng)
// Command Response: 0x54: Get PDU (Distribution Unit) Housekeeping Data (Running Average, Eng)
typedef struct {
    int16_t voltage_internal_board_supply_mV;
    int16_t temperature_mcu_cC; // centiCelsius = 1e-2 degrees C

    EPS_vpid_eng_t vip_total_input;
    
    uint16_t stat_ch_on_bitfield;
    uint16_t stat_ch_ext_on_bitfield; // channels not present on our model
    uint16_t stat_ch_overcurrent_fault_bitfield;
    uint16_t stat_ch_ext_overcurrent_fault_bitfield; // channels not present on our model

    EPS_vpid_eng_t vip_each_voltage_domain[7];
    EPS_vpid_eng_t vip_each_channel[32];
} EPS_struct_pdu_housekeeping_data_eng_t;

// Command Response: 0x62: Get PBU (Battery Unit) Housekeeping Data (Eng)
// Command Response: 0x64: Get PBU (Battery Unit) Housekeeping Data (Running Average, Eng)
typedef struct {
    int16_t voltage_internal_board_supply_mV;
    int16_t temperature_mcu_cC; // centiCelsius = 1e-2 degrees C
    EPS_vpid_eng_t vip_total_input;
    uint16_t battery_pack_status_bitfield; // Table 3-18: Battery Pack Status

    // Note: While the data transfer spec supports 3 battery packs, we only have 1.
    // The in the JSON function, only the first is rendered.
    EPS_battery_pack_datatype_eng_t battery_pack_info_each_pack[3];
} EPS_struct_pbu_housekeeping_data_eng_t;


// Command Response: 0x72: Get PCU (Conditioning Unit) Housekeeping Data (Eng)
// Command Response: 0x74: Get PCU (Conditioning Unit) Housekeeping Data (Running Average, Eng)
typedef struct {
    int16_t voltage_internal_board_supply_mV;
    int16_t temperature_mcu_cC; // centiCelsius = 1e-2 degrees C
    EPS_vpid_eng_t vip_total_input;

    EPS_conditioning_channel_datatype_eng_t conditioning_channel_info_each_channel[4];
} EPS_struct_pcu_housekeeping_data_eng_t;



// Command Response: 0xA2: Get PDU (Distribution Unit) Housekeeping Data (Eng)
// Command Response: 0xA4: Get PDU (Distribution Unit) Housekeeping Data (Running Average, Eng)
typedef struct {
    int16_t voltage_internal_board_supply_mV;
    int16_t temperature_mcu_cC; // centiCelsius = 1e-2 degrees C

    EPS_vpid_eng_t vip_dist_input;
    EPS_vpid_eng_t vip_batt_input;
    uint16_t stat_ch_on_bitfield;
    uint16_t stat_ch_overcurrent_fault_bitfield;
    uint16_t battery_status_bitfield; // Table 3-18: Battery Pack Status
    int16_t battery_temp2_cC;
    int16_t battery_temp3_cC;

    uint16_t vd0_voltage_mV;
    uint16_t vd1_voltage_mV;
    uint16_t vd2_voltage_mV;

    // Note: elements below this line are not in byte order they are received in

    EPS_vpid_eng_t vip_each_channel[32];
    EPS_conditioning_channel_short_datatype_eng_t conditioning_channel_info_each_channel[5];

    uint16_t stat_ch_ext_on_bitfield;
    uint16_t stat_ch_ext_overcurrent_fault_bitfield;
} EPS_struct_piu_housekeeping_data_eng_t;

#pragma pack(pop)

#endif /* INCLUDE_GUARD__EPS_TYPES_H__ */
