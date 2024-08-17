
#ifndef INCLUDE_GUARD__EPS_TYPES_TO_JSON_H__
#define INCLUDE_GUARD__EPS_TYPES_TO_JSON_H__

#include "eps_types.h"


uint8_t EPS_vpid_eng_TO_json(
    const EPS_vpid_eng_t *data,
    char json_output_str[],
    uint16_t json_output_str_len
);

uint8_t EPS_battery_pack_datatype_eng_TO_json(
    const EPS_battery_pack_datatype_eng_t *data,
    char json_output_str[],
    uint16_t json_output_str_len
);

uint8_t EPS_result_system_status_TO_json(
    const EPS_result_system_status_t *data,
    char json_output_str[],
    uint16_t json_output_str_len
);


#endif // INCLUDE_GUARD__EPS_TYPES_TO_JSON_H__
