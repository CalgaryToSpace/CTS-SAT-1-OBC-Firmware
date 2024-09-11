#ifndef INC_ADCS_COMMANDS_H_
#define INC_ADCS_COMMANDS_H_

#include "adcs_types.h"
#include "adcs_internal_drivers.h"

/* Function Definitions */

// ADCS functions
uint8_t ADCS_Reset();
uint8_t ADCS_Get_Identification(ADCS_id_struct_t *output_struct);
uint8_t ADCS_Get_Program_Status(ADCS_boot_running_status_struct_t *output_struct);
uint8_t ADCS_Get_Communication_Status(ADCS_comms_status_struct_t *output_struct);
uint8_t ADCS_Deploy_Magnetometer(uint8_t deploy_timeout);
uint8_t ADCS_Set_Run_Mode(ADCS_run_mode_enum_t mode);
uint8_t ADCS_Clear_Errors();
uint8_t ADCS_Attitude_Control_Mode(ADCS_control_mode_enum_t mode, uint16_t timeout);
uint8_t ADCS_Attitude_Estimation_Mode(ADCS_estimation_mode_enum_t mode);
uint8_t ADCS_CMD_Ack(ADCS_cmd_ack_struct_t *ack);
uint8_t ADCS_Run_Once();
uint8_t ADCS_Set_Magnetometer_Mode(ADCS_magnetometer_mode_enum_t mode);
uint8_t ADCS_Set_Magnetorquer_Output(double x_duty, double y_duty, double z_duty);
uint8_t ADCS_Set_Wheel_Speed(int16_t x_speed, int16_t y_speed, int16_t z_speed);
uint8_t ADCS_Save_Config();
uint8_t ADCS_Get_Estimate_Angular_Rates(ADCS_angular_rates_struct_t *output_struct);
uint8_t ADCS_Get_LLH_Position(ADCS_llh_position_struct_t *output_struct);
uint8_t ADCS_Get_Power_Control(ADCS_power_control_struct_t *output_struct);
uint8_t ADCS_Set_Power_Control(ADCS_power_select_enum_t cube_control_signal, ADCS_power_select_enum_t cube_control_motor, ADCS_power_select_enum_t cube_sense1,
        ADCS_power_select_enum_t cube_sense2, ADCS_power_select_enum_t cube_star_power, ADCS_power_select_enum_t cube_wheel1_power,
        ADCS_power_select_enum_t cube_wheel2_power, ADCS_power_select_enum_t cube_wheel3_power, ADCS_power_select_enum_t motor_power,
        ADCS_power_select_enum_t gps_power);
uint8_t ADCS_Set_Magnetometer_Config(
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
uint8_t ADCS_Bootloader_Clear_Errors();
uint8_t ADCS_Set_Unix_Time_Save_Mode(bool save_now, bool save_on_update, bool save_periodic, uint8_t period);
uint8_t ADCS_Get_Unix_Time_Save_Mode(ADCS_set_unix_time_save_mode_struct_t *output_struct);
uint8_t ADCS_Set_SGP4_Orbit_Params(double inclination, double eccentricity, double ascending_node_right_ascension, double perigee_argument, double b_star_drag_term, double mean_motion, double mean_anomaly, double epoch);
uint8_t ADCS_Get_SGP4_Orbit_Params(ADCS_orbit_params_struct_t *output_struct);
uint8_t ADCS_Save_Orbit_Params();
uint8_t ADCS_Get_Rate_Sensor_Rates(ADCS_rated_sensor_rates_struct_t *output_struct);
uint8_t ADCS_Get_Wheel_Speed(ADCS_wheel_speed_struct_t *output_struct);
uint8_t ADCS_Get_Magnetorquer_Command(ADCS_magnetorquer_command_struct_t *output_struct);
uint8_t ADCS_Get_Raw_Magnetometer_Values(ADCS_raw_magnetometer_values_struct_t *output_struct);
uint8_t ADCS_Get_Estimate_Fine_Angular_Rates(ADCS_fine_angular_rates_struct_t *output_struct);
uint8_t ADCS_Get_Magnetometer_Config(ADCS_magnetometer_config_struct_t *output_struct);
uint8_t ADCS_Get_Commanded_Attitude_Angles(ADCS_commanded_angles_struct_t *output_struct);
uint8_t ADCS_Set_Commanded_Attitude_Angles(double x, double y, double z);
uint8_t ADCS_Set_Estimation_Params(
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
uint8_t ADCS_Get_Estimation_Params(ADCS_estimation_params_struct_t *output_struct);
uint8_t ADCS_Set_Augmented_SGP4_Params(double incl_coefficient,
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
uint8_t ADCS_Get_Augmented_SGP4_Params(ADCS_augmented_sgp4_params_struct_t *output_struct);
uint8_t ADCS_Set_Tracking_Controller_Target_Reference(float lon, float lat, float alt);
uint8_t ADCS_Get_Tracking_Controller_Target_Reference(ADCS_tracking_controller_target_struct_t *output_struct);
uint8_t ADCS_Set_Rate_Gyro_Config(ADCS_axis_select_enum_t gyro1, ADCS_axis_select_enum_t gyro2, ADCS_axis_select_enum_t gyro3, double x_rate_offset, double y_rate_offset, double z_rate_offset, uint8_t rate_sensor_mult);
uint8_t ADCS_Get_Rate_Gyro_Config(ADCS_rate_gyro_config_struct_t *output_struct);
uint8_t ADCS_Get_Estimated_Attitude_Angles(ADCS_estimated_attitude_angles_struct_t *output_struct);
uint8_t ADCS_Get_Magnetic_Field_Vector(ADCS_magnetic_field_vector_struct_t *output_struct);
uint8_t ADCS_Get_Fine_Sun_Vector(ADCS_fine_sun_vector_struct_t *output_struct);
uint8_t ADCS_Get_Nadir_Vector(ADCS_nadir_vector_struct_t *output_struct);
uint8_t ADCS_Get_Commanded_Wheel_Speed(ADCS_wheel_speed_struct_t *output_struct);
uint8_t ADCS_Get_IGRF_Magnetic_Field_Vector(ADCS_magnetic_field_vector_struct_t *output_struct);
uint8_t ADCS_Get_Quaternion_Error_Vector(ADCS_quaternion_error_vector_struct_t *output_struct);
uint8_t ADCS_Get_Estimated_Gyro_Bias(ADCS_estimated_gyro_bias_struct_t *output_struct);
uint8_t ADCS_Get_Estimation_Innovation_Vector(ADCS_estimation_innovation_vector_struct_t *output_struct);
uint8_t ADCS_Get_Raw_Cam1_Sensor(ADCS_raw_cam_sensor_struct_t *output_struct);
uint8_t ADCS_Get_Raw_Cam2_Sensor(ADCS_raw_cam_sensor_struct_t *output_struct);
uint8_t ADCS_Get_Raw_Coarse_Sun_Sensor_1_to_6(ADCS_raw_coarse_sun_sensor_1_to_6_struct_t *output_struct);
uint8_t ADCS_Get_Raw_Coarse_Sun_Sensor_7_to_10(ADCS_raw_coarse_sun_sensor_7_to_10_struct_t *output_struct);
uint8_t ADCS_Get_CubeControl_Current(ADCS_cubecontrol_current_struct_t *output_struct);
uint8_t ADCS_Get_Raw_GPS_Status(ADCS_raw_gps_status_struct_t *output_struct);
uint8_t ADCS_Get_Raw_GPS_Time(ADCS_raw_gps_time_struct_t *output_struct);
uint8_t ADCS_Get_Raw_GPS_X(ADCS_raw_gps_struct_t *output_struct);
uint8_t ADCS_Get_Raw_GPS_Y(ADCS_raw_gps_struct_t *output_struct);
uint8_t ADCS_Get_Raw_GPS_Z(ADCS_raw_gps_struct_t *output_struct);
uint8_t ADCS_Get_Measurements(ADCS_measurements_struct_t *output_struct);

#endif /* INC_ADCS_COMMANDS_H_ */