#ifndef INCLUDE_GUARD_COMMS_DRIVERS_H
#define INCLUDE_GUARD_COMMS_DRIVERS_H

#include <stdint.h>

extern uint8_t COMMS_current_active_antenna;

void COMMS_set_dipole_switch_state(uint8_t dipole_switch_antenna_num);

#endif // INCLUDE_GUARD_COMMS_DRIVERS_H
