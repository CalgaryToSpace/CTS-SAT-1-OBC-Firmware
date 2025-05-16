#include "comms_drivers/comms_drivers.h"
#include "log/log.h"
#include "main.h"

uint8_t COMMS_current_active_antenna = 1; // Default: LOW. Thus, Antenna 1.

/// @brief Sets the state of the dipole switch on the OBC to either Antenna 1 or Antenna 2.
/// @param dipole_switch_antenna_num Either 1 or 2. If not 1 or 2, then remains unchanged.
/// @note Specification in https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/417.
void COMMS_set_dipole_switch_state(uint8_t dipole_switch_antenna_num) {
    if (dipole_switch_antenna_num == 1) {
        HAL_GPIO_WritePin(PIN_UHF_CTRL_OUT_GPIO_Port, PIN_UHF_CTRL_OUT_Pin, GPIO_PIN_RESET);
        COMMS_current_active_antenna = 1;
    }
    else if (dipole_switch_antenna_num == 2) {
        HAL_GPIO_WritePin(PIN_UHF_CTRL_OUT_GPIO_Port, PIN_UHF_CTRL_OUT_Pin, GPIO_PIN_SET);
        COMMS_current_active_antenna = 2;
    }
    else {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Invalid antenna number. Must be 1 or 2."
        );
    }
}
