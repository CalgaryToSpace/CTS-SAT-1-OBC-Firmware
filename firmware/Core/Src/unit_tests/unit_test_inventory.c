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
#include "unit_tests/test_eps_drivers.h"
#include "unit_tests/test_eps_struct_packers.h"

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
        .test_func = TEST_EXEC__GEN_convert_big_endian_four_byte_array_to_uint32,
        .test_file = "transforms/byte_transforms",
        .test_func_name = "GEN_convert_big_endian_four_byte_array_to_uint32"
    },
    
    // ****************** SECTION: test_adcs ******************
    
    {
		.test_func = TEST_EXEC__ADCS_pack_to_ack_struct,
		.test_file = "unit_tests/test_adcs",
		.test_func_name = "ADCS_Pack_to_Ack_Struct"
	},

    {
		.test_func = TEST_EXEC__ADCS_pack_to_identification_struct,
		.test_file = "unit_tests/test_adcs",
		.test_func_name = "ADCS_Pack_to_Identification_Struct"
	},

    {
		.test_func = TEST_EXEC__ADCS_pack_to_program_status_struct,
		.test_file = "unit_tests/test_adcs",
		.test_func_name = "ADCS_Pack_to_Program_Status_Struct"
	},

    {
		.test_func = TEST_EXEC__ADCS_pack_to_comms_status_struct,
		.test_file = "unit_tests/test_adcs",
		.test_func_name = "ADCS_Pack_to_Comms_Status_Struct"
	},

    {
        .test_func = TEST_EXEC__ADCS_pack_to_power_control_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Power_Control_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_angular_rates_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Angular_Rates_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_llh_position_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_LLH_Position_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_unix_time_save_mode_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Unix_Time_Save_Mode_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_orbit_params_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Orbit_Params_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_rated_sensor_rates_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Rated_Sensor_Rates_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_wheel_speed_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Wheel_Speed_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_magnetorquer_command_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Magnetorquer_Command_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_raw_magnetometer_values_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Raw_Magnetometer_Values_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_fine_angular_rates_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Fine_Angular_Rates_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_magnetometer_config_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Magnetometer_Config_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_commanded_attitude_angles_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Commanded_Attitude_Angles_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_estimation_params_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Estimation_Params_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_augmented_sgp4_params_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Augmented_SGP4_Params_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_tracking_controller_target_reference_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Tracking_Controller_Target_Reference_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_rate_gyro_config_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Rate_Gyro_Config_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_estimated_attitude_angles_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Estimated_Attitude_Angles_Struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_magnetic_field_vector_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Magnetic_Field_Vector_Struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_fine_sun_vector_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Fine_Sun_Vector_Struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_nadir_vector_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Nadir_Vector_Struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_commanded_wheel_speed_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Commanded_Wheel_Speed_Struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_igrf_magnetic_field_vector_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_IGRF_Magnetic_Field_Vector_Struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_quaternion_error_vector_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Quaternion_Error_Vector_Struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_estimated_gyro_bias_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Estimated_Gyro_Bias_Struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_estimation_innovation_vector_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Estimation_Innovation_Vector_Struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_raw_cam1_sensor_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Raw_Cam1_Sensor_Struct"
    },
    
    {
        .test_func = TEST_EXEC__ADCS_pack_to_raw_cam2_sensor_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Raw_Cam2_Sensor_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_raw_coarse_sun_sensor_1_to_6_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Raw_Coarse_Sun_Sensor_1_to_6_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_raw_coarse_sun_sensor_7_to_10_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Raw_Coarse_Sun_Sensor_7_to_10_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_cubecontrol_current_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_CubeControl_Current_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_raw_gps_status_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Raw_GPS_Status_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_raw_gps_time_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Raw_GPS_Time_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_raw_gps_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Raw_GPS_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_measurements_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_Measurements_Struct"
    },

    {
        .test_func = TEST_EXEC__ADCS_pack_to_acp_execution_state_struct,
        .test_file = "unit_tests/test_adcs",
        .test_func_name = "ADCS_Pack_to_ACP_Execution_State_Struct"
    },

    // ****************** END SECTION: test_adcs ******************

    {
        .test_func = TEST_EXEC__TCMD_get_suffix_tag_uint64,
        .test_file = "telecommands/telecommand_parser",
        .test_func_name = "TCMD_get_suffix_tag_uint64"
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
        .test_func = TEST_EXEC__EPS_channel_from_str,
        .test_file = "unit_tests/test_eps_drivers",
        .test_func_name = "TEST_EXEC__EPS_channel_from_str",
    },
    {
        .test_func = TEST_EXEC__EPS_check_type_sizes,
        .test_file = "unit_tests/test_eps_struct_packers",
        .test_func_name = "TEST_EXEC__EPS_check_type_sizes",
    },
    {
        .test_func = TEST_EXEC__ANT_convert_raw_temp_to_cCelsius,
        .test_file = "antenna_deploy_drivers/ant_commands",
        .test_func_name = "ANT_convert_raw_temp_to_cCelsius"
    },
};

// extern
const int16_t TEST_definitions_count = sizeof(TEST_definitions) / sizeof(TEST_Definition_t);
