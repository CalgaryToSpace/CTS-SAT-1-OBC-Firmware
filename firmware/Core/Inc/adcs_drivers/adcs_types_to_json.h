#ifndef INC_ADCS_TYPES_TO_JSON_H_
#define INC_ADCS_TYPES_TO_JSON_H_

#include "adcs_drivers/adcs_types.h"

/* Structure: (EPS_vpid_eng_t is the struct)
uint8_t EPS_vpid_eng_TO_json(
    const EPS_vpid_eng_t *data,
    char json_output_str[],
    uint16_t json_output_str_len
);
*/

uint8_t ADCS_cmd_ack_struct_TO_json(const ADCS_cmd_ack_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_id_struct_TO_json(const ADCS_id_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_boot_running_status_struct_TO_json(const ADCS_boot_running_status_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_comms_status_struct_TO_json(const ADCS_comms_status_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_angular_rates_struct_TO_json(const ADCS_angular_rates_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_llh_position_struct_TO_json(const ADCS_llh_position_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_power_control_struct_TO_json(const ADCS_power_control_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_set_unix_time_save_mode_struct_TO_json(const ADCS_set_unix_time_save_mode_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_orbit_params_struct_TO_json(const ADCS_orbit_params_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_rated_sensor_rates_struct_TO_json(const ADCS_rated_sensor_rates_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_wheel_speed_struct_TO_json(const ADCS_wheel_speed_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_magnetorquer_command_struct_TO_json(const ADCS_magnetorquer_command_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_raw_magnetometer_values_struct_TO_json(const ADCS_raw_magnetometer_values_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_fine_angular_rates_struct_TO_json(const ADCS_fine_angular_rates_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_magnetometer_config_struct_TO_json(const ADCS_magnetometer_config_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_commanded_angles_struct_TO_json(const ADCS_commanded_angles_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_estimation_params_struct_TO_json(const ADCS_estimation_params_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_augmented_sgp4_params_struct_TO_json(const ADCS_augmented_sgp4_params_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_tracking_controller_target_struct_TO_json(const ADCS_tracking_controller_target_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_rate_gyro_config_struct_TO_json(const ADCS_rate_gyro_config_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_estimated_attitude_angles_struct_TO_json(const ADCS_estimated_attitude_angles_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_magnetic_field_vector_struct_TO_json(const ADCS_magnetic_field_vector_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_fine_sun_vector_struct_TO_json(const ADCS_fine_sun_vector_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_nadir_vector_struct_TO_json(const ADCS_nadir_vector_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_quaternion_error_vector_struct_TO_json(const ADCS_quaternion_error_vector_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_estimated_gyro_bias_struct_TO_json(const ADCS_estimated_gyro_bias_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_estimation_innovation_vector_struct_TO_json(const ADCS_estimation_innovation_vector_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_raw_cam_sensor_struct_TO_json(const ADCS_raw_cam_sensor_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_raw_coarse_sun_sensor_1_to_6_struct_TO_json(const ADCS_raw_coarse_sun_sensor_1_to_6_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_raw_coarse_sun_sensor_7_to_10_struct_TO_json(const ADCS_raw_coarse_sun_sensor_7_to_10_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_cubecontrol_current_struct_TO_json(const ADCS_cubecontrol_current_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_raw_gps_status_struct_TO_json(const ADCS_raw_gps_status_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_raw_gps_time_struct_TO_json(const ADCS_raw_gps_time_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_raw_gps_struct_TO_json(const ADCS_raw_gps_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_measurements_struct_TO_json(const ADCS_measurements_struct_t *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_sd_download_list_TO_json(ADCS_file_info_struct_t *data, uint16_t data_length, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_generic_telemetry_uint8_array_TO_json(const uint8_t *data, const uint16_t data_length, char json_output_str[], uint16_t json_output_str_len);
#endif /* INC_ADCS_TYPES_TO_JSON_H_ */