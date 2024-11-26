#ifndef INCLUDE_GUARD__EPS_CHANNEL_CONTROL_H
#define INCLUDE_GUARD__EPS_CHANNEL_CONTROL_H

#include <stdint.h>
#include "eps_drivers/eps_types.h"

EPS_CHANNEL_enum_t EPS_channel_from_str(const char channel_name[]);

uint8_t EPS_set_channel_enabled(EPS_CHANNEL_enum_t channel, uint8_t enabled);

void EPS_convert_stat_bit_to_string(char *response_output_buf, uint16_t stat_bit_1 , uint16_t stat_bit_2);

#endif // INCLUDE_GUARD__EPS_CHANNEL_CONTROL_H
