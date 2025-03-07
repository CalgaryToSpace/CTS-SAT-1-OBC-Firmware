#include "comms_drivers/comms_drivers.h"
#include "config/configuration.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"
#include "log/log.h"
#include "main.h"

uint8_t COMMS_current_active_antenna = 2; // Default: LOW. Thus, Antenna 2.

/// @brief Sets the state of the dipole switch on the OBC to either Antenna 1 or Antenna 2.
/// @param dipole_switch_antenna_num Either 1 or 2. If not 1 or 2, then remains unchanged.
/// @note Created per https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/226.
void COMMS_set_dipole_switch_state(uint8_t dipole_switch_antenna_num) {
    if (dipole_switch_antenna_num == 1) {
        HAL_GPIO_WritePin(PIN_UHF_CTRL_OUT_GPIO_Port, PIN_UHF_CTRL_OUT_Pin, GPIO_PIN_SET);
        COMMS_current_active_antenna = 1;
    }
    else if (dipole_switch_antenna_num == 2) {
        HAL_GPIO_WritePin(PIN_UHF_CTRL_OUT_GPIO_Port, PIN_UHF_CTRL_OUT_Pin, GPIO_PIN_RESET);
        COMMS_current_active_antenna = 2;
    }
    else {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Invalid antenna number. Must be 1 or 2."
        );
    }
}

uint8_t COMMS_check_connection() {
    if (COMMS_last_reponse_time < TIM_get_current_unix_epoch_time_ms() - max_time_since_last_response_ms) { //TODO: Add max_time_since_last_response_ms to configuration
        COMMS_set_dipole_switch_state(HAL_GPIO_ReadPin(PIN_UHF_CTRL_OUT_GPIO_Port, PIN_UHF_CTRL_OUT_Pin) ? 1 : 2);
        return 1;
    }
    //TODO: Set optimal antenna if not already set.
    return 0;
}