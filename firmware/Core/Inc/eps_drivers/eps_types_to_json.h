
#ifndef INCLUDE_GUARD__EPS_TYPES_TO_JSON_H__
#define INCLUDE_GUARD__EPS_TYPES_TO_JSON_H__

#include "eps_types.h"


uint8_t EPS_vpid_eng_TO_json(
    const EPS_vpid_eng_t *data,
    char json_output_str[],
    uint16_t json_output_str_size
);

uint8_t EPS_battery_pack_datatype_eng_TO_json(
    const EPS_battery_pack_datatype_eng_t *data,
    char json_output_str[],
    uint16_t json_output_str_size,
    uint8_t enable_show_unsupported_fields
);

uint8_t EPS_struct_system_status_TO_json(
    const EPS_struct_system_status_t *data,
    char json_output_str[],
    uint16_t json_output_str_size
);

uint8_t EPS_struct_pdu_overcurrent_fault_state_TO_json(const EPS_struct_pdu_overcurrent_fault_state_t *data, char json_output_str[], uint16_t json_output_str_size);
uint8_t EPS_struct_pbu_abf_placed_state_TO_json(const EPS_struct_pbu_abf_placed_state_t *data, char json_output_str[], uint16_t json_output_str_size);

uint8_t EPS_struct_pdu_housekeeping_data_eng_TO_json(const EPS_struct_pdu_housekeeping_data_eng_t *data, char json_output_str[], uint16_t json_output_str_size);
uint8_t EPS_struct_single_channel_data_eng_TO_json(const EPS_struct_pdu_housekeeping_data_eng_t *data, const uint8_t eps_channel, char json_output_str[], uint16_t json_output_str_size);
uint8_t EPS_struct_pbu_housekeeping_data_eng_TO_json(const EPS_struct_pbu_housekeeping_data_eng_t *data, char json_output_str[], uint16_t json_output_str_size);
uint8_t EPS_struct_pcu_housekeeping_data_eng_TO_json(const EPS_struct_pcu_housekeeping_data_eng_t *data, char json_output_str[], uint16_t json_output_str_size);
uint8_t EPS_struct_piu_housekeeping_data_eng_TO_json(const EPS_struct_piu_housekeeping_data_eng_t *data, char json_output_str[], uint16_t json_output_str_size);

#endif // INCLUDE_GUARD__EPS_TYPES_TO_JSON_H__
