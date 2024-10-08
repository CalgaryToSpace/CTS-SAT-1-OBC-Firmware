
#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_adcs.h"
#include "telecommands/telecommand_args_helpers.h"
#include "transforms/arrays.h"
#include "timekeeping/timekeeping.h"
#include "debug_tools/debug_uart.h"
#include "uart_handler/uart_handler.h"
#include "mpi/mpi_command_handling.h"
#include "log/log.h"

// Additional telecommand definitions files:
#include "telecommands/freertos_telecommand_defs.h"
#include "telecommands/flash_telecommand_defs.h"
#include "telecommands/lfs_telecommand_defs.h"
#include "telecommands/log_telecommand_defs.h"
#include "telecommands/timekeeping_telecommand_defs.h"
#include "telecommands/antenna_telecommand_defs.h"
#include "telecommands/i2c_telecommand_defs.h"
#include "telecommands/temperature_sensor_telecommand_defs.h"
#include "telecommands/config_telecommand_defs.h"
#include "telecommands/testing_telecommand_defs.h"
#include "telecommands/telecommand_executor.h"
#include "telecommands/agenda_telecommands_defs.h"
#include "telecommands/mpi_telecommand_defs.h"
#include "telecommands/eps_telecommands.h"
#include "telecommands/stm32_internal_flash_telecommand_defs.h"
#include "telecommands/comms_telecommand_defs.h"


#include "timekeeping/timekeeping.h"
#include "littlefs/littlefs_helper.h"
#include "stm32/stm32_reboot_reason.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

extern volatile uint8_t TASK_heartbeat_is_on;
#define TX_TIMEOUT_DURATION_MS 100	// Timeout duration for transmit in milliseconds
#define RX_TIMEOUT_DURATION_MS 200  // Timeout duration for reception in milliseconds

// extern
const TCMD_TelecommandDefinition_t TCMD_telecommand_definitions[] = {
    {
        .tcmd_name = "hello_world",
        .tcmd_func = TCMDEXEC_hello_world,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "heartbeat_off",
        .tcmd_func = TCMDEXEC_heartbeat_off,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "heartbeat_on",
        .tcmd_func = TCMDEXEC_heartbeat_on,
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
        .tcmd_name = "peripheral_send_receive_data",
        .tcmd_func = TCMDEXEC_peripheral_send_receive_data,
        .number_of_args = 2,
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
        .tcmd_name = "fs_write_file",
        .tcmd_func = TCMDEXEC_fs_write_file,
        .number_of_args = 2,
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
        .number_of_args = 0,
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
        .number_of_args = 0,
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
        .number_of_args = 18,
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
        .tcmd_name = "adcs_raw_gps_status",
        .tcmd_func = TCMDEXEC_adcs_raw_gps_status,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_raw_gps_time",
        .tcmd_func = TCMDEXEC_adcs_raw_gps_time,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_raw_gps_x",
        .tcmd_func = TCMDEXEC_adcs_raw_gps_x,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_raw_gps_y",
        .tcmd_func = TCMDEXEC_adcs_raw_gps_y,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "adcs_raw_gps_z",
        .tcmd_func = TCMDEXEC_adcs_raw_gps_z,
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
        .tcmd_name = "mpi_send_command_hex",
        .tcmd_func = TCMDEXEC_mpi_send_command_hex,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION
    },
    {
        .tcmd_name = "mpi_demo_tx_to_mpi",
        .tcmd_func = TCMDEXEC_mpi_demo_tx_to_mpi,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_GROUND_USAGE_ONLY, // Not useful in space.
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

    // ****************** START SECTION: obc_temperature_sensor_telecommand_defs ******************
    {
        .tcmd_name = "obc_read_temperature",
        .tcmd_func = TCMDEXEC_obc_read_temperature,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    // ****************** END SECTION: obc_temperature_sensor_telecommand_defs ******************
    // ****************** START SECTION: comms_telecommand_defs ******************
    {
        .tcmd_name = "comms_dipole_switch_set_state",
        .tcmd_func = TCMDEXEC_comms_dipole_switch_set_state,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FLIGHT_TESTING,
    },
    // ****************** END SECTION: comms_telecommand_defs ******************
};

// extern
const int16_t TCMD_NUM_TELECOMMANDS = sizeof(TCMD_telecommand_definitions) / sizeof(TCMD_TelecommandDefinition_t);

// each telecommand function must have the following signature:
// uint8_t <function_name>(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
//                          char *response_output_buf, uint16_t response_output_buf_len)

/// @brief A simple telecommand that responds with "Hello, world!"
/// @param args_str No arguments expected
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 if successful, >0 if an error occurred (but hello_world can't return an error)
uint8_t TCMDEXEC_hello_world(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    LOG_message(
        LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Hello, world!"
    );
    return 0;
}

uint8_t TCMDEXEC_heartbeat_off(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    TASK_heartbeat_is_on = 0;
    snprintf(response_output_buf, response_output_buf_len, "Heartbeat OFF");
    return 0;
}

uint8_t TCMDEXEC_heartbeat_on(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    TASK_heartbeat_is_on = 1;
    snprintf(response_output_buf, response_output_buf_len, "Heartbeat ON");
    return 0;
}

uint8_t TCMDEXEC_core_system_stats(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    // TODO: Add temperatures (EPS, OBC, antenna, etc.)
    // TODO: Add beacon sent count

    char timestamp_string_ms[20];
    GEN_uint64_to_str(TIM_get_current_unix_epoch_time_ms(), timestamp_string_ms);

    const char* STM32_reset_cause_name = STM32_reset_cause_enum_to_str(STM32_get_reset_cause());

    char time_of_last_tcmd_sent_ms_string[20];
    GEN_uint64_to_str(TCMD_latest_received_tcmd_timestamp_sent, time_of_last_tcmd_sent_ms_string);
    
    snprintf(
        response_output_buf, response_output_buf_len, 
        "{\"timestamp_ms\":\"%s\",\"uptime_ms\":\"%lu\",\"last_resync_ms\":\"%lu\",\"time_synced_ms_ago\":\"%lu\",\"time_of_last_tcmd_sent_ms\":\"%s\",\"total_tcmd_count\":\"%lu\",\"is_lfs_mounted\":\"%u\",\"last_time_sync_source\":\"%c\",\"reboot_reason\":\"%s\"}\n" ,
        timestamp_string_ms, // timestamp_ms
        TIM_get_current_system_uptime_ms(), // uptime_ms
        TIM_system_uptime_at_last_time_resync_ms, // last_resync_ms
        TIM_get_current_system_uptime_ms() - TIM_system_uptime_at_last_time_resync_ms, // time_synced_ms_ago
        time_of_last_tcmd_sent_ms_string, // time_of_last_tcmd_sent_ms
        TCMD_total_tcmd_queued_count, // total_tcmd_count
        LFS_is_lfs_mounted, // is_lfs_mounted
        TIME_sync_source_enum_to_letter_char(TIM_last_synchronization_source), // last_time_sync_source
        STM32_reset_cause_name // reboot_reason
    ); 

    return 0;
}

uint8_t TCMDEXEC_available_telecommands(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
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

uint8_t TCMDEXEC_reboot(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    DEBUG_uart_print_str("Rebooting by telecommand request...\n");

    // Delay to flush UART buffer
    HAL_Delay(100);

    NVIC_SystemReset();
    return 0;
}

/// @brief Send artibrary configuration commands to a peripheral
/// @param args_str 
/// - Arg 0: UART port to send data to: MPI, LORA, GPS, CAMERA, EPS (case insensitive)
/// - Arg 1: Data to be sent (bytes specified as hex - Max 5KiB buffer)
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0: Success, 1: Error parsing args, 2: Invalid uart port requested, 3: Error transmitting data, 
///         4: Error receiving data, 5: Timeout waiting for response / No response, 6: Unhandled error
/// @note This function doesn't support toggling the EPS to send power to the peripherals
/// @todo Add support to send data to the camera once it's supporting functions are implemented
uint8_t TCMDEXEC_peripheral_send_receive_data(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {

    // Parse UART port argument
    char arg_uart_port_name[10] = "";
    const uint8_t uart_port_name_parse_result = TCMD_extract_string_arg(args_str, 0, arg_uart_port_name, 10);

    // Parse hex-encoded config command (string to bytes)
    const uint16_t tx_buffer_max_size = 1024;               // TODO: Set 5KiBs as the Max size of transmit byte array once more space has been allocated to execution thread
    uint16_t arg_bytes_to_send_len;                         // Variable to store the length of the converted byte array
    uint8_t arg_bytes_to_send[tx_buffer_max_size];          // Byte array to store the values of converted hex string
    const uint8_t bytes_to_send_parse_result = TCMD_extract_hex_array_arg(args_str, 1, arg_bytes_to_send, tx_buffer_max_size, &arg_bytes_to_send_len);

    // Check for parsing errors
    if(uart_port_name_parse_result != 0 || bytes_to_send_parse_result !=0) {
        LOG_message(
            LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Error parsing uart port/data to send arg: Arg 0 Err=%d, Arg 1 Err=%d", 
            uart_port_name_parse_result, 
            bytes_to_send_parse_result
        );
        return 1;    // Error code: Error parsing args
    }
    
    // Allocate space to receive incoming response
    const size_t rx_buffer_max_size = 1024;                 // TODO: Set 5KiBs as the max size for rx buffer once more space has been allocated to execution thread        
    uint16_t rx_buffer_len = 0;                             // Length of rx buffer
    uint8_t rx_buffer[rx_buffer_max_size];                  // Buffer to store incoming response from the selected UART peripheral
    memset(rx_buffer, 0, rx_buffer_max_size);               // Initialize all elements to 0

    UART_HandleTypeDef *UART_handle;                        // Selected UART handle
    volatile uint16_t *UART_rx_buffer_write_idx;            // Pointer to write index for selected UART rx buffer
    const uint16_t *UART_rx_buffer_len;                     // Pointer to the length of the selected UART rx buffer
    volatile uint8_t *UART_rx_buffer;                       // Pointer to selected UART rx buffer
    LOG_system_enum_t LOG_source = LOG_SYSTEM_TELECOMMAND;  // Slected system log source

    // UART 1 selected (MPI)
    if(strcmp(arg_uart_port_name, "MPI") == 0) {

        // UART1 is used by the MPI. We need to store the original MPI mode, then set MPI to command mode
        MPI_current_uart_rx_mode = MPI_RX_MODE_COMMAND_MODE;

        // Set log source
        LOG_source = LOG_SYSTEM_MPI;

        // Transmit bytes and receive response (DMA enabled reception)
        const uint8_t MPI_tx_rx_status = MPI_send_telecommand_get_response(arg_bytes_to_send, arg_bytes_to_send_len, rx_buffer, rx_buffer_max_size, &rx_buffer_len);

        // Log successful transmission and print rx_buffer response in hex (uint8_t)
        char rx_buffer_str[rx_buffer_len*3];

        for(uint16_t i = 0; i < rx_buffer_len; i++) {
            sprintf(&rx_buffer_str[i*3], "%02X ", rx_buffer[i]);
        }

        // Send response log detail
        switch(MPI_tx_rx_status) {
            case 0:
                LOG_message(
                    LOG_source, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
                    "Transmitted %u byte(s) to %s.", 
                    arg_bytes_to_send_len, 
                    arg_uart_port_name
                );
                LOG_message(
                    LOG_source, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
                    "Received %u byte(s) from %s: %s",
                    rx_buffer_len,
                    arg_uart_port_name,
                    rx_buffer_str
                );
                return 0;   // Success
            case 2:
                LOG_message(
                    LOG_source, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "Failed UART transmission."
                );
                return 3;   // Error code: Error transmitting data
            case 3:
                LOG_message(
                    LOG_source, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "Failed UART reception."
                );
                return 4;   // Error code: Error receiving data
            case 4:
                LOG_message(
                    LOG_source, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "No response received. Timeout waiting for 1st byte."
                ); 
                return 5;   // Error code: Error receiving data
            default:
                LOG_message(
                    LOG_source, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "Unknown response return: %u", 
                    MPI_tx_rx_status
                );
                return 6;   // Error code: Unhandled error
        }
    }

    // UART 4 Selected (CAMERA)
    else if(strcasecmp(arg_uart_port_name, "CAMERA") == 0) {
        // TODO: Implement once, DMA handling function for camera is added
        UART_handle = UART_camera_port_handle;
        UART_rx_buffer_write_idx = &UART_camera_buffer_write_idx;
        UART_rx_buffer_len = &UART_camera_buffer_len;
        UART_rx_buffer = &UART_camera_buffer[0];
        UART_camera_is_expecting_data = 1;
        LOG_source = LOG_SYSTEM_TELECOMMAND;                    // TODO: CAMERA is not a system log source, Currently set to TELECOMMAND for the time being
    }

    // UART 2,3 & 5 use interrupt based reception, set UART handle and buffers according to selection
    else {

        // UART 2 Selected (LORA)
        if(strcasecmp(arg_uart_port_name, "LORA") == 0) {
            UART_handle = UART_lora_port_handle;
            UART_rx_buffer_write_idx = &UART_lora_buffer_write_idx;
            UART_rx_buffer_len = &UART_lora_buffer_len;
            UART_rx_buffer = &UART_lora_buffer[0];
            UART_lora_is_expecting_data = 1;
            LOG_source = LOG_SYSTEM_TELECOMMAND;                // TODO: LORA is not a system log source, Currently set to TELECOMMAND for the time being
        }

        // UART 3 Selected (GPS)
        else if(strcasecmp(arg_uart_port_name, "GPS") == 0) {
            UART_handle = UART_gps_port_handle;
            UART_rx_buffer_write_idx = &UART_gps_buffer_write_idx;
            UART_rx_buffer_len = &UART_gps_buffer_len;
            UART_rx_buffer = &UART_gps_buffer[0];
            LOG_source = LOG_SYSTEM_GPS;
        }

        // UART 5 Selected (EPS)
        else if(strcasecmp(arg_uart_port_name, "EPS") == 0) {
            UART_handle = UART_eps_port_handle;
            UART_rx_buffer_write_idx = &UART_eps_buffer_write_idx;
            UART_rx_buffer_len = &UART_eps_buffer_len;
            UART_rx_buffer = &UART_eps_buffer[0];
            UART_eps_is_expecting_data = 1;
            LOG_source = LOG_SYSTEM_EPS;
        }

        // Invalid UART selected
        else {
            LOG_message(
                LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
                "Invalid UART peripheral port requested: %s", 
                arg_uart_port_name
            );
            return 2;   // Error code: Invalid UART port requested
        }

        // Transmit config command to requested peripheral
        HAL_StatusTypeDef transmit_status = HAL_UART_Transmit(UART_handle, arg_bytes_to_send, arg_bytes_to_send_len, TX_TIMEOUT_DURATION_MS);

        // Check UART transmission
        if (transmit_status != HAL_OK) {
            LOG_message(
                LOG_source, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
                "Failed to transmit %u bytes to %s UART port.", 
                arg_bytes_to_send_len, 
                arg_uart_port_name
            );
            return 3; // Error code: Error transmitting data
        }

        // Log successful transmission
        LOG_message(
            LOG_source, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Transmitted %u byte(s) to %s.",
            arg_bytes_to_send_len, 
            arg_uart_port_name
        );
        
        // Reset UART buffer write index & record start time for response reception
        *UART_rx_buffer_write_idx = 0;
        const uint32_t UART_rx_last_write_time_ms = HAL_GetTick();

        // Receive from peripheral until a timeout event
        while(1) {

            // Check for response at least until a timout event
            if ((HAL_GetTick() - UART_rx_last_write_time_ms) < RX_TIMEOUT_DURATION_MS) {
                continue;
            }

            // Timeout without receiving any data
            if (*UART_rx_buffer_write_idx == 0) { 
                LOG_message(
                    LOG_SYSTEM_TELECOMMAND, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                    "No response received. Timeout waiting for 1st byte."
                );
                return 5;   // Error code: Timout waiting for first byte
            }

            // Timeout after receiving some data
            else if ((HAL_GetTick() - UART_rx_last_write_time_ms > RX_TIMEOUT_DURATION_MS) && (*UART_rx_buffer_write_idx > 0)) {
                
                rx_buffer_len = *UART_rx_buffer_write_idx;

                // Copy the UART buffer to the last received byte index & clear the UART buffer
                for (uint16_t i = 0; i < rx_buffer_len; i++) {
                    // Copy only until the max size of rx buffer
                    if(i == rx_buffer_max_size){
                        break;
                    }

                    rx_buffer[i] = UART_rx_buffer[i];
                }
                
                // Reset the UART buffer (memset to 0, but volatile-compatible)
                for (uint16_t i = 0; i < *UART_rx_buffer_len; i++) {
                    UART_rx_buffer[i] = 0;
                }

                // Reset UART buffer write index                
                *UART_rx_buffer_write_idx = 0;

                // Reset listening flags if used by peripheral (using the interrupt reception mode)
                if(UART_handle == UART_lora_port_handle){
                    UART_lora_is_expecting_data = 0;
                }
                else if(UART_handle == UART_eps_port_handle){
                    UART_eps_is_expecting_data = 0;
                }

                // Reception complete break out of loop
                break; 
            }
        }
    }

    // Send back complete response if received
    if(rx_buffer_len > 0) { 

        // Convert rx_buffer to a string for logging
        char rx_buffer_str[rx_buffer_len*3];                        

        for(uint16_t i = 0; i < rx_buffer_len; i++) {
            sprintf(&rx_buffer_str[i*3], "%02X ", rx_buffer[i]);
        }

        // Log received response in Hex formatting
        LOG_message(
            LOG_source, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "Received %u byte(s) from %s: %s",
            rx_buffer_len,
            arg_uart_port_name,
            rx_buffer_str
        );        
    }

    return 0;   // Success
}

