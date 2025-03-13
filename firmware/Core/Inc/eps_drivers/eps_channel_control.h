#ifndef INCLUDE_GUARD__EPS_CHANNEL_CONTROL_H
#define INCLUDE_GUARD__EPS_CHANNEL_CONTROL_H

#include <stdint.h>
#include "eps_drivers/eps_types.h"

EPS_CHANNEL_enum_t EPS_channel_from_str(const char channel_name[]);

uint8_t EPS_set_channel_enabled(EPS_CHANNEL_enum_t channel, uint8_t enabled);

char* EPS_channel_to_str(EPS_CHANNEL_enum_t channel);

uint8_t EPS_check_status_bit_of_channel(uint16_t status_bitfield_1 , uint16_t status_bitfield_2, uint8_t channel_number);

uint8_t EPS_convert_ch_num_to_string_and_append(
    char *response_output_buf,
    uint16_t response_output_buf_len,
    uint8_t ch_number
);
void EPS_get_enabled_channels_json(
    uint16_t status_bitfield_1, uint16_t status_bitfield_2,
    char *response_output_buf, uint16_t response_output_buf_len
);

#endif // INCLUDE_GUARD__EPS_CHANNEL_CONTROL_H
