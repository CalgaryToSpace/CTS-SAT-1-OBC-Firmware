#ifndef INCLUDE_GUARD__EPS_CHANNEL_CONTROL_H
#define INCLUDE_GUARD__EPS_CHANNEL_CONTROL_H

#include <stdint.h>
#include "eps_drivers/eps_types.h"

EPS_CHANNEL_enum_t EPS_channel_from_str(const char channel_name[]);

uint8_t EPS_set_channel_enabled(EPS_CHANNEL_enum_t channel, uint8_t enabled);

#endif // INCLUDE_GUARD__EPS_CHANNEL_CONTROL_H
