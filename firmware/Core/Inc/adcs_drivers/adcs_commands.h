#ifndef INC_ADCS_COMMANDS_H_
#define INC_ADCS_COMMANDS_H_

#include "adcs_types.h"
#include "adcs_internal_drivers.h"

/* Function Definitions */

// ADCS functions
uint8_t ADCS_Reset();
uint8_t ADCS_Get_Identification(ADCS_ID_Struct *output_struct);
uint8_t ADCS_Get_Program_Status(ADCS_Boot_Running_Status_Struct *output_struct);
uint8_t ADCS_Get_Communication_Status(ADCS_Comms_Status_Struct *output_struct);
uint8_t ADCS_Deploy_Magnetometer(uint8_t deploy_timeout);
uint8_t ADCS_Set_Run_Mode(ADCS_Run_Mode mode);
uint8_t ADCS_Clear_Errors();
uint8_t ADCS_Attitude_Control_Mode(ADCS_Control_Mode mode, uint16_t timeout);
uint8_t ADCS_Attitude_Estimation_Mode(ADCS_Estimation_Mode mode);
uint8_t ADCS_CMD_Ack(ADCS_CMD_Ack_Struct *ack);
uint8_t ADCS_Run_Once();
uint8_t ADCS_Set_Magnetometer_Mode(ADCS_Magnetometer_Mode mode);
uint8_t ADCS_Set_Magnetorquer_Output(double x_duty, double y_duty, double z_duty);
uint8_t ADCS_Set_Wheel_Speed(int16_t x_speed, int16_t y_speed, int16_t z_speed);
uint8_t ADCS_Save_Config();
uint8_t ADCS_Get_Estimate_Angular_Rates(ADCS_Angular_Rates_Struct *output_struct);
uint8_t ADCS_Get_LLH_Position(ADCS_LLH_Position_Struct *output_struct);
uint8_t ADCS_Get_Power_Control(ADCS_Power_Control_Struct *output_struct);
uint8_t ADCS_Set_Power_Control(ADCS_Power_Select cube_control_signal, ADCS_Power_Select cube_control_motor, ADCS_Power_Select cube_sense1,
        ADCS_Power_Select cube_sense2, ADCS_Power_Select cube_star_power, ADCS_Power_Select cube_wheel1_power,
        ADCS_Power_Select cube_wheel2_power, ADCS_Power_Select cube_wheel3_power, ADCS_Power_Select motor_power,
        ADCS_Power_Select gps_power);
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
uint8_t ADCS_Get_Unix_Time_Save_Mode(ADCS_Set_Unix_Time_Save_Mode_Struct *output_struct);
uint8_t ADCS_Set_SGP4_Orbit_Params(double inclination, double eccentricity, double ascending_node_right_ascension, double perigee_argument, double b_star_drag_term, double mean_motion, double mean_anomaly, double epoch);
uint8_t ADCS_Get_SGP4_Orbit_Params(ADCS_Orbit_Params_Struct *output_struct);
uint8_t ADCS_Save_Orbit_Params();
uint8_t ADCS_Get_Rate_Sensor_Rates(ADCS_Rated_Sensor_Rates_Struct *output_struct);
uint8_t ADCS_Get_Wheel_Speed(ADCS_Wheel_Speed_Struct *output_struct);
uint8_t ADCS_Get_Magnetorquer_Command(ADCS_Magnetorquer_Command_Struct *output_struct);
uint8_t ADCS_Get_Raw_Magnetometer_Values(ADCS_Raw_Magnetometer_Values_Struct *output_struct);
uint8_t ADCS_Get_Estimate_Fine_Angular_Rates(ADCS_Fine_Angular_Rates_Struct *output_struct);
uint8_t ADCS_Get_Magnetometer_Config(ADCS_Magnetometer_Config_Struct *output_struct);
uint8_t ADCS_Get_Commanded_Attitude_Angles(ADCS_Commanded_Angles_Struct *output_struct);
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
                                ADCS_Magnetometer_Mode magnetometer_mode, // this is actually the same one as for ID 56!
                                ADCS_Magnetometer_Mode magnetometer_selection_for_raw_mtm_tlm, // and so is this, actually!
                                bool automatic_estimation_transition_due_to_rate_sensor_errors, 
                                bool wheel_30s_power_up_delay, // present in CubeSupport but not in the manual -- need to test
                                uint8_t cam1_and_cam2_sampling_period);
uint8_t ADCS_Get_Estimation_Params(ADCS_Estimation_Params_Struct *output_struct);
uint8_t ADCS_Set_Augmented_SGP4_Params(double incl_coefficient,
                           double raan_coefficient,
                           double ecc_coefficient,
                           double aop_coefficient,
                           double time_coefficient,
                           double pos_coefficient,
                           double maximum_position_error,
                           ADCS_Augmented_SGP4_Filter augmented_sgp4_filter,
                           double xp_coefficient,
                           double yp_coefficient,
                           uint8_t gps_roll_over,
                           double position_sd,
                           double velocity_sd,
                           uint8_t min_satellites,
                           double time_gain,
                           double max_lag,
                           uint16_t min_samples);
uint8_t ADCS_Get_Augmented_SGP4_Params(ADCS_Augmented_SGP4_Params_Struct *output_struct);
uint8_t ADCS_Set_Tracking_Controller_Target_Reference(float lon, float lat, float alt);
uint8_t ADCS_Get_Tracking_Controller_Target_Reference(ADCS_Tracking_Controller_Target_Struct *output_struct);
uint8_t ADCS_Set_Rate_Gyro_Config(ADCS_Axis_Select gyro1, ADCS_Axis_Select gyro2, ADCS_Axis_Select gyro3, double x_rate_offset, double y_rate_offset, double z_rate_offset, uint8_t rate_sensor_mult);
uint8_t ADCS_Get_Rate_Gyro_Config(ADCS_Rate_Gyro_Config_Struct *output_struct);
uint8_t ADCS_Get_Estimated_Attitude_Angles(ADCS_Estimated_Attitude_Angles_Struct *output_struct);
uint8_t ADCS_Get_Magnetic_Field_Vector(ADCS_Magnetic_Field_Vector_Struct *output_struct);
uint8_t ADCS_Get_Fine_Sun_Vector(ADCS_Fine_Sun_Vector_Struct *output_struct);
uint8_t ADCS_Get_Nadir_Vector(ADCS_Nadir_Vector_Struct *output_struct);
uint8_t ADCS_Get_Commanded_Wheel_Speed(ADCS_Wheel_Speed_Struct *output_struct);
uint8_t ADCS_Get_IGRF_Magnetic_Field_Vector(ADCS_Magnetic_Field_Vector_Struct *output_struct);
uint8_t ADCS_Get_Quaternion_Error_Vector(ADCS_Quaternion_Error_Vector_Struct *output_struct);
uint8_t ADCS_Get_Estimated_Gyro_Bias(ADCS_Estimated_Gyro_Bias_Struct *output_struct);
uint8_t ADCS_Get_Estimation_Innovation_Vector(ADCS_Estimation_Innovation_Vector_Struct *output_struct);
uint8_t ADCS_Get_Raw_Cam1_Sensor(ADCS_Raw_Cam_Sensor_Struct *output_struct);
uint8_t ADCS_Get_Raw_Cam2_Sensor(ADCS_Raw_Cam_Sensor_Struct *output_struct);
uint8_t ADCS_Get_Raw_Coarse_Sun_Sensor_1_to_6(ADCS_Raw_Coarse_Sun_Sensor_1_to_6_Struct *output_struct);
uint8_t ADCS_Get_Raw_Coarse_Sun_Sensor_7_to_10(ADCS_Raw_Coarse_Sun_Sensor_7_to_10_Struct *output_struct);
uint8_t ADCS_Get_CubeControl_Current(ADCS_CubeControl_Current_Struct *output_struct);
uint8_t ADCS_Get_Raw_GPS_Status(ADCS_Raw_GPS_Status_Struct *output_struct);
uint8_t ADCS_Get_Raw_GPS_Time(ADCS_Raw_GPS_Time_Struct *output_struct);
uint8_t ADCS_Get_Raw_GPS_X(ADCS_Raw_GPS_Struct *output_struct);
uint8_t ADCS_Get_Raw_GPS_Y(ADCS_Raw_GPS_Struct *output_struct);
uint8_t ADCS_Get_Raw_GPS_Z(ADCS_Raw_GPS_Struct *output_struct);
uint8_t ADCS_Get_Measurements(ADCS_Measurements_Struct *output_struct);

#endif /* INC_ADCS_COMMANDS_H_ */