
#ifndef __INCLUDE_GUARD__TELECOMMAND_ADCS_H
#define __INCLUDE_GUARD__TELECOMMAND_ADCS_H

#include <stdint.h>
#include "telecommands/telecommand_definitions.h"

// if we fail to properly extract a value, we should return the reason for that failure
#define ABORT_CMD_FOR_FAILED_EXTRACT(x) uint8_t result = x; if (!(result)) { return result; }

uint8_t TCMDEXEC_ADCS_reset(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                            char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_identification(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_program_status(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_communication_status(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                           char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_deploy_magnetometer(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                          char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_set_run_mode(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_clear_errors(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_attitude_control_mode(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_attitude_estimation_mode(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                               char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_ack(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                             char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_run_once(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                               char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_set_magnetometer_mode(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_set_magnetorquer_output(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_set_wheel_speed(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_save_config(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                  char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_estimate_angular_rates(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                             char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_get_llh_position(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                       char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_get_power_control(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_set_power_control(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_set_magnetometer_config(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_bootloader_clear_errors(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_set_unix_time_save_mode(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_get_unix_time_save_mode(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_set_sgp4_orbit_params(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_get_sgp4_orbit_params(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_save_orbit_params(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_rate_sensor_rates(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_get_wheel_speed(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_get_magnetorquer_command(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                               char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_get_raw_magnetometer_values(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                  char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_fine_angular_rates(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                         char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_estimate_fine_angular_rates(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                  char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_get_magnetometer_config(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_get_commanded_attitude_angles(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                    char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_set_commanded_attitude_angles(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                    char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_set_estimation_params(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_get_estimation_params(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_set_asgp4_params(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                       char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_get_asgp4_params(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                       char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_set_tracking_controller_target_reference(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                                char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_get_tracking_controller_target_reference(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                                char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_set_rate_gyro_config(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                           char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_get_rate_gyro_config(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                           char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_estimated_attitude_angles(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_magnetic_field_vector(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_fine_sun_vector(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_nadir_vector(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_commanded_wheel_speed(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                            char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_igrf_magnetic_field_vector(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                 char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_quaternion_error_vector(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                              char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_estimated_gyro_bias(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                          char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_estimation_innovation_vector(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                                   char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_raw_cam1_sensor(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_raw_cam2_sensor(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_raw_css_1_to_6(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_raw_css_7_to_10(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                      char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_cubecontrol_current(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                          char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_raw_gps_status(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                     char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_raw_gps_time(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_raw_gps_x(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_raw_gps_y(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_raw_gps_z(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_ADCS_measurements(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                   char *response_output_buf, uint16_t response_output_buf_len);

#endif // __INCLUDE_GUARD__TELECOMMAND_ADCS_H
