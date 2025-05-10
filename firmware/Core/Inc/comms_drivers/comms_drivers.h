#ifndef INCLUDE_GUARD_COMMS_DRIVERS_H
#define INCLUDE_GUARD_COMMS_DRIVERS_H

#include <stdint.h>

typedef enum {
    COMMS_ANTENNA_SELECTION_MODE_USE_ADCS = 0,
    COMMS_ANTENNA_SELECTION_MODE_TOGGLE_BEFORE_EVERY_BEACON = 1,
    COMMS_ANTENNA_SELECTION_MODE_OVERRIDE_BY_TELECOMMAND_FOR_30_MINUTES  = 2,
    COMMS_ANTENNA_SELECTION_MODE_UNKNOWN = 255
} COMMS_antenna_selection_mode_enum_t;

extern uint8_t COMMS_current_active_antenna;
extern COMMS_antenna_selection_mode_enum_t COMMS_current_ant_mode;

void COMMS_set_dipole_switch_state(uint8_t dipole_switch_antenna_num);

void COMMS_determine_and_update_dipole_antenna_switch(uint64_t current_time);

uint8_t COMMS_find_optimal_antenna();

void COMMS_toggle_active_antenna();

COMMS_antenna_selection_mode_enum_t COMMS_mode_from_str(const char channel_name[]);

#endif // INCLUDE_GUARD_COMMS_DRIVERS_H
