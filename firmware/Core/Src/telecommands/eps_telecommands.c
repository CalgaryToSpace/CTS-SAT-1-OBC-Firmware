#include "eps_drivers/eps_commands.h"
#include "eps_drivers/eps_types.h"
#include "eps_drivers/eps_types_to_json.h"
#include "eps_drivers/eps_channel_control.h"

#include "telecommands/eps_telecommands.h"
#include "telecommands/telecommand_args_helpers.h"

#include <stdio.h>
#include <string.h>

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

/// @brief Switches the EPS to "nominal" or "safety" mode.
/// @param args_str
/// - Arg 0: New mode to switch to. Either "nominal" or "safety".
/// @return 0 on success, 1 on failure.
uint8_t TCMDEXEC_eps_switch_to_mode(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    uint8_t result = 42;

    if (strcmp(args_str, "nominal") == 0) {
        result = EPS_CMD_switch_to_nominal_mode();
    }
    else if (strcmp(args_str, "safety") == 0) {
        result = EPS_CMD_switch_to_safety_mode();
    }
    else {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Invalid mode: %s", args_str);
        return 1;
    }

    if (result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "EPS_CMD_switch_to_%s_mode() failed (err %d)", args_str, result);
        return 1;
    }

    snprintf(
        response_output_buf, response_output_buf_len,
        "EPS_CMD_switch_to_%s_mode() successful.",
        args_str
    );
    return 0;
}

/// @brief Sets the EPS channel to be enabled (on) or disabled (off).
/// @param args_str 
/// - Arg 0: The channel name or number (string).
/// - Arg 1: 1 to enable (power on), 0 to disable (power off)
/// @return 0 on success, >0 on failure
/// @note Valid string values for Arg 0: "vbatt_stack", "5v_stack", "3v3_stack", "camera",
///     "uhf_antenna_deploy", "lora_module", "mpi", "boom".
uint8_t TCMDEXEC_eps_set_channel_enabled(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    // Extract Arg 0: The channel name/number.
    char channel_str[30];
    const uint8_t arg_0_result = TCMD_extract_string_arg(args_str, 0, channel_str, sizeof(channel_str));
    if (arg_0_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing channel arg: Error %d", arg_0_result);
        return 1;
    }

    // Extract Arg 1: 1 to enable (power on), 0 to disable (power off)
    uint64_t enabled_val_u64 = 42;
    const uint8_t arg_1_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &enabled_val_u64);
    if (arg_1_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing enabled arg: Error %d", arg_1_result);
        return 2;
    }
    if (enabled_val_u64 != 0 && enabled_val_u64 != 1) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing enabled arg: Must be 0 or 1");
        return 3;
    }
    
    // Convert the channel string to an enum value.
    const EPS_CHANNEL_enum_t eps_channel = EPS_channel_from_str(channel_str);
    if (eps_channel == EPS_CHANNEL_UNKNOWN) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Unknown channel: %s", channel_str);
        return 4;
    }


    const uint8_t eps_result = EPS_set_channel_enabled(eps_channel, (uint8_t)enabled_val_u64);

    if (eps_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "EPS set channel enabled failed (err %d)", eps_result);
        return 5;
    }
    snprintf(
        response_output_buf, response_output_buf_len,
        "EPS set channel enabled successful.");
    return 0;
}


/// @brief Gets the EPS system status, and returns it as a JSON string.
/// @return 0 on success, >0 on failure.
uint8_t TCMDEXEC_eps_get_system_status_json(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    
    EPS_struct_system_status_t status;
    const uint8_t result = EPS_CMD_get_system_status(&status);

    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_CMD_get_system_status (err %d)", result);
        return 1;
    }

    const uint8_t result_json = EPS_struct_system_status_TO_json(
        &status, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_struct_system_status_TO_json failed (err %d)", result_json);
        return 2;
    }
    return 0;
}

