#ifndef INC_ADCS_COMMANDS_H_
#define INC_ADCS_COMMANDS_H_

#include "adcs_drivers/adcs_types.h"
#include "adcs_drivers/adcs_internal_drivers.h"

/* Function Definitions */

// ADCS functions
uint8_t ADCS_initialise();
uint8_t ADCS_reset();
uint8_t ADCS_get_identification(ADCS_id_struct_t *output_struct);
uint8_t ADCS_get_program_status(ADCS_boot_running_status_struct_t *output_struct);
uint8_t ADCS_get_communication_status(ADCS_comms_status_struct_t *output_struct);
uint8_t ADCS_deploy_magnetometer(uint8_t deploy_timeout);
uint8_t ADCS_set_run_mode(ADCS_run_mode_enum_t mode);
uint8_t ADCS_clear_errors();
uint8_t ADCS_attitude_control_mode(ADCS_control_mode_enum_t mode, uint16_t timeout);
uint8_t ADCS_attitude_estimation_mode(ADCS_estimation_mode_enum_t mode);
uint8_t ADCS_cmd_ack(ADCS_cmd_ack_struct_t *ack);
uint8_t ADCS_run_once();
uint8_t ADCS_set_magnetometer_mode(ADCS_magnetometer_mode_enum_t mode);
uint8_t ADCS_set_magnetorquer_output(double x_duty, double y_duty, double z_duty);
uint8_t ADCS_set_wheel_speed(int16_t x_speed, int16_t y_speed, int16_t z_speed);
uint8_t ADCS_save_config();
uint8_t ADCS_get_estimate_angular_rates(ADCS_angular_rates_struct_t *output_struct);
uint8_t ADCS_get_llh_position(ADCS_llh_position_struct_t *output_struct);
uint8_t ADCS_get_power_control(ADCS_power_control_struct_t *output_struct);
uint8_t ADCS_set_power_control(ADCS_power_select_enum_t cube_control_signal, ADCS_power_select_enum_t cube_control_motor, ADCS_power_select_enum_t cube_sense1,
        ADCS_power_select_enum_t cube_sense2, ADCS_power_select_enum_t cube_star_power, ADCS_power_select_enum_t cube_wheel1_power,
        ADCS_power_select_enum_t cube_wheel2_power, ADCS_power_select_enum_t cube_wheel3_power, ADCS_power_select_enum_t motor_power,
        ADCS_power_select_enum_t gps_power);
uint8_t ADCS_set_magnetometer_config(
        double mounting_transform_alpha_angle,
        double mounting_transform_beta_angle,
        double mounting_transform_gamma_angle,
        double channel_1_offset,
        double channel_2_offset,
        double channel_3_offset,
        double sensitivity_matrix_s11,
        double sensitivity_matrix_s22,
        double sensitivity_matrix_s33,
        double sensitivity_matrix_s12,
        double sensitivity_matrix_s13,
        double sensitivity_matrix_s21,
        double sensitivity_matrix_s23,
        double sensitivity_matrix_s31,
        double sensitivity_matrix_s32);
uint8_t ADCS_bootloader_clear_errors();
uint8_t ADCS_set_unix_time_save_mode(bool save_now, bool save_on_update, bool save_periodic, uint8_t period);
uint8_t ADCS_get_unix_time_save_mode(ADCS_set_unix_time_save_mode_struct_t *output_struct);
uint8_t ADCS_set_sgp4_orbit_params(double inclination, double eccentricity, double ascending_node_right_ascension, double perigee_argument, double b_star_drag_term, double mean_motion, double mean_anomaly, double epoch);
uint8_t ADCS_get_sgp4_orbit_params(ADCS_orbit_params_struct_t *output_struct);
uint8_t ADCS_save_orbit_params();
uint8_t ADCS_get_rate_sensor_rates(ADCS_rated_sensor_rates_struct_t *output_struct);
uint8_t ADCS_get_wheel_speed(ADCS_wheel_speed_struct_t *output_struct);
uint8_t ADCS_get_magnetorquer_command(ADCS_magnetorquer_command_struct_t *output_struct);
uint8_t ADCS_get_raw_magnetometer_values(ADCS_raw_magnetometer_values_struct_t *output_struct);
uint8_t ADCS_get_estimate_fine_angular_rates(ADCS_fine_angular_rates_struct_t *output_struct);
uint8_t ADCS_get_magnetometer_config(ADCS_magnetometer_config_struct_t *output_struct);
uint8_t ADCS_get_commanded_attitude_angles(ADCS_commanded_angles_struct_t *output_struct);
uint8_t ADCS_set_commanded_attitude_angles(double x, double y, double z);
uint8_t ADCS_set_estimation_params(
                                float magnetometer_rate_filter_system_noise, 
                                float extended_kalman_filter_system_noise, 
                                float coarse_sun_sensor_measurement_noise, 
                                float sun_sensor_measurement_noise, 
                                float nadir_sensor_measurement_noise, 
                                float magnetometer_measurement_noise, 
                                float star_tracker_measurement_noise, 
                                bool use_sun_sensor, 
                                bool use_nadir_sensor, 
                                bool use_css, 
                                bool use_star_tracker, 
                                bool nadir_sensor_terminator_test, 
                                bool automatic_magnetometer_recovery, 
                                ADCS_magnetometer_mode_enum_t magnetometer_mode, // this is actually the same one as for ID 56!
                                ADCS_magnetometer_mode_enum_t magnetometer_selection_for_raw_magnetometer_telemetry, // and so is this, actually!
                                bool automatic_estimation_transition_due_to_rate_sensor_errors, 
                                bool wheel_30s_power_up_delay, // present in CubeSupport but not in the manual -- need to test
                                uint8_t cam1_and_cam2_sampling_period);
uint8_t ADCS_get_estimation_params(ADCS_estimation_params_struct_t *output_struct);
uint8_t ADCS_set_augmented_sgp4_params(double incl_coefficient,
                           double raan_coefficient,
                           double ecc_coefficient,
                           double aop_coefficient,
                           double time_coefficient,
                           double pos_coefficient,
                           double maximum_position_error,
                           ADCS_augmented_sgp4_filter_enum_t augmented_sgp4_filter,
                           double xp_coefficient,
                           double yp_coefficient,
                           uint8_t gps_roll_over,
                           double position_sd,
                           double velocity_sd,
                           uint8_t min_satellites,
                           double time_gain,
                           double max_lag,
                           uint16_t min_samples);
uint8_t ADCS_get_augmented_sgp4_params(ADCS_augmented_sgp4_params_struct_t *output_struct);
uint8_t ADCS_set_tracking_controller_target_reference(float lon, float lat, float alt);
uint8_t ADCS_get_tracking_controller_target_reference(ADCS_tracking_controller_target_struct_t *output_struct);
uint8_t ADCS_set_rate_gyro_config(ADCS_axis_select_enum_t gyro1, ADCS_axis_select_enum_t gyro2, ADCS_axis_select_enum_t gyro3, double x_rate_offset, double y_rate_offset, double z_rate_offset, uint8_t rate_sensor_mult);
uint8_t ADCS_get_rate_gyro_config(ADCS_rate_gyro_config_struct_t *output_struct);
uint8_t ADCS_get_estimated_attitude_angles(ADCS_estimated_attitude_angles_struct_t *output_struct);
uint8_t ADCS_get_magnetic_field_vector(ADCS_magnetic_field_vector_struct_t *output_struct);
uint8_t ADCS_get_fine_sun_vector(ADCS_fine_sun_vector_struct_t *output_struct);
uint8_t ADCS_get_nadir_vector(ADCS_nadir_vector_struct_t *output_struct);
uint8_t ADCS_get_commanded_wheel_speed(ADCS_wheel_speed_struct_t *output_struct);
uint8_t ADCS_get_igrf_magnetic_field_vector(ADCS_magnetic_field_vector_struct_t *output_struct);
uint8_t ADCS_get_quaternion_error_vector(ADCS_quaternion_error_vector_struct_t *output_struct);
uint8_t ADCS_get_estimated_gyro_bias(ADCS_estimated_gyro_bias_struct_t *output_struct);
uint8_t ADCS_get_estimation_innovation_vector(ADCS_estimation_innovation_vector_struct_t *output_struct);
uint8_t ADCS_get_raw_cam1_sensor(ADCS_raw_cam_sensor_struct_t *output_struct);
uint8_t ADCS_get_raw_cam2_sensor(ADCS_raw_cam_sensor_struct_t *output_struct);
uint8_t ADCS_get_raw_coarse_sun_sensor_1_to_6(ADCS_raw_coarse_sun_sensor_1_to_6_struct_t *output_struct);
uint8_t ADCS_get_raw_coarse_sun_sensor_7_to_10(ADCS_raw_coarse_sun_sensor_7_to_10_struct_t *output_struct);
uint8_t ADCS_get_cubecontrol_current(ADCS_cubecontrol_current_struct_t *output_struct);
uint8_t ADCS_get_raw_gps_status(ADCS_raw_gps_status_struct_t *output_struct);
uint8_t ADCS_get_raw_gps_time(ADCS_raw_gps_time_struct_t *output_struct);
uint8_t ADCS_get_raw_gps_x(ADCS_raw_gps_struct_t *output_struct);
uint8_t ADCS_get_raw_gps_y(ADCS_raw_gps_struct_t *output_struct);
uint8_t ADCS_get_raw_gps_z(ADCS_raw_gps_struct_t *output_struct);
uint8_t ADCS_get_measurements(ADCS_measurements_struct_t *output_struct);
uint8_t ADCS_reset_file_list_read_pointer();
uint8_t ADCS_advance_file_list_read_pointer();
uint8_t ADCS_get_file_info_telemetry(ADCS_file_info_struct_t *output_struct);
uint8_t ADCS_load_file_download_block(ADCS_file_type_enum_t file_type, uint8_t counter, uint32_t offset, uint16_t block_length);
uint8_t ADCS_get_download_block_ready_telemetry(ADCS_download_block_ready_struct_t *output_struct);
uint8_t ADCS_initiate_download_burst(uint8_t message_length, bool ignore_hole_map);
uint8_t ADCS_set_hole_map(uint8_t *hole_map, uint8_t which_map);
uint8_t ADCS_get_hole_map(uint8_t *hole_map_struct, uint8_t which_map);
uint8_t ADCS_format_sd();
uint8_t ADCS_erase_file(ADCS_file_type_enum_t filetype, uint8_t filecounter, bool erase_all);
uint8_t ADCS_get_sd_card_format_erase_progress(ADCS_sd_card_format_erase_progress_struct_t *output_struct);
uint8_t ADCS_get_file_download_buffer(ADCS_file_download_buffer_struct_t *output_struct);
uint8_t ADCS_get_acp_execution_state(ADCS_acp_execution_state_struct_t *output_struct);
uint8_t ADCS_get_current_state_1(ADCS_current_state_1_struct_t *output_struct);
uint8_t ADCS_get_raw_star_tracker_data(ADCS_raw_star_tracker_struct_t *output_struct);
uint8_t ADCS_save_image_to_sd(ADCS_camera_select_enum_t camera_select, ADCS_image_size_enum_t image_size);
uint8_t ADCS_get_current_unix_time();
uint8_t ADCS_synchronise_unix_time();
uint8_t ADCS_set_sd_log_config(uint8_t which_log, const uint8_t **log_array, uint8_t log_array_size, uint16_t log_period, ADCS_sd_log_destination_enum_t which_sd);
uint8_t ADCS_get_sd_log_config(uint8_t which_log, ADCS_sd_log_config_struct* config);
#endif /* INC_ADCS_COMMANDS_H_ */