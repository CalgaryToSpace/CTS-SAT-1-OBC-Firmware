#include "main.h"
#include "log/log.h"
#include "log/lazy_file_log_sink.h"
#include "eps_drivers/eps_types.h"
#include "eps_drivers/eps_calculations.h"
#include "timekeeping/timekeeping.h"
#include "stm32/stm32_reboot_reason.h"
#include "eps_drivers/eps_commands.h"
#include "littlefs/littlefs_helper.h"
#include "transforms/arrays.h"
#include "self_checks/complete_self_check.h"
#include "obc_systems/external_led_and_rbf.h"
#include "obc_systems/obc_temperature_sensor.h"
#include "system/system_temperature.h"
#include "mpi/mpi_command_handling.h"
#include "mpi/mpi_types.h"
#include "uart_handler/uart_handler.h"
#include "rtos_tasks/rtos_bootup_operation_fsm_task.h"
#include "gnss_receiver/gnss_internal_drivers.h"
#include "uart_handler/uart_handler.h"

#include "telecommands/system_telecommand_defs.h"
#include "telecommand_exec/telecommand_definitions.h"
#include "telecommand_exec/telecommand_executor.h"
#include "telecommand_exec/telecommand_args_helpers.h"

#include <stdio.h>
#include <string.h>

extern uint32_t COMMS_total_beacon_count_since_boot;
extern MPI_transceiver_state_enum_t MPI_current_transceiver_state;
extern MPI_reason_for_stopping_active_mode MPI_last_reason_for_stopping_active_mode;

/// @brief A simple telecommand that responds with "Hello, world!" (log message and TCMD response)
/// @param args_str No arguments expected
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 if successful, >0 if an error occurred (but hello_world can't return an error)
uint8_t TCMDEXEC_hello_world(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    LOG_message(
        LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Hello, world!"
    );
    snprintf(
        response_output_buf, response_output_buf_len,
        "Hello, world"
    );
    return 0;
}

/// @brief Get the currently-running OBC Firmware Version (build date/time, comment).
/// @param args_str No arguments.
/// @return 0 on success (cannot fail).
uint8_t TCMDEXEC_obc_firmware_version(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
#ifdef __OPTIMIZE__
    const char* optimize_str = "O";
#else
    const char* optimize_str = "O0";
#endif

#ifdef NDEBUG
    const char* asserts_str = "off";
#else
    const char* asserts_str = "on";
#endif

    snprintf(
        response_output_buf,
        response_output_buf_len,
        "{"
        "\"build_date\":\"%s\","
        "\"build_time\":\"%s\","
        "\"compiler\":\"%s\","
        "\"optimize\":\"%s\","
        "\"asserts\":\"%s\","
        "\"comment\":\"%s\""
        "}",
        __DATE__, // "build_date"
        __TIME__, // "build_time"
        __VERSION__, // "compiler"
        optimize_str, // "optimize"
        asserts_str, // "asserts" (least-useful one)
        "dev" // "comment"
    );

    return 0;
}

/// @brief Get many essential system stats as a JSON dict.
/// @param args_str No arguments.
/// @return 0 on success. Cannot fail.
uint8_t TCMDEXEC_core_system_stats(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    char timestamp_string_ms[20];
    GEN_uint64_to_str(TIME_get_current_unix_epoch_time_ms(), timestamp_string_ms);

    const char* STM32_reset_cause_name = STM32_reset_cause_enum_to_str(STM32_get_reset_cause());

    char time_of_last_tcmd_sent_ms_string[20];
    GEN_uint64_to_str(TCMD_latest_received_tcmd_timestamp_sent, time_of_last_tcmd_sent_ms_string);

    // Get EPS battery percentage as float, or null if error.
    char eps_battery_percent_str[10] = "null";
    {
        EPS_struct_pbu_housekeeping_data_eng_t eps_pbu_data;
        const uint8_t eps_pbu_result = EPS_CMD_get_pbu_housekeeping_data_eng(&eps_pbu_data);
        if (eps_pbu_result == 0) {
            const float battery_percent = EPS_convert_battery_voltage_to_percent(
                eps_pbu_data.battery_pack_info_each_pack[0]
            );
            snprintf(eps_battery_percent_str, sizeof(eps_battery_percent_str), "%0.2f", battery_percent);
        }
    }

    // Get EPS total fault count.
    int32_t eps_total_fault_count = -1;
    {
        EPS_struct_pdu_overcurrent_fault_state_t eps_pdu_fault_data;
        const uint8_t eps_pdu_result = EPS_CMD_get_pdu_overcurrent_fault_state(&eps_pdu_fault_data);
        if (eps_pdu_result == 0) {
            eps_total_fault_count = EPS_calculate_total_fault_count(&eps_pdu_fault_data);
        }
    }

    snprintf(
        response_output_buf, response_output_buf_len, 
        "{"
        "\"timestamp_ms\":%s,"
        "\"uptime_ms\":%lu,"
        "\"systick_ms\":%lu,"
        "\"last_time_resync_ms\":%lu,"
        "\"time_synced_ms_ago\":%lu,"
        "\"last_time_sync_source\":\"%c\","
        "\"last_tcmd_tssent_ms\":%s,"
        "\"total_tcmd_count\":%lu,"
        "\"pending_tcmd_count\":%u,"
        "\"total_beacon_count\":%lu,"
        "\"is_lfs_mounted\":%u,"
        "\"reboot_reason\":\"%s\","
        "\"sys_clock_speed_hz\":%lu,"
        "\"operation_state\":\"%s\","
        "\"obc_temperature_cC\":%ld,"
        "\"mpi_rx_mode\":\"%s\","
        "\"mpi_transceiver_state\":\"%s\","
        "\"mpi_last_reason_for_stopping\":\"%s\","
        "\"gnss_uart_interrupt_enabled\":%u,"
        "\"gnss_rx_mode\":\"%s\","
        "\"eps_total_fault_count\":%lu,"
        "\"eps_battery_percent\":%s"
        "}",
        timestamp_string_ms, // timestamp_ms
        TIME_uptime_ms(), // uptime_ms
        HAL_GetTick(), // systick_ms (like uptime, but prone to running slowly)
        TIME_system_uptime_at_last_time_resync_ms, // last_time_resync_ms
        TIME_uptime_ms() - TIME_system_uptime_at_last_time_resync_ms, // time_synced_ms_ago
        TIME_sync_source_enum_to_letter_char(TIME_last_synchronization_source), // last_time_sync_source
        time_of_last_tcmd_sent_ms_string, // last_tcmd_tssent_ms (not actually that useful - as-reported from the ground)
        TCMD_total_tcmd_queued_count, // total_tcmd_count
        TCMD_get_agenda_used_slots_count(), // pending_tcmd_count
        COMMS_total_beacon_count_since_boot, // total_beacon_count
        LFS_is_lfs_mounted, // is_lfs_mounted
        STM32_reset_cause_name, // reboot_reason
        SystemCoreClock, // sys_clock_speed_hz
        CTS1_operation_state_enum_TO_str(CTS1_operation_state), // operation_state
        OBC_TEMP_SENSOR_get_temperature_cC(), // obc_temperature_cC
        MPI_rx_mode_enum_to_str(MPI_current_uart_rx_mode), // mpi_rx_mode
        MPI_transceiver_state_enum_to_str(MPI_current_transceiver_state), // mpi_transceiver_state
        MPI_reason_for_stopping_active_mode_enum_to_str(MPI_last_reason_for_stopping_active_mode), // mpi_last_reason_for_stopping
        UART_gnss_uart_interrupt_enabled, // gnss_uart_interrupt_enabled
        GNSS_rx_mode_enum_to_str(GNSS_current_rx_mode), // gnss_rx_mode
        eps_total_fault_count, // eps_total_fault_count
        eps_battery_percent_str // eps_battery_percent
    ); 

    return 0;
}

uint8_t TCMDEXEC_available_telecommands(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    char *p = response_output_buf;
    uint16_t remaining_space = response_output_buf_len;

    // Start response with header
    snprintf(p, remaining_space, "Available_telecommands\n");
    const uint16_t header_length = strlen(p);
    p += header_length;
    remaining_space -= header_length;

    // Append each telecommand name to the response
    for (uint16_t tcmd_idx = 0; tcmd_idx < TCMD_NUM_TELECOMMANDS; tcmd_idx++) {
        const uint16_t line_length = snprintf(
            p,
            remaining_space,
            "%3u) %s\n",
            tcmd_idx + 1,
            TCMD_telecommand_definitions[tcmd_idx].tcmd_name
        );
        if (line_length >= remaining_space) {
            // Not enough space left to append more telecommands
            break;
        }
        p += line_length;
        remaining_space -= line_length;
    }

    return 0;
}


/// @brief Reboot the OBC. Unmounts the filesystem first for safety.
/// @param args_str 
/// @param response_output_buf 
/// @param response_output_buf_len 
/// @return 
uint8_t TCMDEXEC_reboot(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    LOG_message(
        LOG_SYSTEM_OBC, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Rebooting by telecommand request."
    );

    // If logging to file is enabled, flush the current log file.
    // Not an emergency, but the "emergency" one is good for a pending reboot.
    LOG_emergency_sync_current_log_file();
    
    // Delay to flush log sinks if enabled.
    HAL_Delay(500);

    LFS_ensure_unmounted();

    // Delay to flush UART buffer.
    HAL_Delay(100);

    // Actually reboot.
    NVIC_SystemReset();
    return 0; // Never reached.
}

/// @brief System self-check of all peripherals and systems.
/// @param args_str No arguments expected
/// @param response_output_buf Buffer is filled with a JSON list of strings of the FAILING checks
/// @return 0 regardless; see the response_output_buf for the results of the self-check.
/// @note Output is a JSON list of the failing checks (as strings). Returns 0 regardless.
uint8_t TCMDEXEC_system_self_check_failures_as_json(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    CTS1_system_self_check_result_struct_t self_check_result;
    CTS1_run_system_self_check(&self_check_result);
    CTS1_self_check_struct_TO_json_list(
        self_check_result, response_output_buf, response_output_buf_len,
        0 // Don't show passed checks.
    );
    return 0;
}

/// @brief System self-check of all peripherals and systems.
/// @param args_str No arguments expected
/// @param response_output_buf Buffer is filled with a JSON list of strings of the FAILING checks
/// @return 0 regardless; see the response_output_buf for the results of the self-check.
/// @note Output is a JSON list of the failing checks (as strings). Returns 0 regardless.
uint8_t TCMDEXEC_system_self_check_as_json(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    CTS1_system_self_check_result_struct_t self_check_result;
    CTS1_run_system_self_check(&self_check_result);
    CTS1_self_check_struct_TO_json_list(
        self_check_result, response_output_buf, response_output_buf_len,
        1 // Show passed checks.
    );
    return 0;
}

uint8_t TCMDEXEC_obc_get_rbf_state(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    const OBC_rbf_state_enum_t rbf_state = OBC_get_rbf_state();
    const char *rbf_state_str = (rbf_state == OBC_RBF_STATE_FLYING) ? "FLYING" : "BENCH";
    snprintf(
        response_output_buf, response_output_buf_len,
        "{\"rbf_state\":\"%s\"}",
        rbf_state_str
    );
    return 0;
}


/// @brief Get a variety of system thermal info as JSON.
/// @param args_str No arguments.
/// @return 0 on success.
/// @note Fields include OBC temp, antenna temps, solar panel generation, battery info.
/// @note The EPS_CHANNEL_3V3_UHF_ANTENNA_DEPLOY channel is powered off after,
///       even if it was powered on previously. Totally safe, but just an FYI.
uint8_t TCMDEXEC_get_all_system_thermal_info(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    SYS_TEMP_raw_thermal_info_t raw_temp_info;
    SYS_TEMP_thermal_info_t output_temp_info;
    uint8_t error_ret = 0;

    const uint8_t result = SYS_TEMP_get_raw_thermal_info(&raw_temp_info, &error_ret);
    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "SYS_TEMP_get_raw_thermal_info (err %d)", result);
        return result;
    }

    SYS_TEMP_pack_to_system_thermal_info(&raw_temp_info, &output_temp_info, error_ret);

    snprintf(
        response_output_buf, response_output_buf_len, 
        "{"
        "\"obc_temperature_cC\":%ld,"
        "\"ant_temperature_cC\":[%ld,%ld]," // Bus A, Bus B
        "\"solar_panel_input_cW\": [%ld,%ld,%ld,%ld],"
        "\"solar_panel_output_total_cW\":%ld,"
        "\"eps_battery_percent\":%0.02f,"
        "\"battery_heater_active\":%d,"
        "\"battery_sensor_temp_cC\": [%d,%d]"
        "}\n", 
        output_temp_info.system_OBC_temperature_cC,
        output_temp_info.system_ANT_temperature_i2c_bus_A_cC,
        output_temp_info.system_ANT_temperature_i2c_bus_B_cC,
        output_temp_info.solar_panel_power_input_cW[0],
        output_temp_info.solar_panel_power_input_cW[1],
        output_temp_info.solar_panel_power_input_cW[2],
        output_temp_info.solar_panel_power_input_cW[3],
        output_temp_info.solar_panel_power_output_total_cW,
        output_temp_info.system_eps_battery_percent,
        output_temp_info.system_eps_battery_heater_status_bit,
        output_temp_info.system_eps_battery_each_sensor_temperature_cC[1],
        output_temp_info.system_eps_battery_each_sensor_temperature_cC[2]
    );

    return 0;
}


typedef uint8_t (*blob_entry_t)(
    const char *args_str,
    char       *response_buf,
    uint16_t    response_buf_len
);


/// @brief Executes an arbitrary blob/program from the filesystem.
/// @param args_str
/// - Arg 0: File name of the blob
/// - Arg 1: Where to load blob (0=malloc, 1=mpi_buffer_one, 2=mpi_buffer_two)
/// - Arg 2: Argument to pass to the blob function (e.g., another filename)
/// @param response_output_buf The blob optionally writes its intermediate workings and results to this buffer.
/// @return 99 on pre-blob execution error. Otherwise, blob return value (presumably 0 on success).
/// @details Refer to `misc_tools/exec_blob/README.md` for more information on how to use this complex telecommand.
uint8_t TCMDEXEC_exec_blob_from_fs(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    char arg_file_name[LFS_MAX_PATH_LENGTH];
    const uint8_t parse_dir_name_result = TCMD_extract_string_arg(
        args_str, 0, arg_file_name, sizeof(arg_file_name)
    );
    uint64_t arg_where_to_load;
    const uint8_t parse_int_arg_result = TCMD_extract_uint64_arg(
        args_str, strlen(args_str), 1, &arg_where_to_load
    );
    if ((parse_dir_name_result != 0) || (parse_int_arg_result != 0) || (arg_where_to_load > 2)) {
        snprintf(
            response_output_buf,
            response_output_buf_len,
            "Error parsing args: err_filename=%d, err_num=%d, number_invalid=%d",
            parse_dir_name_result, parse_int_arg_result,
            (arg_where_to_load > 2)
        );
        return 99;
    }
    const uint8_t arg_file_name_len = strlen(arg_file_name);

    const char *args_str_to_blob = args_str + arg_file_name_len + 3; // Past comma, num_arg, comma:

    uint8_t* blob_buffer;
    uint32_t blob_buffer_size = 0;
    if (arg_where_to_load == 0) {
        // Determine buffer size from file size.
        blob_buffer_size = LFS_file_size(arg_file_name, 1) + 20; // Add 20 bytes because it feels reasonable.
        if (blob_buffer_size <= 0) {
            snprintf(
                response_output_buf, response_output_buf_len,
                "ERR: LFS_file_size failed (%ld)", blob_buffer_size
            );
            return 99;
        }

        // Malloc the buffer.
        // Note: portBYTE_ALIGNMENT must be 4 (or a multiple of 4) to ensure
        // instructions are aligned to half-words.
        blob_buffer = (uint8_t*)pvPortMalloc(blob_buffer_size);
        if (blob_buffer == NULL) {
            snprintf(
                response_output_buf, response_output_buf_len,
                "ERR: pvPortMalloc failed (%ld bytes)", blob_buffer_size
            );
            return 99;
        }
    }
    else if (arg_where_to_load == 1) {
        blob_buffer = (uint8_t*)MPI_science_buffer_one;
        blob_buffer_size = MPI_science_buffer_len;
    }
    else if (arg_where_to_load == 2) {
        blob_buffer = (uint8_t*)MPI_science_buffer_two;
        blob_buffer_size = MPI_science_buffer_len;
    }
    else {
        snprintf(
            response_output_buf, response_output_buf_len,
            "ERR: Invalid arg_where_to_load=%d",
            (uint8_t)arg_where_to_load
        );
        return 99;
    }

    memset(blob_buffer, 0, blob_buffer_size);
    const int32_t bytes_read = LFS_read_file(
        arg_file_name, 0, blob_buffer, blob_buffer_size
    );
    if (bytes_read <= 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "ERR: lfs_read_file failed (%ld)", bytes_read
        );
        return 99;
    }

    // Flush caches before executing from SRAM.
    __DSB();
    __ISB();

    // Execute from offset 0, +1 for Thumb mode.
    const blob_entry_t blob_entry = (blob_entry_t)((uint32_t)blob_buffer | 0x1U);
    const uint8_t blob_result = blob_entry(args_str_to_blob, response_output_buf, response_output_buf_len);

    // Safety: Ensure that the buffer is null-terminated, in case the blob forgot to.
    response_output_buf[response_output_buf_len - 1] = '\0';

    // Free the buffer, if we used malloc for it.
    if (arg_where_to_load == 0) {
        vPortFree(blob_buffer);
    }

    return blob_result;
}
