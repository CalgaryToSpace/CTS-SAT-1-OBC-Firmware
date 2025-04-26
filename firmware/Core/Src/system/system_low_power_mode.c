#include "system/system_low_power_mode.h"
#include "eps_drivers/eps_channel_control.h"
#include "eps_drivers/eps_commands.h"
#include "eps_drivers/eps_calculations.h"
#include "adcs_drivers/adcs_commands.h"
#include "boom_deploy_drivers/boom_deploy_drivers.h"
#include "log/log.h"

#include <stdio.h>
#include <string.h>

/// @brief Enters low power mode by disabling all non-essential channels, disabling Boom pins
/// @note Essential channels are: EPS, OBC, COMMS
/// @note GPS and ADCS Channels are always on, must enter "low power mode" instead of disabling channels
/// @return 0 on success, more than 0 on failure.
uint8_t SYS_enter_low_power_mode()
{
    uint8_t error_ret = 0;
    // TODO: Disable GPS

    // Disable ADCS: 

    // There are 2 "low power modes" that we can place the ADCS in.
    // - Stable-attitude mode: The ADCS will be stable, but all other internals are turned off.
    // - All-Peripheral turned off mode: The ADCS will not be stable (y-thomson spin AKA slowly de-tumbling) and all other internals are turned off.
    // For now, we will enter All-Peripheral turned off mode, which will be the lowest power mode.
    const uint8_t disable_adcs = ADCS_disable_peripherals_and_SD_logs_without_stabilisation();
    if (disable_adcs != 0) {
        error_ret |= SYS_LOW_POWER_MODE_ERROR_ADCS;
    }

    // Disable MPI
    const uint8_t disable_mpi_5v = EPS_set_channel_enabled(EPS_CHANNEL_5V_MPI, 0);
    if (disable_mpi_5v != 0) {
        error_ret |= SYS_LOW_POWER_MODE_ERROR_MPI_5V;
    }

    const uint8_t disable_mpi_12v = EPS_set_channel_enabled(EPS_CHANNEL_12V_MPI, 0);
    if (disable_mpi_12v != 0) {
        error_ret |= SYS_LOW_POWER_MODE_ERROR_MPI_12V;
    }

    // Disable Camera
    const uint8_t disable_camera = EPS_set_channel_enabled(EPS_CHANNEL_3V3_CAMERA, 0);
    if (disable_camera != 0) {
        error_ret |= SYS_LOW_POWER_MODE_ERROR_CAMERA;
    }

    // Disable Boom
    const uint8_t disable_boom_12v = EPS_set_channel_enabled(EPS_CHANNEL_12V_BOOM, 0);
    if (disable_boom_12v != 0) {
        error_ret |= SYS_LOW_POWER_MODE_ERROR_BOOM_12V;
    }

    // This does not return anything, 
    BOOM_disable_all_burns();

    const GPIO_PinState pin1_state = HAL_GPIO_ReadPin(PIN_BOOM_DEPLOY_EN_1_OUT_GPIO_Port, PIN_BOOM_DEPLOY_EN_1_OUT_Pin);
    const GPIO_PinState pin2_state = HAL_GPIO_ReadPin(PIN_BOOM_DEPLOY_EN_2_OUT_GPIO_Port, PIN_BOOM_DEPLOY_EN_2_OUT_Pin);
    
    if (pin1_state == GPIO_PIN_SET || pin2_state == GPIO_PIN_SET) {
        error_ret |= SYS_LOW_POWER_MODE_ERROR_BOOM_PINS;
    }

    return error_ret;
}

/// @brief Checks if the EPS is in emergency-low-power mode and enters low power mode if it is.
/// @note This function is called in the background upkeep task.
/// @return 0 on success, 1 if getting eps system status fails
uint8_t SYS_check_eps_and_enter_low_power_mode() 
{
    EPS_struct_system_status_t EPS_system_status;
    const uint8_t eps_status = EPS_CMD_get_system_status(&EPS_system_status);
    if (eps_status != 0) {
        LOG_message(
            LOG_SYSTEM_EPS,
            LOG_SEVERITY_ERROR,
            LOG_SINK_ALL,
            "SYS LOW POWER MODE CHECK -> EPS_CMD_get_system_status() -> Error: %d", eps_status
        );
        return 1;
    }
    
    // mode 0 = startup, 1 = nominal, 2 = safety, 3 = emergency low power
    if (EPS_system_status.mode == 2 || EPS_system_status.mode == 3) {
        LOG_message(
            LOG_SYSTEM_EPS,
            LOG_SEVERITY_CRITICAL,
            LOG_SINK_ALL,
            "SYS LOW POWER MODE CHECK -> EPS System Status mode: %u. Entering Low Power Mode!",
            EPS_system_status.mode
        );
    
        const uint8_t result_low_power_mode = SYS_enter_low_power_mode();
        char enter_low_power_mode_json[LOW_POWER_MODE_JSON_STRING_LEN] = {0};
        // Not checking return because buffer is
        // definitely not null and size is definitely >= low_power_mode_json_string_len
        SYS_low_power_mode_result_to_json(result_low_power_mode, enter_low_power_mode_json, sizeof(enter_low_power_mode_json));
        LOG_message(
            LOG_SYSTEM_OBC,
            result_low_power_mode ? LOG_SEVERITY_ERROR : LOG_SEVERITY_NORMAL,
            LOG_SINK_ALL,
            "%s",
            enter_low_power_mode_json
        );
    }
    return 0;
}

/// @brief Checks if the battery percentage is below 10% and enters low power mode if it is.
/// @note This function is called in the background upkeep task.
/// @return 0 on success, 1 if getting housekeeping data fails
uint8_t SYS_check_battery_and_enter_low_power_mode() 
{
    EPS_struct_pbu_housekeeping_data_eng_t data;
    const uint8_t result = EPS_CMD_get_pbu_housekeeping_data_eng(&data);
    if (result != 0) {
        LOG_message(
            LOG_SYSTEM_EPS,
            LOG_SEVERITY_ERROR,
            LOG_SINK_ALL,
            "SYS LOW POWER MODE CHECK -> EPS_CMD_get_pbu_housekeeping_data_eng (err %d)", result
        );
        return 1;
    }

    const float battery_percent = EPS_convert_battery_voltage_to_percent(data.battery_pack_info_each_pack[0]);
    // If the EPS battery measurement fails (i.e., returns 0 mV = -300%), then disregard the failing measurement.
    if (battery_percent > -99.0f && battery_percent < 10.0f) {
        LOG_message(
            LOG_SYSTEM_EPS,
            LOG_SEVERITY_CRITICAL,
            LOG_SINK_ALL,
            "SYS LOW POWER MODE CHECK -> Battery percentage is low: %0.2f%%. Entering Low Power Mode!", battery_percent
        );
    
        const uint8_t result_low_power_mode = SYS_enter_low_power_mode();
        char enter_low_power_mode_json[LOW_POWER_MODE_JSON_STRING_LEN] = {0};
        // Not checking return because buffer is
        // definitely not null and size is definitely >= low_power_mode_json_string_len
        SYS_low_power_mode_result_to_json(result_low_power_mode, enter_low_power_mode_json, sizeof(enter_low_power_mode_json));
        LOG_message(
            LOG_SYSTEM_OBC,
            result_low_power_mode ? LOG_SEVERITY_ERROR : LOG_SEVERITY_NORMAL,
            LOG_SINK_ALL,
            "%s",
            enter_low_power_mode_json
        );

    }
    return 0;
}

/// @brief Given mask value of low power mode, returns respective system
/// @param error_mask 
/// @return String of subsystem. Ex: "GPS", "MPI 12V"
char *SYS_low_power_mode_error_enum_to_string(SYS_low_power_mode_error_enum_t error_mask)
{
    switch (error_mask) {
        case SYS_LOW_POWER_MODE_ERROR_GPS:
            return "GPS";
        case SYS_LOW_POWER_MODE_ERROR_ADCS:
            return "ADCS";
        case SYS_LOW_POWER_MODE_ERROR_MPI_5V:
            return "MPI_5V";
        case SYS_LOW_POWER_MODE_ERROR_MPI_12V:
            return "MPI_12V";
        case SYS_LOW_POWER_MODE_ERROR_CAMERA:
            return "CAMERA";
        case SYS_LOW_POWER_MODE_ERROR_BOOM_12V:
            return "BOOM_12V";
        case SYS_LOW_POWER_MODE_ERROR_BOOM_PINS:
            return "BOOM_PINS";
        default:
            return "Unknown"; // Unknown error
    }
}

/// @brief Converts the error result of low power mode to a JSON string, showing whether shutting down that system was a success or failure
/// @param error Value of the error result from entering low power mode
/// @param buffer Resulting JSON string
/// @param buffer_size Size of the buffer, minimum LOW_POWER_MODE_JSON_STR_LEN
/// @return 0 on success, 1 on failure
uint8_t SYS_low_power_mode_result_to_json(SYS_low_power_mode_error_enum_t error, char *buffer, uint16_t buffer_size) 
{
    if (buffer == NULL || buffer_size < LOW_POWER_MODE_JSON_STRING_LEN) {
        return 1;
    }

    uint16_t offset = 0;
    offset += snprintf(buffer + offset, buffer_size - offset, "{");

    // loop through all systems disabled by channels
    for (uint8_t i = 0; i < LOW_POWER_MODE_DISABLED_CHANNEL_COUNT; i++) {
        SYS_low_power_mode_error_enum_t error_mask = (SYS_low_power_mode_error_enum_t)(1 << i);
        const char *subsystem = SYS_low_power_mode_error_enum_to_string(error_mask);
        const char *shutdown_status = (error & error_mask) ? "Error Disabling" : "Successfully Disabled";

        if (i != 0) {
            offset += snprintf(buffer + offset, buffer_size - offset, ", ");
        }

        offset += snprintf(buffer + offset, buffer_size - offset, "\"%s\":\"%s\"", subsystem, shutdown_status);
    }

    const uint8_t BOOM_pins_error = (error & SYS_LOW_POWER_MODE_ERROR_BOOM_PINS);
    offset += snprintf(buffer + offset, 
                       buffer_size - offset, 
                       ", \"BOOM_PINS\":\"%s\"",
                       BOOM_pins_error ? 
                       "Error Disabling Boom Pins" :
                       "Successfully Disabled Boom Pins");

    const uint8_t GPS_error = (error & SYS_LOW_POWER_MODE_ERROR_GPS);
    offset += snprintf(buffer + offset, 
                       buffer_size - offset, 
                       ", \"GPS\":\"%s\"",
                       GPS_error ? 
                       "Error Setting Low Power Mode" :
                       "Successfully Set Low Power Mode");

    const uint8_t ADCS_error = (error & SYS_LOW_POWER_MODE_ERROR_ADCS);
    offset += snprintf(buffer + offset, 
                       buffer_size - offset,
                       ", \"ADCS\":\"%s\"",
                       ADCS_error ? 
                       "Error Setting Low Power Mode" :
                       "Successfully Set Low Power Mode");

    snprintf(buffer + offset, buffer_size - offset, "}");

    return 0;
}