
#include "telecommand_exec/telecommand_definitions.h"
#include "telecommands/telecommand_adcs.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "transforms/arrays.h"
#include "timekeeping/timekeeping.h"
#include "debug_tools/debug_uart.h"
#include "uart_handler/uart_handler.h"
#include "mpi/mpi_command_handling.h"
#include "log/log.h"

// Additional telecommand definitions files:
#include "telecommands/system_telecommand_defs.h"
#include "telecommands/freertos_telecommand_defs.h"
#include "telecommands/flash_telecommand_defs.h"
#include "telecommands/lfs_telecommand_defs.h"
#include "telecommands/log_telecommand_defs.h"
#include "telecommands/timekeeping_telecommand_defs.h"
#include "telecommands/antenna_telecommand_defs.h"
#include "telecommands/i2c_telecommand_defs.h"
#include "telecommands/obc_systems_telecommand_defs.h"
#include "telecommands/uart_telecommand_defs.h"
#include "telecommands/uart_error_tracking_telecommands.h"
#include "telecommands/config_telecommand_defs.h"
#include "telecommands/testing_telecommand_defs.h"
#include "telecommand_exec/telecommand_executor.h"
#include "telecommands/agenda_telecommands_defs.h"
#include "telecommands/mpi_telecommand_defs.h"
#include "telecommands/boom_deploy_telecommand_defs.h"
#include "telecommands/eps_telecommands.h"
#include "telecommands/stm32_internal_flash_telecommand_defs.h"
#include "telecommands/comms_telecommand_defs.h"
#include "telecommands/gnss_telecommand_defs.h"
#include "telecommands/camera_telecommand_defs.h"

#include "timekeeping/timekeeping.h"
#include "littlefs/littlefs_helper.h"
#include "stm32/stm32_reboot_reason.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

// extern
const TCMD_TelecommandDefinition_t TCMD_telecommand_definitions[] = {
    {
        .tcmd_name = "hello_world",
        .tcmd_func = TCMDEXEC_hello_world,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "core_system_stats",
        .tcmd_func = TCMDEXEC_core_system_stats,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "get_system_time",
        .tcmd_func = TCMDEXEC_get_system_time,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "set_system_time",
        .tcmd_func = TCMDEXEC_set_system_time,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "correct_system_time",
        .tcmd_func = TCMDEXEC_correct_system_time,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "set_eps_time_based_on_obc_time",
        .tcmd_func = TCMDEXEC_set_eps_time_based_on_obc_time,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "set_obc_time_based_on_eps_time",
        .tcmd_func = TCMDEXEC_set_obc_time_based_on_eps_time,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "available_telecommands",
        .tcmd_func = TCMDEXEC_available_telecommands,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "reboot",
        .tcmd_func = TCMDEXEC_reboot,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "scan_i2c_bus",
        .tcmd_func = TCMDEXEC_scan_i2c_bus,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "scan_i2c_bus_verbose",
        .tcmd_func = TCMDEXEC_scan_i2c_bus_verbose,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "system_self_check_failures_as_json",
        .tcmd_func = TCMDEXEC_system_self_check_failures_as_json,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "system_self_check_as_json",
        .tcmd_func = TCMDEXEC_system_self_check_as_json,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "obc_get_rbf_state",
        .tcmd_func = TCMDEXEC_obc_get_rbf_state,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    
    // ****************** SECTION: uart_telecommand_defs ******************
    {
        .tcmd_name = "uart_send_hex_get_response_hex",
        .tcmd_func = TCMDEXEC_uart_send_hex_get_response_hex,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "uart_get_last_rx_times_json",
        .tcmd_func = TCMDEXEC_uart_get_last_rx_times_json,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "uart_get_errors_json",
        .tcmd_func = TCMDEXEC_uart_get_errors_json,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },

    // ****************** SECTION: testing_telecommand_defs ******************

    {
        .tcmd_name = "echo_back_args",
        .tcmd_func = TCMDEXEC_echo_back_args,
        .number_of_args = 1, // TODO: support more than 1 arg
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "echo_back_uint32_args",
        .tcmd_func = TCMDEXEC_echo_back_uint32_args,
        .number_of_args = 3,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "run_all_unit_tests",
        .tcmd_func = TCMDEXEC_run_all_unit_tests,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "demo_blocking_delay",
        .tcmd_func = TCMDEXEC_demo_blocking_delay,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FLIGHT_TESTING, // Can cause crash via Watchdog reset.
    },

    // ****************** END SECTION: testing_telecommand_defs ******************

    // ****************** SECTION: config_telecommand_defs ******************
    {
        .tcmd_name = "config_set_int_var",
        .tcmd_func = TCMDEXEC_config_set_int_var,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "config_set_str_var",
        .tcmd_func = TCMDEXEC_config_set_str_var,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "config_get_int_var_json",
        .tcmd_func = TCMDEXEC_config_get_int_var_json,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "config_get_str_var_json",
        .tcmd_func = TCMDEXEC_config_get_str_var_json,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "config_get_all_vars_jsonl",
        .tcmd_func = TCMDEXEC_config_get_all_vars_jsonl,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    // ****************** END SECTION: config_telecommand_defs ******************

    // ****************** SECTION: flash_telecommand_defs ******************
    {
        .tcmd_name = "flash_activate_each_cs",
        .tcmd_func = TCMDEXEC_flash_activate_each_cs,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "flash_each_is_reachable",
        .tcmd_func = TCMDEXEC_flash_each_is_reachable,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "flash_read_hex",
        .tcmd_func = TCMDEXEC_flash_read_hex,
        .number_of_args = 3,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "flash_write_hex",
        .tcmd_func = TCMDEXEC_flash_write_hex,
        .number_of_args = 3,
        .readiness_level = TCMD_READINESS_LEVEL_FLIGHT_TESTING,
    },
    {
        .tcmd_name = "flash_erase",
        .tcmd_func = TCMDEXEC_flash_erase,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FLIGHT_TESTING,
    },
    {
        .tcmd_name = "flash_benchmark_erase_write_read",
        .tcmd_func = TCMDEXEC_flash_benchmark_erase_write_read,
        .number_of_args = 3,
        .readiness_level = TCMD_READINESS_LEVEL_FLIGHT_TESTING,
    },
    {
        .tcmd_name = "flash_reset",
        .tcmd_func = TCMDEXEC_flash_reset,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_IN_PROGRESS,
    },
    {
        .tcmd_name = "flash_read_status_register",
        .tcmd_func = TCMDEXEC_flash_read_status_register,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "flash_write_enable",
        .tcmd_func = TCMDEXEC_flash_write_enable,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    // ****************** END SECTION: flash_telecommand_defs ******************

    // ****************** SECTION: lfs_telecommand_defs ******************
    {
        .tcmd_name = "fs_format_storage",
        .tcmd_func = TCMDEXEC_fs_format_storage,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "fs_mount",
        .tcmd_func = TCMDEXEC_fs_mount,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "fs_unmount",
        .tcmd_func = TCMDEXEC_fs_unmount,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "fs_list_directory",
        .tcmd_func = TCMDEXEC_fs_list_directory,
        .number_of_args = 3,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "fs_list_directory_json",
        .tcmd_func = TCMDEXEC_fs_list_directory_json,
        .number_of_args = 3,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "fs_make_directory",
        .tcmd_func = TCMDEXEC_fs_make_directory,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "fs_write_file_str",
        .tcmd_func = TCMDEXEC_fs_write_file_str,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "fs_write_file_hex",
        .tcmd_func = TCMDEXEC_fs_write_file_hex,
        .number_of_args = 3,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "fs_delete_file",
        .tcmd_func = TCMDEXEC_fs_delete_file,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "fs_read_file_hex",
        .tcmd_func = TCMDEXEC_fs_read_file_hex,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "fs_read_text_file",
        .tcmd_func = TCMDEXEC_fs_read_text_file,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "fs_demo_write_then_read",
        .tcmd_func = TCMDEXEC_fs_demo_write_then_read,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "fs_benchmark_write_read",
        .tcmd_func = TCMDEXEC_fs_benchmark_write_read,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    // ****************** END SECTION: lfs_telecommand_defs ******************
    
    // ****************** SECTION: telecommand_adcs ******************
    {
        .tcmd_name = "adcs_ack",
        .tcmd_func = TCMDEXEC_adcs_ack,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_set_wheel_speed",
        .tcmd_func = TCMDEXEC_adcs_set_wheel_speed,
        .number_of_args = 3,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_reset",
        .tcmd_func = TCMDEXEC_adcs_reset,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_identification",
        .tcmd_func = TCMDEXEC_adcs_identification,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_program_status",
        .tcmd_func = TCMDEXEC_adcs_program_status,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_communication_status",
        .tcmd_func = TCMDEXEC_adcs_communication_status,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_deploy_magnetometer",
        .tcmd_func = TCMDEXEC_adcs_deploy_magnetometer,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_set_run_mode",
        .tcmd_func = TCMDEXEC_adcs_set_run_mode,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_clear_errors",
        .tcmd_func = TCMDEXEC_adcs_clear_errors,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_attitude_control_mode",
        .tcmd_func = TCMDEXEC_adcs_attitude_control_mode,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_attitude_estimation_mode",
        .tcmd_func = TCMDEXEC_adcs_attitude_estimation_mode,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_run_once",
        .tcmd_func = TCMDEXEC_adcs_run_once,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_set_magnetometer_mode",
        .tcmd_func = TCMDEXEC_adcs_set_magnetometer_mode,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_set_magnetorquer_output",
        .tcmd_func = TCMDEXEC_adcs_set_magnetorquer_output,
        .number_of_args = 3,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_save_config",
        .tcmd_func = TCMDEXEC_adcs_save_config,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_estimate_angular_rates",
        .tcmd_func = TCMDEXEC_adcs_estimate_angular_rates,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_get_llh_position",
        .tcmd_func = TCMDEXEC_adcs_get_llh_position,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_get_power_control",
        .tcmd_func = TCMDEXEC_adcs_get_power_control,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_set_power_control",
        .tcmd_func = TCMDEXEC_adcs_set_power_control,
        .number_of_args = 10,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    }, 
    {
        .tcmd_name = "adcs_enter_low_power_mode",
        .tcmd_func = TCMDEXEC_adcs_enter_low_power_mode,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_track_sun",
        .tcmd_func = TCMDEXEC_adcs_track_sun,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    }, 
    {
        .tcmd_name = "adcs_set_magnetometer_config",
        .tcmd_func = TCMDEXEC_adcs_set_magnetometer_config,
        .number_of_args = 15,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_bootloader_clear_errors",
        .tcmd_func = TCMDEXEC_adcs_bootloader_clear_errors,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_set_unix_time_save_mode",
        .tcmd_func = TCMDEXEC_adcs_set_unix_time_save_mode,
        .number_of_args = 4,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_get_unix_time_save_mode",
        .tcmd_func = TCMDEXEC_adcs_get_unix_time_save_mode,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_set_sgp4_orbit_params",
        .tcmd_func = TCMDEXEC_adcs_set_sgp4_orbit_params,
        .number_of_args = 8,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_get_sgp4_orbit_params",
        .tcmd_func = TCMDEXEC_adcs_get_sgp4_orbit_params,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_save_orbit_params",
        .tcmd_func = TCMDEXEC_adcs_save_orbit_params,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_rate_sensor_rates",
        .tcmd_func = TCMDEXEC_adcs_rate_sensor_rates,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_get_wheel_speed",
        .tcmd_func = TCMDEXEC_adcs_get_wheel_speed,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_get_magnetorquer_command",
        .tcmd_func = TCMDEXEC_adcs_get_magnetorquer_command,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_get_raw_magnetometer_values",
        .tcmd_func = TCMDEXEC_adcs_get_raw_magnetometer_values,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_estimate_fine_angular_rates",
        .tcmd_func = TCMDEXEC_adcs_estimate_fine_angular_rates,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_get_magnetometer_config",
        .tcmd_func = TCMDEXEC_adcs_get_magnetometer_config,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_get_commanded_attitude_angles",
        .tcmd_func = TCMDEXEC_adcs_get_commanded_attitude_angles,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_set_commanded_attitude_angles",
        .tcmd_func = TCMDEXEC_adcs_set_commanded_attitude_angles,
        .number_of_args = 3,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_set_estimation_params",
        .tcmd_func = TCMDEXEC_adcs_set_estimation_params,
        .number_of_args = 17,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_get_estimation_params",
        .tcmd_func = TCMDEXEC_adcs_get_estimation_params,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_set_augmented_sgp4_params",
        .tcmd_func = TCMDEXEC_adcs_set_augmented_sgp4_params,
        .number_of_args = 17,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_get_augmented_sgp4_params",
        .tcmd_func = TCMDEXEC_adcs_get_augmented_sgp4_params,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_set_tracking_controller_target_reference",
        .tcmd_func = TCMDEXEC_adcs_set_tracking_controller_target_reference,
        .number_of_args = 3,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_get_tracking_controller_target_reference",
        .tcmd_func = TCMDEXEC_adcs_get_tracking_controller_target_reference,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_set_rate_gyro_config",
        .tcmd_func = TCMDEXEC_adcs_set_rate_gyro_config,
        .number_of_args = 7,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_get_rate_gyro_config",
        .tcmd_func = TCMDEXEC_adcs_get_rate_gyro_config,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_estimated_attitude_angles",
        .tcmd_func = TCMDEXEC_adcs_estimated_attitude_angles,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_magnetic_field_vector",
        .tcmd_func = TCMDEXEC_adcs_magnetic_field_vector,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_fine_sun_vector",
        .tcmd_func = TCMDEXEC_adcs_fine_sun_vector,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_nadir_vector",
        .tcmd_func = TCMDEXEC_adcs_nadir_vector,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_commanded_wheel_speed",
        .tcmd_func = TCMDEXEC_adcs_commanded_wheel_speed,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_igrf_magnetic_field_vector",
        .tcmd_func = TCMDEXEC_adcs_igrf_magnetic_field_vector,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_quaternion_error_vector",
        .tcmd_func = TCMDEXEC_adcs_quaternion_error_vector,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_estimated_gyro_bias",
        .tcmd_func = TCMDEXEC_adcs_estimated_gyro_bias,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_estimation_innovation_vector",
        .tcmd_func = TCMDEXEC_adcs_estimation_innovation_vector,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_raw_cam1_sensor",
        .tcmd_func = TCMDEXEC_adcs_raw_cam1_sensor,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_raw_cam2_sensor",
        .tcmd_func = TCMDEXEC_adcs_raw_cam2_sensor,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_raw_coarse_sun_sensor_1_to_6",
        .tcmd_func = TCMDEXEC_adcs_raw_coarse_sun_sensor_1_to_6,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_raw_coarse_sun_sensor_7_to_10",
        .tcmd_func = TCMDEXEC_adcs_raw_coarse_sun_sensor_7_to_10,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_cubecontrol_current",
        .tcmd_func = TCMDEXEC_adcs_cubecontrol_current,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_measurements",
        .tcmd_func = TCMDEXEC_adcs_measurements,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_download_sd_file",
        .tcmd_func = TCMDEXEC_adcs_download_sd_file,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_IN_PROGRESS,
    }, 
    {
        .tcmd_name = "adcs_generic_command",
        .tcmd_func = TCMDEXEC_adcs_generic_command,
        .number_of_args = 2, 
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_generic_telemetry_request",
        .tcmd_func = TCMDEXEC_adcs_generic_telemetry_request,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_acp_execution_state",
        .tcmd_func = TCMDEXEC_adcs_acp_execution_state,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_get_current_state_1",
        .tcmd_func = TCMDEXEC_adcs_get_current_state_1,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_save_image_to_sd",
        .tcmd_func = TCMDEXEC_adcs_save_image_to_sd,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_request_commissioning_telemetry",
        .tcmd_func = TCMDEXEC_adcs_request_commissioning_telemetry,
        .number_of_args = 3,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_synchronize_unix_time",
        .tcmd_func = TCMDEXEC_adcs_synchronize_unix_time,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    }, 
    {
        .tcmd_name = "adcs_get_current_unix_time",
        .tcmd_func = TCMDEXEC_adcs_get_current_unix_time,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    }, 
    {
        .tcmd_name = "adcs_set_sd_log_config",
        .tcmd_func = TCMDEXEC_adcs_set_sd_log_config,
        .number_of_args = 4,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    }, 
    {
        .tcmd_name = "adcs_get_sd_log_config",
        .tcmd_func = TCMDEXEC_adcs_get_sd_log_config,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    }, 
    {
        .tcmd_name = "adcs_format_sd",
        .tcmd_func = TCMDEXEC_adcs_format_sd,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    }, 
    {
        .tcmd_name = "adcs_download_index_file",
        .tcmd_func = TCMDEXEC_adcs_download_index_file,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    }, 
    {
        .tcmd_name = "adcs_set_commissioning_modes",
        .tcmd_func = TCMDEXEC_adcs_set_commissioning_modes,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_erase_sd_file",
        .tcmd_func = TCMDEXEC_adcs_erase_sd_file,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    }, 
    {
        .tcmd_name = "adcs_exit_bootloader",
        .tcmd_func = TCMDEXEC_adcs_exit_bootloader,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },

    // ****************** END SECTION: telecommand_adcs ******************

    // ****************** SECTION: log_telecommand_defs ******************
    {
        .tcmd_name = "log_set_sink_enabled_state",
        .tcmd_func = TCMDEXEC_log_set_sink_enabled_state,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "log_set_system_file_logging_enabled_state",
        .tcmd_func = TCMDEXEC_log_set_system_file_logging_enabled_state,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "log_report_sink_enabled_state",
        .tcmd_func = TCMDEXEC_log_report_sink_enabled_state,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "log_report_all_sink_enabled_states",
        .tcmd_func = TCMDEXEC_log_report_all_sink_enabled_states,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "log_report_system_file_logging_state",
        .tcmd_func = TCMDEXEC_log_report_system_file_logging_state,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "log_report_all_system_file_logging_states",
        .tcmd_func = TCMDEXEC_log_report_all_system_file_logging_states,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "log_set_sink_debugging_messages_state",
        .tcmd_func = TCMDEXEC_log_set_sink_debugging_messages_state,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "log_set_system_debugging_messages_state",
        .tcmd_func = TCMDEXEC_log_set_system_debugging_messages_state,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "log_report_latest_message_from_memory",
        .tcmd_func = TCMDEXEC_log_report_latest_message_from_memory,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "log_report_n_latest_messages_from_memory",
        .tcmd_func = TCMDEXEC_log_report_n_latest_messages_from_memory,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },

    // ****************** END SECTION: log_telecommand_defs ******************

    // ****************** SECTION: freertos_telecommand_defs ******************

    {
        .tcmd_name = "freetos_list_tasks_jsonl",
        .tcmd_func = TCMDEXEC_freetos_list_tasks_jsonl,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },

    {
        .tcmd_name = "freertos_demo_stack_usage",
        .tcmd_func = TCMDEXEC_freertos_demo_stack_usage,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FLIGHT_TESTING, // Can cause crash via stack overflow.
    },

    // ****************** END SECTION: freertos_telecommand_defs ******************


    /* ============================ EPS-Related (eps_telecommands.c/h) ================= */
    {
        .tcmd_name = "eps_watchdog",
        .tcmd_func = TCMDEXEC_eps_watchdog,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "eps_system_reset",
        .tcmd_func = TCMDEXEC_eps_system_reset,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "eps_no_operation",
        .tcmd_func = TCMDEXEC_eps_no_operation,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "eps_cancel_operation",
        .tcmd_func = TCMDEXEC_eps_cancel_operation,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "eps_switch_to_mode",
        .tcmd_func = TCMDEXEC_eps_switch_to_mode,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "eps_set_channel_enabled",
        .tcmd_func = TCMDEXEC_eps_set_channel_enabled,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "eps_get_system_status_json",
        .tcmd_func = TCMDEXEC_eps_get_system_status_json,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "eps_get_pdu_overcurrent_fault_state_json",
        .tcmd_func = TCMDEXEC_eps_get_pdu_overcurrent_fault_state_json,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "eps_get_pbu_abf_placed_state_json",
        .tcmd_func = TCMDEXEC_eps_get_pbu_abf_placed_state_json,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "eps_get_pdu_data_for_channel_json",
        .tcmd_func = TCMDEXEC_eps_get_pdu_data_for_channel_json,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "eps_get_pdu_housekeeping_data_eng_json",
        .tcmd_func = TCMDEXEC_eps_get_pdu_housekeeping_data_eng_json,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "eps_get_pdu_housekeeping_data_run_avg_json",
        .tcmd_func = TCMDEXEC_eps_get_pdu_housekeeping_data_run_avg_json,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "eps_get_pbu_housekeeping_data_eng_json",
        .tcmd_func = TCMDEXEC_eps_get_pbu_housekeeping_data_eng_json,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "eps_get_pbu_housekeeping_data_run_avg_json",
        .tcmd_func = TCMDEXEC_eps_get_pbu_housekeeping_data_run_avg_json,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "eps_get_pcu_housekeeping_data_eng_json",
        .tcmd_func = TCMDEXEC_eps_get_pcu_housekeeping_data_eng_json,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "eps_get_pcu_housekeeping_data_run_avg_json",
        .tcmd_func = TCMDEXEC_eps_get_pcu_housekeeping_data_run_avg_json,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "eps_get_piu_housekeeping_data_eng_json",
        .tcmd_func = TCMDEXEC_eps_get_piu_housekeeping_data_eng_json,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "eps_get_piu_housekeeping_data_run_avg_json",
        .tcmd_func = TCMDEXEC_eps_get_piu_housekeeping_data_run_avg_json,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    }, 
    {
        .tcmd_name = "eps_get_enabled_channels_json",  
        .tcmd_func = TCMDEXEC_eps_get_enabled_channels_json,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },

    {
        .tcmd_name = "eps_get_current_battery_percent",
        .tcmd_func = TCMDEXEC_eps_get_current_battery_percent,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION
    },
    {
        .tcmd_name = "eps_power_management_set_current_threshold",
        .tcmd_func = TCMDEXEC_eps_power_management_set_current_threshold,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    /* *************************** END EPS Section ************************************** */
    
    
    // ****************** SECTION: agenda_telecommand_defs ******************

    {
        .tcmd_name = "agenda_delete_all",
        .tcmd_func = TCMDEXEC_agenda_delete_all,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },

    {
        .tcmd_name = "agenda_delete_by_tssent",
        .tcmd_func = TCMDEXEC_agenda_delete_by_tssent,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },

    {
        .tcmd_name = "agenda_fetch_jsonl",
        .tcmd_func = TCMDEXEC_agenda_fetch_jsonl,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },

    {
        .tcmd_name = "agenda_delete_by_name",
        .tcmd_func = TCMDEXEC_agenda_delete_by_name,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },

    // ****************** END SECTION: agenda_telecommand_defs ******************

    // ****************** START: MPI_telecommand_definitions ******************
    {
        .tcmd_name = "mpi_send_command_get_response_hex",
        .tcmd_func = TCMDEXEC_mpi_send_command_get_response_hex,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION
    },
    {
        .tcmd_name = "mpi_demo_tx_to_mpi",
        .tcmd_func = TCMDEXEC_mpi_demo_tx_to_mpi,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_GROUND_USAGE_ONLY, // Not useful in space.
    },
    {
        .tcmd_name = "mpi_demo_set_transceiver_mode",
        .tcmd_func = TCMDEXEC_mpi_demo_set_transceiver_mode,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_GROUND_USAGE_ONLY, // Not useful in space.
    },
    {
        .tcmd_name = "mpi_enable_active_mode",
        .tcmd_func = TCMDEXEC_mpi_enable_active_mode,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "mpi_disable_active_mode",
        .tcmd_func = TCMDEXEC_mpi_disable_active_mode,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    // ****************** END: MPI_telecommand_definitions ********************
    // ****************** START SECTION: stm32_internal_flash_telecommand_defs ******************

    {
        .tcmd_name = "stm32_internal_flash_read",
        .tcmd_func = TCMDEXEC_stm32_internal_flash_read,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_GROUND_USAGE_ONLY,
    },

    {
        .tcmd_name = "stm32_internal_flash_write",
        .tcmd_func = TCMDEXEC_stm32_internal_flash_write,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_GROUND_USAGE_ONLY,
    },

    {
        .tcmd_name = "stm32_internal_flash_erase",
        .tcmd_func = TCMDEXEC_stm32_internal_flash_erase,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_GROUND_USAGE_ONLY,
    },

    {
        .tcmd_name = "stm32_internal_flash_get_option_bytes",
        .tcmd_func = TCMDEXEC_stm32_internal_flash_get_option_bytes,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },

    {
        .tcmd_name = "stm32_internal_flash_get_active_flash_bank",
        .tcmd_func = TCMDEXEC_stm32_internal_flash_get_active_flash_bank,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    
    {
        .tcmd_name = "stm32_internal_flash_set_active_flash_bank",
        .tcmd_func = TCMDEXEC_stm32_internal_flash_set_active_flash_bank,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    

    // ****************** END SECTION: stm32_internal_flash_telecommand_defs ******************

    // ****************** SECTION: antenna_telecommand_defs ******************
    {
        .tcmd_name = "ant_reset",
        .tcmd_func = TCMDEXEC_ant_reset,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "ant_arm_antenna_system",
        .tcmd_func = TCMDEXEC_ant_arm_antenna_system,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "ant_disarm_antenna_system",
        .tcmd_func = TCMDEXEC_ant_disarm_antenna_system,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "ant_deploy_antenna",
        .tcmd_func = TCMDEXEC_ant_deploy_antenna,
        .number_of_args = 3,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "ant_start_automated_antenna_deployment",
        .tcmd_func = TCMDEXEC_ant_start_automated_antenna_deployment,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "ant_deploy_antenna_with_override",
        .tcmd_func = TCMDEXEC_ant_deploy_antenna_with_override,
        .number_of_args = 3,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "ant_cancel_deployment_system_activation",
        .tcmd_func = TCMDEXEC_ant_cancel_deployment_system_activation,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "ant_measure_temp",
        .tcmd_func = TCMDEXEC_ant_measure_temp,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "ant_report_deployment_status",
        .tcmd_func = TCMDEXEC_ant_report_deployment_status,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "ant_report_antenna_deployment_activation_count",
        .tcmd_func = TCMDEXEC_ant_report_antenna_deployment_activation_count,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "ant_report_antenna_deployment_activation_time",
        .tcmd_func = TCMDEXEC_ant_report_antenna_deployment_activation_time,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    // ****************** END SECTION: antenna_telecommand_defs ******************

    // ****************** START SECTION: obc_systems_telecommand_defs ******************
    {
        .tcmd_name = "obc_read_temperature_complex",
        .tcmd_func = TCMDEXEC_obc_read_temperature_complex,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "obc_read_temperature",
        .tcmd_func = TCMDEXEC_obc_read_temperature,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },

    {
        .tcmd_name = "obc_adc_read_vbat_voltage",
        .tcmd_func = TCMDEXEC_obc_adc_read_vbat_voltage,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    // ****************** END SECTION: obc_systems_telecommand_defs ******************
    // ****************** START SECTION: comms_telecommand_defs ******************
    {
        .tcmd_name = "comms_set_rf_switch_control_mode",
        .tcmd_func = TCMDEXEC_comms_set_rf_switch_control_mode,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FLIGHT_TESTING,
    },
    {
        .tcmd_name = "comms_get_rf_switch_info",
        .tcmd_func = TCMDEXEC_comms_get_rf_switch_info,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "comms_bulk_file_downlink_start",
        .tcmd_func = TCMDEXEC_comms_bulk_file_downlink_start,
        .number_of_args = 3,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "comms_bulk_file_downlink_pause",
        .tcmd_func = TCMDEXEC_comms_bulk_file_downlink_pause,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "comms_bulk_file_downlink_resume",
        .tcmd_func = TCMDEXEC_comms_bulk_file_downlink_resume,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    // ****************** END SECTION: comms_telecommand_defs ******************
    // ****************** SECTION: gnss_telecommand_defs ******************
    {
        .tcmd_name = "gnss_send_cmd_ascii",
        .tcmd_func = TCMDEXEC_gnss_send_cmd_ascii,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    // ****************** END SECTION: gnss_telecommand_defs ******************
    // ****************** SECTION: camera_telecommand_defs *******************
    {
        .tcmd_name = "camera_setup",
        .tcmd_func = TCMDEXEC_camera_setup,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "camera_test",
        .tcmd_func = TCMDEXEC_camera_test,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "camera_change_baud_rate",
        .tcmd_func = TCMDEXEC_camera_change_baud_rate,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "camera_capture",
        .tcmd_func = TCMDEXEC_camera_capture,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION
    },
    // ****************** END SECTION: camera_telecommand_defs *******************
    // ****************** SECTION: boom_deploy_telecommand_defs ******************
    {
        .tcmd_name = "boom_deploy_timed",
        .tcmd_func = TCMDEXEC_boom_deploy_timed,
        .number_of_args = 2,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "boom_self_check",
        .tcmd_func = TCMDEXEC_boom_self_check,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    // ****************** END SECTION: boom_deploy_telecommand_defs ******************
};

// extern
const int16_t TCMD_NUM_TELECOMMANDS = sizeof(TCMD_telecommand_definitions) / sizeof(TCMD_TelecommandDefinition_t);

// Each telecommand function must have the following signature:
// uint8_t <function_name>(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
//                          char *response_output_buf, uint16_t response_output_buf_len)
