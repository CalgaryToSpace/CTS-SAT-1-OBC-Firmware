
#ifndef INCLUDE_GUARD__TELECOMMAND_ADCS_H
#define INCLUDE_GUARD__TELECOMMAND_ADCS_H

#include <stdint.h>
#include "telecommand_exec/telecommand_definitions.h"

// if we fail to properly extract a value, we should return the reason for that failure
#define ABORT_CMD_FOR_FAILED_EXTRACT(x) uint8_t result = x; if (!(result)) { return result; }
#define CHECK_ADCS_COMMAND_SUCCESS(x) if ((x)) { return x; }

uint8_t TCMDEXEC_adcs_reset(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_identification(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_program_status(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_communication_status(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_deploy_magnetometer(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_set_run_mode(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_clear_errors(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_attitude_control_mode(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_attitude_estimation_mode(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_ack(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_run_once(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_set_magnetometer_mode(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_set_magnetorquer_output(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_set_wheel_speed(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_save_config(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_estimate_angular_rates(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_get_llh_position(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_get_power_control(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_set_power_control(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);
      
uint8_t TCMDEXEC_adcs_enter_low_power_mode(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_track_sun(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_set_magnetometer_config(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_bootloader_clear_errors(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_set_unix_time_save_mode(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_get_unix_time_save_mode(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_set_sgp4_orbit_params(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_get_sgp4_orbit_params(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_save_orbit_params(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_rate_sensor_rates(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_get_wheel_speed(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_get_magnetorquer_command(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_get_raw_magnetometer_values(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_fine_angular_rates(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_estimate_fine_angular_rates(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_get_magnetometer_config(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_get_commanded_attitude_angles(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_set_commanded_attitude_angles(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_set_estimation_params(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_get_estimation_params(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_set_augmented_sgp4_params(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_get_augmented_sgp4_params(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_set_tracking_controller_target_reference(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_get_tracking_controller_target_reference(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_set_rate_gyro_config(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_get_rate_gyro_config(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_estimated_attitude_angles(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_magnetic_field_vector(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_fine_sun_vector(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_nadir_vector(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_commanded_wheel_speed(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_igrf_magnetic_field_vector(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_quaternion_error_vector(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_estimated_gyro_bias(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_estimation_innovation_vector(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_raw_cam1_sensor(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_raw_cam2_sensor(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_raw_coarse_sun_sensor_1_to_6(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_raw_coarse_sun_sensor_7_to_10(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_cubecontrol_current(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_measurements(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_generic_command(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_generic_telemetry_request(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_download_sd_file(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);
            
uint8_t TCMDEXEC_adcs_acp_execution_state(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_get_current_state_1(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_save_image_to_sd(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_request_commissioning_telemetry(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_synchronize_unix_time(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_get_current_unix_time(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_set_sd_log_config(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_get_sd_log_config(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_format_sd(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_download_index_file(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);
                         
uint8_t TCMDEXEC_adcs_set_commissioning_modes(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_erase_sd_file(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_adcs_exit_bootloader(const char *args_str, char *response_output_buf, uint16_t response_output_buf_len);

#endif // INCLUDE_GUARD__TELECOMMAND_adcs_H
