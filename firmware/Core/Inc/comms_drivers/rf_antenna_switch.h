#ifndef INCLUDE_GUARD__RF_ANTENNA_SWITCH
#define INCLUDE_GUARD__RF_ANTENNA_SWITCH

#include <stdint.h>

typedef enum {
    COMMS_RF_SWITCH_CONTROL_MODE_TOGGLE_BEFORE_EVERY_BEACON = 0, // Default.
    COMMS_RF_SWITCH_CONTROL_MODE_FORCE_ANT1 = 1,
    COMMS_RF_SWITCH_CONTROL_MODE_FORCE_ANT2 = 2,
    COMMS_RF_SWITCH_CONTROL_MODE_USE_ADCS_NORMAL = 3,
    COMMS_RF_SWITCH_CONTROL_MODE_USE_ADCS_FLIPPED = 4, // Redundant implementation that assumes the antennas are backwards.
    // TODO: Implement a RSSI auto-selection mode, where it selects based on the strongest uplink RSSI
    COMMS_RF_SWITCH_CONTROL_MODE_UNKNOWN = 255, // Used for error handling in string parser.
} COMMS_rf_switch_control_mode_enum_t;

extern uint8_t COMMS_active_rf_switch_antenna;
extern COMMS_rf_switch_control_mode_enum_t COMMS_rf_switch_control_mode;
extern uint32_t CONFIG_max_duration_without_uplink_before_setting_default_rf_switch_mode_sec;

void COMMS_set_rf_switch_state(uint8_t antenna_num);

uint8_t COMMS_find_optimal_antenna_using_adcs();

COMMS_rf_switch_control_mode_enum_t COMMS_rf_switch_control_mode_enum_from_string(const char *str);
const char* COMMS_rf_switch_control_mode_enum_to_string(COMMS_rf_switch_control_mode_enum_t mode);


#endif // INCLUDE_GUARD__RF_ANTENNA_SWITCH
