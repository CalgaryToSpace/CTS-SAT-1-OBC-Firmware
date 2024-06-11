
#ifndef __INCLUDE_GUARD__EPS_TYPES_TO_JSON_H__
#define __INCLUDE_GUARD__EPS_TYPES_TO_JSON_H__

#include "eps_types.h"

uint8_t eps_vpid_raw_TO_json(const eps_vpid_raw_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t eps_vpid_eng_TO_json(const eps_vpid_eng_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t eps_battery_pack_datatype_raw_TO_json(const eps_battery_pack_datatype_raw_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t eps_battery_pack_datatype_eng_TO_json(const eps_battery_pack_datatype_eng_t *data, char json_output_str[], uint16_t json_output_str_len);


#endif // __INCLUDE_GUARD__EPS_TYPES_TO_JSON_H__
