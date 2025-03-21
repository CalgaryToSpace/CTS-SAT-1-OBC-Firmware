#ifndef INCLUDE_GUARD_COMMS_DRIVERS_H
#define INCLUDE_GUARD_COMMS_DRIVERS_H

#include <stdint.h>

extern uint8_t COMMS_current_active_antenna;

void COMMS_set_dipole_switch_state(uint8_t dipole_switch_antenna_num);

uint8_t COMMS_persistant_dipole_logic();

uint8_t COMMS_find_optimal_antenna();

#endif // INCLUDE_GUARD_COMMS_DRIVERS_H
