#include "system/system_safe_mode.h"
#include "eps_drivers/eps_channel_control.h"
#include "log/log.h"


/// @brief Enters safe mode by disabling all non-essential channels.
/// @note Essential channels are: EPS, OBC, COMMS
/// @note GPS and ADCS Channels are always on, must enter "low power mode" instead of disabling channels
/// @return 0 on success, more than 0 on failure.
uint8_t SYS_enter_safe_mode()
{
    // disable GPS


    // disable ADCS 


    // disable MPI
    const uint8_t disable_mpi_5v = EPS_set_channel_enabled(EPS_CHANNEL_5V_MPI, 0);
    if (disable_mpi_5v != 0) {
        return 3;
    }

    const uint8_t disable_mpi_12v = EPS_set_channel_enabled(EPS_CHANNEL_12V_MPI, 0);
    if (disable_mpi_12v != 0) {
        return 4;
    }

    // disable Camera
    const uint8_t disable_camera = EPS_set_channel_enabled(EPS_CHANNEL_3V3_CAMERA, 0);
    if (disable_camera != 0) {
        return 5;
    }

    // disable Boom
    const uint8_t disable_boom = EPS_set_channel_enabled(EPS_CHANNEL_12V_BOOM, 0);
    if (disable_boom != 0) {
        return 6;
    }

    return 0;
}

/// @brief Checks if the EPS is in low power mode and enters safe mode if it is.
/// @note This function is called in the background upkeep task.
/// @return 0 on success, 1 if getting system status fails, more than 1 on failure to enter safe mode.
uint8_t SYS_eps_status_safe_mode_check() {
        const EPS_struct_system_status_t EPS_system_status;
        const uint8_t eps_status = EPS_CMD_get_system_status(&EPS_system_status);
        if (eps_status != 0) {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_ERROR,
                LOG_SINK_ALL,
                "SYS SAFE MODE CHECK -> EPS_CMD_get_system_status() -> Error: %d", eps_status
            );
            return 1;
        }
        
        // mode 0 = startup, 1 = nominal, 2 = safety, 3 = emergency low power
        if (EPS_system_status.mode == 3) {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_CRITICAL,
                LOG_SINK_ALL,
                "SYS SAFE MODE CHECK -> EPS in low power mode. Entering Safe Mode!",
                EPS_system_status.uptime_sec
            );

        const uint8_t result_safe_mode = SYS_enter_safe_mode();
        return result_safe_mode == 0 ? 0 : 1 + result_safe_mode;
        }
}

/// @brief Checks if the battery percentage is below 10% and enters safe mode if it is.
/// @note This function is called in the background upkeep task.
/// @return 0 on success, 1 if getting housekeeping data fails, more than 1 on failure to enter safe mode.
uint8_t SYS_battery_safe_mode_check() {
        EPS_struct_pbu_housekeeping_data_eng_t data;
        const uint8_t result = EPS_CMD_get_pbu_housekeeping_data_eng(&data);
        if (result != 0) {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_ERROR,
                LOG_SINK_ALL,
                "SYS SAFE MODE CHECK -> EPS_CMD_get_pbu_housekeeping_data_eng (err %d)", result
            );
            return 1;
        }

        const float battery_percent = EPS_convert_battery_voltage_to_percent(data.battery_pack_info_each_pack[0]);
        if (battery_percent < 10.0) {
            LOG_message(
                LOG_SYSTEM_EPS,
                LOG_SEVERITY_CRITICAL,
                LOG_SINK_ALL,
                "SYS SAFE MODE CHECK -> Battery percentage is low: %0.2f%%. Entering Safe Mode!", battery_percent
            );
        }
        const uint8_t result_safe_mode = SYS_enter_safe_mode();
        return result_safe_mode == 0 ? 0 : 1 + result_safe_mode;
}
