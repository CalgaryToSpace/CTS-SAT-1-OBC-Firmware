#include <eps_drivers/eps_commands.h>
#include <eps_drivers/eps_types.h>
#include <eps_drivers/eps_types_to_json.h>

#include <telecommands/eps_telecommands.h>

#include <stdio.h>

/// @brief Triggers/services the EPS watchdog. No args.
/// @param args_str No arguments.
/// @return 0 on success, 1 on failure.
uint8_t TCMDEXEC_eps_watchdog(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    const uint8_t result = EPS_CMD_watchdog();

    if (result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "EPS_CMD_watchdog() failed (err %d)", result);
        return 1;
    }

    snprintf(response_output_buf, response_output_buf_len, "EPS_CMD_watchdog() successful.");
    return 0;
}

/// @brief Resets the EPS system.
/// @param args_str No arguments.
/// @return 0 on success, 1 on failure.
uint8_t TCMDEXEC_eps_system_reset(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    const uint8_t result = EPS_CMD_system_reset();

    if (result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "EPS_CMD_system_reset() failed (err %d)", result);
        return 1;
    }

    snprintf(response_output_buf, response_output_buf_len, "EPS_CMD_system_reset() successful.");
    return 0;
}

/// @brief Calls the EPS no-op (no operation) command. Likely not useful.
/// @param args_str No arguments.
/// @return 0 on success, 1 on failure.
uint8_t TCMDEXEC_eps_no_operation(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) { 
    const uint8_t result = EPS_CMD_no_operation();

    if (result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "EPS_CMD_no_operation() failed (err %d)", result);
        return 1;
    }

    snprintf(response_output_buf, response_output_buf_len, "EPS_CMD_no_operation() successful.");
    return 0;
}

/// @brief Calls the EPS cancel operation command. Likely not useful.
/// @param args_str No arguments.
/// @return 0 on success, 1 on failure.
/// @note This command likely isn't useful, as telecommands cannot be trigger simultaneously, and
///     thus another command to the EPS cannot really be cancelled from here.
uint8_t TCMDEXEC_eps_cancel_operation(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    const uint8_t result = EPS_CMD_cancel_operation();

    if (result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "EPS_CMD_cancel_operation() failed (err %d)", result);
        return 1;
    }

    snprintf(response_output_buf, response_output_buf_len, "EPS_CMD_cancel_operation() successful.");
    return 0;
}

/// @brief Switches the EPS to nominal mode.
/// @param args_str No arguments.
/// @return 0 on success, 1 on failure.
uint8_t TCMDEXEC_eps_switch_to_nominal_mode(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) { 
    const uint8_t result = EPS_CMD_switch_to_nominal_mode();

    if (result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "EPS_CMD_switch_to_nominal_mode() failed (err %d)", result);
        return 1;
    }

    snprintf(response_output_buf, response_output_buf_len, "EPS_CMD_switch_to_nominal_mode() successful.");
    return 0;
}

/// @brief Switches the EPS to safety mode.
/// @param args_str No arguments.
/// @return 0 on success, 1 on failure.
uint8_t TCMDEXEC_eps_switch_to_safety_mode(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) { 
    const uint8_t result = EPS_CMD_switch_to_safety_mode();

    if (result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "EPS_CMD_switch_to_safety_mode() failed (err %d)", result);
        return 1;
    }

    snprintf(response_output_buf, response_output_buf_len, "EPS_CMD_switch_to_safety_mode() successful.");
    return 0;
}

/// @brief Gets the EPS system status, and returns it as a JSON string.
/// @return 0 on success, >0 on failure.
uint8_t TCMDEXEC_eps_get_system_status_json(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    
    EPS_result_system_status_t status;
    const uint8_t result = EPS_CMD_get_system_status(&status);

    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS get system status failed (err %d)", result);
        return 1;
    }

    const uint8_t result_json = EPS_result_system_status_TO_json(
        &status, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS get system status JSON failed (err %d)", result_json);
        return 2;
    }
    return 0;
}
