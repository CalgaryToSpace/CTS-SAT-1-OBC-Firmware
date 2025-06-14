#include "eps_drivers/eps_commands.h"
#include "eps_drivers/eps_types.h"
#include "eps_drivers/eps_types_to_json.h"
#include "eps_drivers/eps_channel_control.h"
#include "eps_drivers/eps_time.h"
#include "eps_drivers/eps_calculations.h"
#include "eps_drivers/eps_power_management.h"
#include "telecommands/eps_telecommands.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "log/log.h"
#include "obc_systems/adc_vbat_monitor.h"
#include <stdio.h>
#include <string.h>

/// @brief Triggers/services the EPS watchdog. No args.
/// @param args_str No arguments.
/// @return 0 on success, 1 on failure.
uint8_t TCMDEXEC_eps_watchdog(
    const char *args_str,
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
    const char *args_str,
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
    const char *args_str,
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
    const char *args_str,
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
/// - Arg 0: New mode to switch to. Either "nominal" or "safety". Case-insensitive.
/// @return 0 on success, 1 on failure.
/// @note See EPS Software ICD, Page 12, Section 3 (Functional Description) for state/mode definitions.
uint8_t TCMDEXEC_eps_switch_to_mode(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    uint8_t result = 42;

    if (strcasecmp(args_str, "nominal") == 0) {
        result = EPS_CMD_switch_to_nominal_mode();
    }
    else if (strcasecmp(args_str, "safety") == 0) {
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
/// - Arg 0: The channel name or number (case-insensitive string).
/// - Arg 1: 1 to enable (power on), 0 to disable (power off)
/// @return 0 on success, >0 on failure
/// @note Channel name argument: A lowercase c-string of the channel name (e.g., "mpi"), or a number
/// representing the channel number (e.g., "1" or "16").
/// Valid string values: "vbatt_stack", "stack_5v", "stack_3v3", "camera", "uhf_antenna_deploy",
/// "gnss", "mpi_5v", "mpi_12v", "boom".
uint8_t TCMDEXEC_eps_set_channel_enabled(
    const char *args_str,
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
    char enabled_str[20] = "?";
    if (enabled_val_u64 == 0) {
        strcpy(enabled_str, "disable");
    }
    else if (enabled_val_u64 == 1) {
        strcpy(enabled_str, "enable");
    }
    else {
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

    // Convert to a nice string/channel number.
    const char *eps_channel_name_str = EPS_channel_to_str(eps_channel);
    const uint8_t eps_channel_num = (uint8_t) eps_channel;

    if (
        ((eps_channel == EPS_CHANNEL_3V3_STACK)
        || (eps_channel == EPS_CHANNEL_5V_STACK)
        || (eps_channel == EPS_CHANNEL_VBATT_STACK))
        && (enabled_val_u64 == 0)
    ) {
        LOG_message(
            LOG_SYSTEM_EPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "Cannot disable the stack channels: %s. Trying anyway.",
            eps_channel_name_str
        );
        return 8;
    }

    const uint8_t eps_result = EPS_set_channel_enabled(eps_channel, (uint8_t)enabled_val_u64);

    if (eps_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "EPS %s channel %d (%s) failed (err %d).",
            enabled_str,
            eps_channel_num, eps_channel_name_str,
            eps_result
        );
        return 5;
    }
    snprintf(
        response_output_buf, response_output_buf_len,
        // Note: The word "success" is intentionally omitted here, because attempting to disable
        // certain channels (e..g, the always-on stack channels) will return success, but will not
        // actually disable the channel.
        "EPS %s channel %d (%s) executed.",
        enabled_str,
        eps_channel_num, eps_channel_name_str
    );
    return 0;
}


/// @brief Get the EPS system status, and display it as a JSON string.
/// @return 0 on success, >0 on failure.
uint8_t TCMDEXEC_eps_get_system_status_json(
    const char *args_str,
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



/// @brief Get the EPS PDU (Power Distribution Unit) overcurrent fault status, and display it as a JSON string.
/// @return 0 on success, >0 on failure.
uint8_t TCMDEXEC_eps_get_pdu_overcurrent_fault_state_json(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    
    EPS_struct_pdu_overcurrent_fault_state_t status;
    const uint8_t result = EPS_CMD_get_pdu_overcurrent_fault_state(&status);

    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_CMD_get_pdu_overcurrent_fault_state (err %d)", result);
        return 1;
    }

    const uint8_t result_json = EPS_struct_pdu_overcurrent_fault_state_TO_json(
        &status, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_struct_pdu_overcurrent_fault_state_TO_json failed (err %d)", result_json);
        return 2;
    }
    return 0;
}


/// @brief Get the EPS PBU (Power Battery Unit) ABF placed status, and display it as a JSON string.
/// @return 0 on success, >0 on failure.
uint8_t TCMDEXEC_eps_get_pbu_abf_placed_state_json(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    
    EPS_struct_pbu_abf_placed_state_t status;
    const uint8_t result = EPS_CMD_get_pbu_abf_placed_state(&status);

    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_CMD_get_pbu_abf_placed_state (err %d)", result);
        return 1;
    }

    const uint8_t result_json = EPS_struct_pbu_abf_placed_state_TO_json(
        &status, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_struct_pbu_abf_placed_state_TO_json failed (err %d)", result_json);
        return 2;
    }
    return 0;
}




/// @brief Get the EPS PDU (Power Distribution Unit) housekeeping data, and display it as a JSON string.
/// @return 0 on success, >0 on failure.
uint8_t TCMDEXEC_eps_get_pdu_housekeeping_data_eng_json(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    
    EPS_struct_pdu_housekeeping_data_eng_t status;
    const uint8_t result = EPS_CMD_get_pdu_housekeeping_data_eng(&status);

    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_CMD_get_pdu_housekeeping_data_eng (err %d)", result);
        return 1;
    }

    const uint8_t result_json = EPS_struct_pdu_housekeeping_data_eng_TO_json(
        &status, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_struct_pdu_housekeeping_data_eng_TO_json failed (err %d)", result_json);
        return 2;
    }
    return 0;
}


/// @brief Gets the Voltage, Current, and Power for a single channel on the EPS.
/// @param args_str 
/// - Arg 0: The channel name or number (case-insensitive string).
/// @return 0 on success, >0 on failure.
/// @note Channel name argument: A lowercase c-string of the channel name (e.g., "mpi"), or a number
/// representing the channel number (e.g., "1" or "16").
/// Valid string values: "vbatt_stack", "stack_5v", "stack_3v3", "camera", "uhf_antenna_deploy",
/// "gnss", "mpi_5v", "mpi_12v", "boom".
uint8_t TCMDEXEC_eps_get_pdu_data_for_channel_json(
    const char *args_str,
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

    // Convert the channel string to an enum value.
    const EPS_CHANNEL_enum_t eps_channel = EPS_channel_from_str(channel_str);
    if (eps_channel == EPS_CHANNEL_UNKNOWN) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Unknown channel: %s", channel_str);
        return 4;
    }

    // Convert to a nice channel number
    const uint8_t eps_channel_num = (uint8_t) eps_channel;

    // Define the destination of where data is written into 
    EPS_struct_pdu_housekeeping_data_eng_t data;

    // Get data from EPS
    const uint8_t result_from_eps = EPS_CMD_get_pdu_housekeeping_data_eng(&data);

    if (result_from_eps != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_CMD_get_pdu_housekeeping_data_eng (err %d)", result_from_eps);
        return 1;
    }

    // Format the data to JSON string
    const uint8_t result_json = EPS_struct_single_channel_data_eng_TO_json(
        &data, eps_channel_num, response_output_buf, response_output_buf_len);
    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_struct_single_channel_data_eng_TO_json failed (err %d)", result_json);
        return 2;
    }
    return 0;
}


/// @brief Get the EPS PDU (Power Distribution Unit) housekeeping data (running average), and display it as a JSON string.
/// @return 0 on success, >0 on failure.
uint8_t TCMDEXEC_eps_get_pdu_housekeeping_data_run_avg_json(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    
    EPS_struct_pdu_housekeeping_data_eng_t status;
    const uint8_t result = EPS_CMD_get_pdu_housekeeping_data_run_avg(&status);

    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_CMD_get_pdu_housekeeping_data_run_avg (err %d)", result);
        return 1;
    }

    const uint8_t result_json = EPS_struct_pdu_housekeeping_data_eng_TO_json(
        &status, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_struct_pdu_housekeeping_data_eng_TO_json failed (err %d)", result_json);
        return 2;
    }
    return 0;
}



/// @brief Get the EPS PBU (Power Battery Unit) housekeeping data, and display it as a JSON string.
/// @return 0 on success, >0 on failure.
uint8_t TCMDEXEC_eps_get_pbu_housekeeping_data_eng_json(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    
    EPS_struct_pbu_housekeeping_data_eng_t status;
    const uint8_t result = EPS_CMD_get_pbu_housekeeping_data_eng(&status);

    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_CMD_get_pbu_housekeeping_data_eng (err %d)", result);
        return 1;
    }

    const uint8_t result_json = EPS_struct_pbu_housekeeping_data_eng_TO_json(
        &status, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_struct_pbu_housekeeping_data_eng_TO_json failed (err %d)", result_json);
        return 2;
    }
    return 0;
}



/// @brief Get the EPS PBU (Power Battery Unit) housekeeping data (running average), and display it as a JSON string.
/// @return 0 on success, >0 on failure.
uint8_t TCMDEXEC_eps_get_pbu_housekeeping_data_run_avg_json(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    
    EPS_struct_pbu_housekeeping_data_eng_t status;
    const uint8_t result = EPS_CMD_get_pbu_housekeeping_data_run_avg(&status);

    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_CMD_get_pbu_housekeeping_data_run_avg (err %d)", result);
        return 1;
    }

    const uint8_t result_json = EPS_struct_pbu_housekeeping_data_eng_TO_json(
        &status, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_struct_pbu_housekeeping_data_eng_TO_json failed (err %d)", result_json);
        return 2;
    }
    return 0;
}



/// @brief Get the EPS PCU (Power Conditioning Unit, solar panel MPPT) housekeeping data, and display it as a JSON string.
/// @return 0 on success, >0 on failure.
uint8_t TCMDEXEC_eps_get_pcu_housekeeping_data_eng_json(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    
    EPS_struct_pcu_housekeeping_data_eng_t status;
    const uint8_t result = EPS_CMD_get_pcu_housekeeping_data_eng(&status);

    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_CMD_get_pcu_housekeeping_data_eng (err %d)", result);
        return 1;
    }

    const uint8_t result_json = EPS_struct_pcu_housekeeping_data_eng_TO_json(
        &status, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_struct_pcu_housekeeping_data_eng_TO_json failed (err %d)", result_json);
        return 2;
    }
    return 0;
}



/// @brief Get the EPS PCU (Power Conditioning Unit, solar panel MPPT) housekeeping data (running average), and display it as a JSON string.
/// @return 0 on success, >0 on failure.
uint8_t TCMDEXEC_eps_get_pcu_housekeeping_data_run_avg_json(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    
    EPS_struct_pcu_housekeeping_data_eng_t status;
    const uint8_t result = EPS_CMD_get_pcu_housekeeping_data_run_avg(&status);

    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_CMD_get_pcu_housekeeping_data_run_avg (err %d)", result);
        return 1;
    }

    const uint8_t result_json = EPS_struct_pcu_housekeeping_data_eng_TO_json(
        &status, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_struct_pcu_housekeeping_data_eng_TO_json failed (err %d)", result_json);
        return 2;
    }
    return 0;
}


/// @brief Gets the EPS PIU (Power Integrated Unit, info about all systems) housekeeping data, and returns it as a JSON string.
/// @return 0 on success, >0 on failure.
uint8_t TCMDEXEC_eps_get_piu_housekeeping_data_eng_json(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    
    EPS_struct_piu_housekeeping_data_eng_t status;
    const uint8_t result = EPS_CMD_get_piu_housekeeping_data_eng(&status);

    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_CMD_get_piu_housekeeping_data_eng (err %d)", result);
        return 1;
    }

    const uint8_t result_json = EPS_struct_piu_housekeeping_data_eng_TO_json(
        &status, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_struct_piu_housekeeping_data_eng_TO_json failed (err %d)", result_json);
        return 2;
    }
    return 0;
}



/// @brief Get the EPS PIU (Power Integrated Unit, info about all systems) housekeeping data (running average), and display it as a JSON string.
/// @return 0 on success, >0 on failure.
uint8_t TCMDEXEC_eps_get_piu_housekeeping_data_run_avg_json(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    
    EPS_struct_piu_housekeeping_data_eng_t status;
    const uint8_t result = EPS_CMD_get_piu_housekeeping_data_run_avg(&status);

    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_CMD_get_piu_housekeeping_data_run_avg (err %d)", result);
        return 1;
    }

    const uint8_t result_json = EPS_struct_piu_housekeeping_data_eng_TO_json(
        &status, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_struct_piu_housekeeping_data_eng_TO_json failed (err %d)", result_json);
        return 2;
    }
    return 0;
}

/// @brief Get current battery voltage percent from PBU
uint8_t TCMDEXEC_eps_get_current_battery_percent(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len) {

    EPS_struct_pbu_housekeeping_data_eng_t data;
    const uint8_t result = EPS_CMD_get_pbu_housekeeping_data_eng(&data);

    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_CMD_get_pbu_housekeeping_data_eng (err %d)", result);
        return 1;
    }

    const float battery_percent = EPS_convert_battery_voltage_to_percent(data.battery_pack_info_each_pack[0]);

    snprintf(response_output_buf, response_output_buf_len, 
             "Battery Percentage: %0.2f%%", battery_percent);

    
    return 0;    
}

uint8_t TCMDEXEC_eps_get_enabled_channels_json(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len) {

    EPS_struct_pdu_housekeeping_data_eng_t status;
    const uint8_t result = EPS_CMD_get_pdu_housekeeping_data_eng(&status);

    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS_CMD_get_piu_housekeeping_data_run_avg (err %d)", result);
        return 1;
    }

    EPS_get_enabled_channels_json(
        status.stat_ch_on_bitfield, status.stat_ch_ext_on_bitfield,
        response_output_buf, response_output_buf_len
    );

    return 0;
}
/// @brief Sets the EPS power managements max sustained current for the specified channel.
/// @param args_str 
/// - Arg 0: The channel name or number (case-insensitive string).
/// - Arg 1: Threshold current (in mA) to set.
/// @return 0 on success, >0 on failure
/// @note Valid string values (Arg 0): "vbatt_stack", "stack_5v", "stack_3v3", "camera",
/// "uhf_antenna_deploy", "gnss", "mpi_5v", "mpi_12v", "boom".
uint8_t TCMDEXEC_eps_power_management_set_current_threshold(
    const char *args_str,
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

    // Extract Arg 1: Threshold current (in mA) to set.
    uint64_t current_threshold = 0;
    const uint8_t arg_1_result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 1, &current_threshold);
    if (arg_1_result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Error parsing enabled arg: Error %d", arg_1_result);
        return 2;
    }

    // Convert the channel string to an enum value.
    const EPS_CHANNEL_enum_t eps_channel = EPS_channel_from_str(channel_str);
    if (eps_channel == EPS_CHANNEL_UNKNOWN) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Unknown channel: %s", channel_str);
        return 3;
    }

    EPS_CMD_power_management_set_current_threshold(eps_channel, current_threshold);
    snprintf(
        response_output_buf, response_output_buf_len,
        "EPS_CMD_power_management_set_current_threshold: Channel %s, %ld", channel_str,  (uint32_t) current_threshold);
    return 0;
}
