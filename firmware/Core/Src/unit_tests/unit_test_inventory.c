#include "unit_tests/unit_test_helpers.h"
#include "unit_tests/unit_test_inventory.h"
#include "unit_tests/test_ant_commands.h"
#include "unit_tests/test_assorted_prototypes.h"
#include "unit_tests/test_byte_transforms.h"
#include "unit_tests/test_logging.h"
#include "unit_tests/test_telecommand_parser.h"
#include "unit_tests/test_tests.h"
#include "unit_tests/test_adcs.h"
#include "unit_tests/test_telecommand_arg_helpers.h"
#include "unit_tests/unit_test_helpers.h"
#include "unit_tests/test_configuration_variables.h"
#include "unit_tests/test_obc_temperature_sensor.h"
#include "unit_tests/unit_test_gnss.h"
#include "unit_tests/test_uart_error_tracking.h"
#include "unit_tests/test_eps_drivers.h"
#include "unit_tests/test_eps_struct_packers.h"
#include "unit_tests/test_eps_calculations.h"
#include "unit_tests/test_sha256.h"
#include "unit_tests/test_gnss_time.h"

// extern
const TEST_Definition_t TEST_definitions[] = {

    {
        .test_func = TEST_EXEC__test_asserts,
        .test_file = "unit_tests/test_tests",
        .test_func_name = "test_asserts"
    },
    
    {
        .test_func = TEST_EXEC__this_test_always_fails,
        .test_file = "unit_tests/test_tests",
        .test_func_name = "this_test_always_fails"
    },

    {
        .test_func = TEST_EXEC__TCMD_is_char_alphanumeric,
        .test_file = "telecommands/telecommand_parser",
        .test_func_name = "TCMD_is_char_alphanumeric"
    },
    
    {
        .test_func = TEST_EXEC__GEN_get_index_of_substring_in_array,
        .test_file = "telecommands/telecommand_parser",
        .test_func_name = "GEN_get_index_of_substring_in_array"
    },
    
    {
        .test_func = TEST_EXEC__TCMD_check_starts_with_device_id,
        .test_file = "telecommands/telecommand_parser",
        .test_func_name = "TCMD_check_starts_with_device_id"
    },
    {
        .test_func = TEST_EXEC__TCMD_process_suffix_tag_sha256,
        .test_file = "telecommands/telecommand_parser",
        .test_func_name = "TCMD_process_suffix_tag_sha256"
    },
    {
        .test_func = TEST_EXEC__TCMD_process_suffix_tag_tsexec,
        .test_file = "telecommands/telecommand_parser",
        .test_func_name = "TCMD_process_suffix_tag_tsexec"
    },
    {
        .test_func = TEST_EXEC__TCMD_process_suffix_tag_tssent,
        .test_file = "telecommands/telecommand_parser",
        .test_func_name = "TCMD_process_suffix_tag_tssent"
    },
    {
        .test_func = TEST_EXEC__TCMD_process_suffix_tag_resp_fname,
        .test_file = "telecommands/telecommand_parser",
        .test_func_name = "TCMD_process_suffix_tag_resp_fname"
    },
    {
        .test_func = TEST_EXEC__GEN_convert_big_endian_four_byte_array_to_uint32,
        .test_file = "transforms/byte_transforms",
        .test_func_name = "GEN_convert_big_endian_four_byte_array_to_uint32"
    },

    {
        .test_func = TEST_EXEC__GEN_int64_to_str,
        .test_file = "transforms/arrays",
        .test_func_name = "GEN_int64_to_str"
    },
    // ****************** SECTION: test_gnss_time ******************
    {
        .test_func = TEST_EXEC__GNSS_format_and_convert_to_unix_epoch,
        .test_file = "gnss_receiver/gnss_time",
        .test_func_name = "GNSS_format_and_convert_to_unix_epoch"
    },
    // ****************** END SECTION: test_gnss_time ******************
    // ****************** SECTION: test_adcs ******************
    
    {
		.test_func = TEST_EXEC__ADCS_pack_to_ack_struct,
		.test_file = "unit_tests/test_adcs",
		.test_func_name = "ADCS_pack_to_ack_struct"
	},

    {
		.test_func = TEST_EXEC__ADCS_pack_to_identification_struct,
		.test_file = "unit_tests/test_adcs",
		.test_func_name = "ADCS_pack_to_identification_struct"
	},

    {
		.test_func = TEST_EXEC__ADCS_pack_to_program_status_struct,
		.test_file = "unit_tests/test_adcs",
		.test_func_name = "ADCS_pack_to_program_status_struct"
	},

    {
		.test_func = TEST_EXEC__ADCS_pack_to_comms_status_struct,
		.test_file = "unit_tests/test_adcs",
		.test_func_name = "ADCS_pack_to_comms_status_struct"
	},

    {
        .test_func = TEST_EXEC__ADCS_pack_to_power_control_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_power_control_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_angular_rates_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_angular_rates_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_llh_position_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_llh_position_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_unix_time_save_mode_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_unix_time_save_mode_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_orbit_params_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_orbit_params_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_rated_sensor_rates_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_rated_sensor_rates_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_wheel_speed_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_wheel_speed_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_magnetorquer_command_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_magnetorquer_command_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_raw_magnetometer_values_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_raw_magnetometer_values_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_fine_angular_rates_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_fine_angular_rates_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_magnetometer_config_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_magnetometer_config_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_commanded_attitude_angles_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_commanded_attitude_angles_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_estimation_params_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_estimation_params_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_augmented_sgp4_params_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_augmented_sgp4_params_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_tracking_controller_target_reference_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_tracking_controller_target_reference_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_rate_gyro_config_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_rate_gyro_config_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_estimated_attitude_angles_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_estimated_attitude_angles_struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_magnetic_field_vector_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_magnetic_field_vector_struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_fine_sun_vector_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_fine_sun_vector_struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_nadir_vector_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_nadir_vector_struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_commanded_wheel_speed_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_commanded_wheel_speed_struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_igrf_magnetic_field_vector_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_igrf_magnetic_field_vector_struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_quaternion_error_vector_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_quaternion_error_vector_struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_estimated_gyro_bias_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_estimated_gyro_bias_struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_estimation_innovation_vector_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_estimation_innovation_vector_struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_raw_cam1_sensor_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_raw_cam1_sensor_struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_raw_cam2_sensor_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_raw_cam2_sensor_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_raw_coarse_sun_sensor_1_to_6_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_raw_coarse_sun_sensor_1_to_6_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_raw_coarse_sun_sensor_7_to_10_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_raw_coarse_sun_sensor_7_to_10_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_cubecontrol_current_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_cubecontrol_current_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_raw_gps_status_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_raw_gps_status_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_raw_gps_time_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_raw_gps_time_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_raw_gps_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_raw_gps_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_measurements_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_measurements_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_acp_execution_state_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_acp_execution_state_struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_current_state_1_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_current_state_1_struct"
    }, 

    {
        .test_func = TEST_EXEC__ADCS_pack_to_raw_star_tracker_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_raw_star_tracker_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_unix_time_ms,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_unix_time_ms"
    }, 

    {
        .test_func = TEST_EXEC__ADCS_pack_to_sd_log_config_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_sd_log_config_struct"
    }, 

    {
        .test_func = TEST_EXEC__ADCS_convert_double_to_string,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_convert_double_to_string"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_file_info_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_file_info_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_download_block_ready_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_download_block_ready_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_sd_card_format_erase_progress_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_sd_card_format_erase_progress_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_file_download_buffer_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_pack_to_file_download_buffer_struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_combine_sd_log_bitmasks,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_combine_sd_log_bitmasks"
    },
    // ****************** END SECTION: test_adcs ******************

    {
        .test_func = TEST_EXEC__TCMD_get_suffix_tag_uint64,
        .test_file = "telecommands/telecommand_parser",
        .test_func_name = "TCMD_get_suffix_tag_uint64"
    },
    {
        .test_func = TEST_EXEC__TCMD_get_suffix_tag_str,
        .test_file = "telecommands/telecommand_parser",
        .test_func_name = "TCMD_get_suffix_tag_str"
    },
    {
        .test_func = TEST_EXEC__TCMD_extract_hex_array_arg,
        .test_file = "telecommands/telecommand_args_helpers",
        .test_func_name = "TCMD_extract_hex_array_arg"
    },
    {
        .test_func = TEST_EXEC__TCMD_extract_uint64_arg,
        .test_file = "telecommands/telecommand_args_helpers",
        .test_func_name = "TCMD_extract_uint64_arg"
    },
    {
        .test_func = TEST_EXEC__TCMD_ascii_to_uint64,
        .test_file = "telecommands/telecommand_args_helpers",
        .test_func_name = "TCMD_ascii_to_uint64"
    },
    {
        .test_func = TEST_EXEC__TCMD_ascii_to_double,
        .test_file = "telecommands/telecommand_args_helpers",
        .test_func_name = "TCMD_ascii_to_double"
    },
    {
        .test_func = TEST_EXEC_TCMD_parse_full_telecommand,
        .test_file = "telecommands/telecommand_parser",
        .test_func_name = "TCMD_parse_full_telecommand"
    },
    {
        .test_func = TEST_EXEC__LOG_enable_disable_LOG_SINK_FILE,
        .test_file = "log/log",
        .test_func_name = "TEST_EXEC__LOG_enable_disable_LOG_SINK_FILE"
    },
    {
        .test_func = TEST_EXEC__LOG_enable_disable_LOG_SINK_UHF_RADIO,
        .test_file = "log/log",
        .test_func_name = "TEST_EXEC__LOG_enable_disable_LOG_SINK_UHF_RADIO"
    },
    {
        .test_func = TEST_EXEC__LOG_enable_disable_LOG_SINK_UMBILICAL_UART,
        .test_file = "log/log",
        .test_func_name = "TEST_EXEC__LOG_enable_disable_LOG_SINK_UMBILICAL_UART"
    },
    {
        .test_func = TEST_EXEC__LOG_enable_disable_LOG_FILE_LOGGING_FOR_SYSTEM_MPI,
        .test_file = "log/log",
        .test_func_name = "TEST_EXEC__LOG_enable_disable_LOG_FILE_LOGGING_FOR_SYSTEM_MPI"
    },
    {
        .test_func = TEST_EXEC__LOG_all_sinks_except,
        .test_file = "log/log",
        .test_func_name = "TEST_EXEC__LOG_all_sinks_except"
    },
    {
        .test_func = TEST_EXEC__setup_configuration_variables,
        .test_file = "configuration/configuration_variables",
        .test_func_name = "TEST_EXEC__setup_configuration_variables"
    },
    {
        .test_func = TEST_EXEC__get_int_var_index,
        .test_file = "configuration/configuration_variables",
        .test_func_name = "TEST_EXEC__get_int_var_index"
    },
    {
        .test_func = TEST_EXEC__get_str_var_index,
        .test_file = "configuration/configuration_variables",
        .test_func_name = "TEST_EXEC__get_str_var_index"
    },
    {
        .test_func = TEST_EXEC__set_int_variable,
        .test_file = "configuration/configuration_variables",
        .test_func_name = "TEST_EXEC__set_int_variable"
    },
    {
        .test_func = TEST_EXEC__set_str_variable,
        .test_file = "configuration/configuration_variables",
        .test_func_name = "TEST_EXEC__set_str_variable"
    },
    {
        .test_func = TEST_EXEC__TCMD_ascii_to_int64,
        .test_file = "telecommands/telecommand_args_helpers",
        .test_func_name = "TCMD_ascii_to_int64"
    },
    {
        .test_func = TEST_EXEC__TCMD_extract_int64_arg,
        .test_file = "telecommands/telecommand_args_helpers",
        .test_func_name = "TCMD_extract_int64_arg"
    },
    {
        .test_func = TEST_EXEC__OBC_TEMP_SENSOR_configure_precision_values,
        .test_file = "temperature_sensor/obc_temperature_sensor_driver",
        .test_func_name = "TEST_EXEC__TEMP_configure_precision_values"
    },
    {
        .test_func = TEST_EXEC__OBC_TEMP_SENSOR_convert_raw_to_deg_c,
        .test_file = "temperature_sensor/obc_temperature_sensor_driver",
        .test_func_name = "TEST_EXEC__TEMP_convert_raw_to_deg_c"
    },
    {
        .test_func = TEST_EXEC__EPS_channel_from_str,
        .test_file = "unit_tests/test_eps_drivers",
        .test_func_name = "TEST_EXEC__EPS_channel_from_str",
    },
    {
        .test_func = TEST_EXEC__EPS_channel_to_str,
        .test_file = "unit_tests/test_eps_drivers",
        .test_func_name = "TEST_EXEC__EPS_channel_to_str",
    },
    {
        .test_func = TEST_EXEC__EPS_check_status_bit_of_channel,
        .test_file = "unit_tests/test_eps_drivers",
        .test_func_name = "TEST_EXEC__EPS_check_status_bit_of_channel",
    },
    {
        .test_func = TEST_EXEC__EPS_check_type_sizes,
        .test_file = "unit_tests/test_eps_struct_packers",
        .test_func_name = "TEST_EXEC__EPS_check_type_sizes",
    },
    {
        .test_func = TEST_EXEC__EPS_convert_battery_voltage_to_percent,
        .test_file = "unit_tests/test_eps_calculations",
        .test_func_name = "TEST_EXEC__EPS_convert_battery_voltage_to_percent"
    },
    {
        .test_func = TEST_EXEC__ANT_convert_raw_temp_to_cCelsius,
        .test_file = "antenna_deploy_drivers/ant_commands",
        .test_func_name = "ANT_convert_raw_temp_to_cCelsius"
    },
    {
        .test_func = TEST_EXEC__CRYPT_compute_sha256_hash,
        .test_file = "unit_tests/test_sha256",
        .test_func_name = "TEST_EXEC__CRYPT_compute_sha256_hash",
    },
    // ****************** SECTION: unit_test_gnss ******************
 
    {
        .test_func = TEST_EXEC__GNSS_reference_time_status_str_to_enum,
        .test_file = "gnss/gnss_types",
        .test_func_name = "GNSS_reference_time_status_str_to_enum"
    },
    {
        .test_func = TEST_EXEC__GNSS_solution_status_str_to_enum,
        .test_file = "gnss/gnss_types",
        .test_func_name = "GNSS_solution_status_str_to_enum"
    },
    {
        .test_func = TEST_EXEC__GNSS_position_type_str_to_enum,
        .test_file = "gnss/gnss_types",
        .test_func_name = "GNSS_position_type_str_to_enum"
    },
    {
        .test_func = TEST_EXEC__GNSS_header_response_parser,
        .test_file = "gnss/gnss_ascii_parsers",
        .test_func_name = "GNSS_header_response_parser"
    },
    {
        .test_func = TEST_EXEC__GNSS_bestxyza_data_parser,
        .test_file = "gnss/gnss_ascii_parsers",
        .test_func_name = "GNSS_bestxyza_data_parser"
    },
    {
        .test_func = TEST_EXEC__GNSS_timea_data_parser,
        .test_file = "gnss/gnss_ascii_parsers",
        .test_func_name = "GNSS_timea_data_parser"
    },

    // ****************** END SECTION: unit_test_gnss ******************
    // ****************** SECTION: unit_test_uart_error_tracking ******************
    {
        .test_func = TEST_EXEC__UART_single_subsystem_error_info_to_json,
        .test_file = "unit_tests/test_uart_error_tracking",
        .test_func_name = "UART_single_subsystem_error_info_to_json"
    },
};

// extern
const int16_t TEST_definitions_count = sizeof(TEST_definitions) / sizeof(TEST_Definition_t);
