#ifndef INCLUDE_GUARD__ADCS_TEST_PROTOTYPES_H__
#define INCLUDE_GUARD__ADCS_TEST_PROTOTYPES_H__

// epsilon for comparing doubles
static const float ADCS_TEST_EPSILON = 1e-6;

// ADCS test definitions
uint8_t TEST_EXEC__ADCS_pack_to_ack_struct();
uint8_t TEST_EXEC__ADCS_pack_to_identification_struct();
uint8_t TEST_EXEC__ADCS_pack_to_program_status_struct();
uint8_t TEST_EXEC__ADCS_pack_to_comms_status_struct();
uint8_t TEST_EXEC__ADCS_pack_to_power_control_struct();
uint8_t TEST_EXEC__ADCS_pack_to_angular_rates_struct();
uint8_t TEST_EXEC__ADCS_pack_to_llh_position_struct();
uint8_t TEST_EXEC__ADCS_pack_to_unix_time_save_mode_struct();
uint8_t TEST_EXEC__ADCS_pack_to_orbit_params_struct();
uint8_t TEST_EXEC__ADCS_pack_to_rated_sensor_rates_struct();
uint8_t TEST_EXEC__ADCS_pack_to_wheel_speed_struct();
uint8_t TEST_EXEC__ADCS_pack_to_magnetorquer_command_struct();
uint8_t TEST_EXEC__ADCS_pack_to_raw_magnetometer_values_struct();
uint8_t TEST_EXEC__ADCS_pack_to_fine_angular_rates_struct();
uint8_t TEST_EXEC__ADCS_pack_to_magnetometer_config_struct();
uint8_t TEST_EXEC__ADCS_pack_to_commanded_attitude_angles_struct();
uint8_t TEST_EXEC__ADCS_pack_to_estimation_params_struct();
uint8_t TEST_EXEC__ADCS_pack_to_augmented_sgp4_params_struct();
uint8_t TEST_EXEC__ADCS_pack_to_tracking_controller_target_reference_struct();
uint8_t TEST_EXEC__ADCS_pack_to_rate_gyro_config_struct();
uint8_t TEST_EXEC__ADCS_pack_to_estimated_attitude_angles_struct();
uint8_t TEST_EXEC__ADCS_pack_to_magnetic_field_vector_struct();
uint8_t TEST_EXEC__ADCS_pack_to_fine_sun_vector_struct();
uint8_t TEST_EXEC__ADCS_pack_to_nadir_vector_struct();
uint8_t TEST_EXEC__ADCS_pack_to_commanded_wheel_speed_struct();
uint8_t TEST_EXEC__ADCS_pack_to_igrf_magnetic_field_vector_struct();
uint8_t TEST_EXEC__ADCS_pack_to_quaternion_error_vector_struct();
uint8_t TEST_EXEC__ADCS_pack_to_estimated_gyro_bias_struct();
uint8_t TEST_EXEC__ADCS_pack_to_estimation_innovation_vector_struct();
uint8_t TEST_EXEC__ADCS_pack_to_raw_cam1_sensor_struct();
uint8_t TEST_EXEC__ADCS_pack_to_raw_cam2_sensor_struct();
uint8_t TEST_EXEC__ADCS_pack_to_raw_coarse_sun_sensor_1_to_6_struct();
uint8_t TEST_EXEC__ADCS_pack_to_raw_coarse_sun_sensor_7_to_10_struct();
uint8_t TEST_EXEC__ADCS_pack_to_cubecontrol_current_struct();
uint8_t TEST_EXEC__ADCS_pack_to_raw_gps_status_struct();
uint8_t TEST_EXEC__ADCS_pack_to_raw_gps_time_struct();
uint8_t TEST_EXEC__ADCS_pack_to_raw_gps_struct();
uint8_t TEST_EXEC__ADCS_pack_to_measurements_struct();
uint8_t TEST_EXEC__ADCS_pack_to_file_info_struct(); 
uint8_t TEST_EXEC__ADCS_pack_to_download_block_ready_struct(); 
uint8_t TEST_EXEC__ADCS_pack_to_sd_card_format_erase_progress_struct(); 
uint8_t TEST_EXEC__ADCS_pack_to_file_download_buffer_struct(); 
uint8_t TEST_EXEC__ADCS_pack_to_acp_execution_state_struct();
uint8_t TEST_EXEC__ADCS_pack_to_current_state_1_struct();
uint8_t TEST_EXEC__ADCS_pack_to_raw_star_tracker_struct();
uint8_t TEST_EXEC__ADCS_pack_to_unix_time_ms();
uint8_t TEST_EXEC__ADCS_pack_to_sd_log_config_struct();
uint8_t TEST_EXEC__ADCS_convert_double_to_string();
uint8_t TEST_EXEC__ADCS_combine_sd_log_bitmasks();

#endif // INCLUDE_GUARD__ADCS_TEST_PROTOTYPES_H__


