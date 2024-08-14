/*
 * adcs_types.h
 *
 *  Created on: Oct 28, 2023
 *      Authors: Saksham Puri, Zachary Uy, Nadeem Moosa
 */

#ifndef INC_ADCS_TYPES_H_
#define INC_ADCS_TYPES_H_

#include "main.h"

#include <stdint.h>
#include <stdbool.h>

// Bit 7 of Telecommand and Telemetry Request - Section 4.1 of Firmware Manual
static const uint8_t ADCS_TELECOMMAND = 0;
static const uint8_t ADCS_TELEMETRY_REQUEST = 1;

// ADCS Magic Number defined in Section 5.1 Table 10 of Firmware Manual
static const uint8_t ADCS_MAGIC_NUMBER = 0x5A; // Used to make sure that the reset command is valid

// I2C Write and Read Addresses - Section 5.3 Table 5 of Firmware Manual
static const uint8_t ADCS_I2C_WRITE = 0xAE;
static const uint8_t ADCS_I2C_READ = 0xAF;
static const uint8_t ADCS_I2C_ADDRESS = 0x57;

// Identifiers of UART Start of Message and End of Message - Section 4.2 of Firmware Manual
// 0x1F | 0x7F ....... | 0x1F 0xFF
static const uint8_t ADCS_ESC_CHARACTER = 0x1F;
static const uint8_t ADCS_START_MESSAGE = 0x7F;
static const uint8_t ADCS_END_MESSAGE = 0xFF;

// CRC defines
static const uint8_t ADCS_CRC_POLY = 0x91;

// define for timeout
static const uint16_t ADCS_HAL_TIMEOUT = 1000;

// epsilon for comparing doubles
static const uint8_t ADCS_TEST_EPSILON = 1e-6;

/* Enumerated Values */

// Telecommand Error Flags - Section 5.2.2 Figure 6 of Firmware Manual
typedef enum TC_Error_Flag {
	TC_Error_None,
	TC_Error_Invalid_ID,
	TC_Error_Wrong_Length,
	TC_Error_Invalid_Params,
	TC_Error_CRC
} TC_Error_Flag;

// ADCS Run Modes - Section 5.3.1 Table 75 of Firmware Manual
typedef enum ADCS_Run_Mode {
	ADCS_Run_Mode_Off,
	ADCS_Run_Mode_Enabled,
	ADCS_Run_Mode_Triggered,
	ADCS_Run_Mode_Simulation
} ADCS_Run_Mode;

// ADCS Reset Causes - Section 6.1.2 Table 28 of Firmware Manual
typedef enum ADCS_Reset_Cause {
    ADCS_Power_On_Reset,
    ADCS_Brown_Out_Detected_On_Regulated_Power,
    ADCS_Brown_Out_Detected_On_Unregulated_Power,
    ADCS_External_Watchdog_Reset,
    ADCS_External_Reset,
    ADCS_Watchdog_Reset,
    ADCS_Lockup_System_Reset,
    ADCS_Lockup_Reset,
    ADCS_System_Request_Reset,
    ADCS_Backup_Brown_Out,
    ADCS_Backup_Mode_Reset,
    ADCS_Backup_Mode_Reset_And_Backup_Brown_Out_Vdd_Regulated,
    ADCS_Backup_Mode_Reset_And_Backup_Brown_Out_Vdd_Regulated_And_Brown_Out_Regulated,
    ADCS_Backup_Mode_Reset_And_Watchdog_Reset,
    ADCS_Backup_Brown_Out_Buvin_And_System_Request_Reset,
    ADCS_Unknown_Reset_Cause
} ADCS_Reset_Cause;

// ADCS Boot Causes - Section 6.1.2 Table 29 of Firmware Manual
typedef enum ADCS_Boot_Cause {
    ADCS_Boot_Cause_Unexpected,
    ADCS_Boot_Cause_Not_Used_1,
    ADCS_Boot_Cause_Communications_Timeout,
    ADCS_Boot_Cause_Commanded,
    ADCS_Boot_Cause_Not_Used_2,
    ADCS_Boot_Cause_SRAM_Latchup
} ADCS_Boot_Cause;

// ADCS Running Programs - Section 6.1.2 Table 30 of Firmware Manual
typedef enum ADCS_Running_Program {
    ADCS_Running_ADCS = 1,
    ADCS_Running_Bootloader = 2
} ADCS_Running_Program;

// ADCS Control Modes - Section 6.3.1 Table 77 of Firmware Manual
typedef enum ADCS_Control_Mode {
    ADCS_Control_Mode_None,
    ADCS_Control_Mode_Detumbling,
    ADCS_Control_Mode_Y_Thomson_Spin,
    ADCS_Control_Mode_Y_Wheel_Momentum_Stabilized_Initial_Pitch_Acquisition,
    ADCS_Control_Mode_Y_Wheel_Momentum_Stabilized_Steady_State,
    ADCS_Control_Mode_XYZ_Wheel,
    ADCS_Control_Mode_Rwheel_Sun_Tracking,
    ADCS_Control_Mode_Rwheel_Target_Tracking,
    ADCS_Control_Mode_Very_Fast_Spin_Detumbling,
    ADCS_Control_Mode_Fast_Spin_Detumbling,
    ADCS_Control_Mode_User_Specific_1,
    ADCS_Control_Mode_User_Specific_2,
    ADCS_Control_Mode_Stop_R_Wheels,
    ADCS_Control_Mode_User_Coded,
    ADCS_Control_Mode_Sun_Tracking_Yaw_Or_Roll_Only_Wheel,
    ADCS_Control_Mode_Target_Tracking_Yaw_Only_Wheel
} ADCS_Control_Mode;

typedef enum ADCS_Estimation_Mode {
    ADCS_Estimation_Mode_None,
    ADCS_Estimation_Mode_MEMS_Rate_Sensing,
    ADCS_Estimation_Mode_Magnetometer_Rate_Filter,
    ADCS_Estimation_Mode_Magnetometer_Rate_Filter_With_Pitch_Estimation,
    ADCS_Estimation_Mode_Magnetometer_And_Fine_Sun_TRIAD_Algorithm,
    ADCS_Estimation_Mode_Full_State_EKF,
    ADCS_Estimation_Mode_MEMS_Gyro_EKF,
    ADCS_Estimation_Mode_User_Coded_Estimation_Mode
} ADCS_Estimation_Mode;

typedef enum ADCS_Power_Select {
	ADCS_Power_Select_Off = 0,
	ADCS_Power_Select_On = 1,
	ADCS_Power_Select_Same = 2
} ADCS_Power_Select;

typedef enum ADCS_Magnetometer_Mode {
	ADCS_Magnetometer_Mode_Main_Signal,
	ADCS_Magnetometer_Mode_Redundant_Signal,
	ADCS_Magnetometer_Mode_Main_Motor,
	ADCS_Magnetometer_Mode_None
} ADCS_Magnetometer_Mode;

typedef enum ADCS_ASGP4_Filter {
    ADCS_ASGP4_Filter_Lowpass,
    ADCS_ASGP4_Filter_Average
} ADCS_ASGP4_Filter;

typedef enum ADCS_Axis_Select {
    ADCS_Positive_X,    
    ADCS_Negative_X,    
    ADCS_Positive_Y,
    ADCS_Negative_Y,
    ADCS_Positive_Z,
    ADCS_Negative_Z,
    ADCS_Not_Used
} ADCS_Axis_Select;

typedef enum ADCS_Capture_Result {
    ADCS_Capture_Result_Startup,
    ADCS_Capture_Result_Pending,
    ADCS_Capture_Result_Success,
    ADCS_Capture_Result_Success_Shift,
    ADCS_Capture_Result_Timeout,
    ADCS_Capture_Result_SRAM_Error
} ADCS_Capture_Result;

typedef enum ADCS_Detect_Result {
    ADCS_Detect_Result_Startup,
    ADCS_Detect_Result_No_Detect,
    ADCS_Detect_Result_Pending,
    ADCS_Detect_Result_Too_Many_Edges,
    ADCS_Detect_Result_Too_Few_Edges,
    ADCS_Detect_Result_Bad_Fit,
    ADCS_Detect_Result_Sun_Not_Found,
    ADCS_Detect_Result_Success
} ADCS_Detect_Result;

typedef enum ADCS_Which_Cam_Sensor {
    ADCS_Cam_None,
    ADCS_Cam1_Sensor,
    ADCS_Cam2_Sensor
} ADCS_Which_Cam_Sensor;

typedef enum ADCS_GPS_Solution_Status {
    ADCS_GPS_Solution_Status_Solution_Computed,
    ADCS_GPS_Solution_Status_Insufficient_Observations,
    ADCS_GPS_Solution_Status_No_Convergence,
    ADCS_GPS_Solution_Status_Singularity_At_Parameters_Matrix,
    ADCS_GPS_Solution_Status_Covariance_Trace_Exceeds_Maximum,
    ADCS_GPS_Solution_Status_Not_Yet_Converged_From_Cold_Start,
    ADCS_GPS_Solution_Status_Height_Or_Velocity_Limits_Exceeded,
    ADCS_GPS_Solution_Status_Variance_Exceeds_Limits,
    ADCS_GPS_Solution_Status_Large_Residuals,
    ADCS_GPS_Solution_Status_Calculating_Comparison_To_User_Provided,
    ADCS_GPS_Solution_Status_Fixed_Position_Invalid, 
    ADCS_GPS_Solution_Status_Position_Type_Unauthorized 
} ADCS_GPS_Solution_Status;

typedef enum ADCS_GPS_Axis {
    ADCS_GPS_X,
    ADCS_GPS_Y,
    ADCS_GPS_Z
} ADCS_GPS_Axis;

/* Structs */

typedef struct ADCS_TC_Ack_Struct {
	uint8_t last_id;
	bool processed;
	enum TC_Error_Flag error_flag;
	uint8_t error_index;
} ADCS_TC_Ack_Struct;

typedef struct ADCS_ID_Struct {
	uint8_t node_type;
	uint8_t interface_version;
	uint8_t major_firmware_version;
	uint8_t minor_firmware_version;
	uint16_t seconds_since_startup;
	uint16_t ms_past_second;
} ADCS_ID_Struct;

typedef struct ADCS_Boot_Running_Status_Struct {
    ADCS_Reset_Cause reset_cause;
    ADCS_Boot_Cause boot_cause;
    uint16_t boot_counter;
    ADCS_Running_Program boot_program_index;
    uint8_t major_firmware_version;
    uint8_t minor_firmware_version;
} ADCS_Boot_Running_Status_Struct;

typedef struct ADCS_Comms_Status_Struct {
    uint16_t tc_counter;
    uint16_t tlm_counter;
    bool tc_buffer_overrun;
    bool i2c_tlm_error;
    bool i2c_tc_error;
} ADCS_Comms_Status_Struct;

typedef struct ADCS_Angular_Rates_Struct {
    double x_rate;
    double y_rate;
    double z_rate;
} ADCS_Angular_Rates_Struct;

typedef struct ADCS_LLH_Position_Struct {
    double latitude;
    double longitude;
    double altitude;
} ADCS_LLH_Position_Struct;

typedef struct ADCS_Power_Control_Struct{
	ADCS_Power_Select cube_control_signal;
	ADCS_Power_Select cube_control_motor;
	ADCS_Power_Select cube_sense1;
	ADCS_Power_Select cube_sense2;
	ADCS_Power_Select cube_star_power;
	ADCS_Power_Select cube_wheel1_power;
	ADCS_Power_Select cube_wheel2_power;
	ADCS_Power_Select cube_wheel3_power;
	ADCS_Power_Select motor_power;
	ADCS_Power_Select gps_power;
} ADCS_Power_Control_Struct;

typedef struct ADCS_Set_Unix_Time_Save_Mode_Struct{
	bool save_now;
	bool save_on_update;
	bool save_periodic;
	uint8_t period;
} ADCS_Set_Unix_Time_Save_Mode_Struct;

typedef struct ADCS_Orbit_Params_Struct {
    double inclination;
    double eccentricity;
    double ascending_node_right_ascension;
    double perigee_argument;
    double b_star_drag_term;
    double mean_motion;
    double mean_anomaly;
    double epoch;
} ADCS_Orbit_Params_Struct;

typedef struct ADCS_Rated_Sensor_Rates_Struct {
	double x; 
	double y;
	double z; 
} ADCS_Rated_Sensor_Rates_Struct;

typedef struct ADCS_Wheel_Speed_Struct {
	int16_t x; 
	int16_t y;
	int16_t z; 
} ADCS_Wheel_Speed_Struct;

typedef struct ADCS_Magnetorquer_Command_Struct {
	double x; 
	double y;
	double z; 
} ADCS_Magnetorquer_Command_Struct;

typedef struct ADCS_Raw_Mag_TLM_Struct {
	int16_t x; 
	int16_t y;
	int16_t z; 
} ADCS_Raw_Mag_TLM_Struct;

typedef struct ADCS_Fine_Angular_Rates_Struct {
	double x; 
	double y;
	double z; 
} ADCS_Fine_Angular_Rates_Struct;

typedef struct ADCS_Magnetometer_Config_Struct {
    double mounting_transform_alpha_angle;
    double mounting_transform_beta_angle;
    double mounting_transform_gamma_angle;
    double channel_1_offset;
    double channel_2_offset;
    double channel_3_offset;
    double sensitivity_matrix_s11;
    double sensitivity_matrix_s22;
    double sensitivity_matrix_s33;
    double sensitivity_matrix_s12;
    double sensitivity_matrix_s13;
    double sensitivity_matrix_s21;
    double sensitivity_matrix_s23;
    double sensitivity_matrix_s31;
    double sensitivity_matrix_s32;
} ADCS_Magnetometer_Config_Struct;

typedef struct ADCS_Commanded_Angles_Struct {
	double x; 
	double y;
	double z; 
} ADCS_Commanded_Angles_Struct;

typedef struct ADCS_Estimation_Params_Struct {
    float magnetometer_rate_filter_system_noise;
    float ekf_system_noise;
    float css_measurement_noise;
    float sun_sensor_measurement_noise;
    float nadir_sensor_measurement_noise;
    float magnetometer_measurement_noise;
    float star_tracker_measurement_noise;
    bool use_sun_sensor;
    bool use_nadir_sensor;
    bool use_css;
    bool use_star_tracker;
    bool nadir_sensor_terminator_test;
    bool automatic_magnetometer_recovery;
    ADCS_Magnetometer_Mode magnetometer_mode;
    ADCS_Magnetometer_Mode magnetometer_selection_for_raw_mtm_tlm;
    bool automatic_estimation_transition_due_to_rate_sensor_errors;
    bool wheel_30s_power_up_delay;
    uint8_t cam1_and_cam2_sampling_period;
} ADCS_Estimation_Params_Struct;

typedef struct ADCS_ASGP4_Params_Struct {
    double incl_coefficient;
    double raan_coefficient;
    double ecc_coefficient;
    double aop_coefficient;
    double time_coefficient;
    double pos_coefficient;
    double maximum_position_error;
    ADCS_ASGP4_Filter asgp4_filter;
    double xp_coefficient;
    double yp_coefficient;
    uint8_t gps_roll_over;
    double position_sd;
    double velocity_sd;
    uint8_t min_satellites;
    double time_gain;
    double max_lag;
    uint16_t min_samples;
} ADCS_ASGP4_Params_Struct;

typedef struct ADCS_Tracking_Controller_Target_Struct {
    float lon;
    float lat;
    float alt;
} ADCS_Tracking_Controller_Target_Struct;

typedef struct ADCS_Rate_Gyro_Config_Struct {
    ADCS_Axis_Select gyro1; 
    ADCS_Axis_Select gyro2; 
    ADCS_Axis_Select gyro3; 
    double x_rate_offset; 
    double y_rate_offset; 
    double z_rate_offset; 
    uint8_t rate_sensor_mult;
} ADCS_Rate_Gyro_Config_Struct;

typedef struct ADCS_Estimated_Attitude_Angles_Struct {
    double estimated_roll_angle;
    double estimated_pitch_angle;
    double estimated_yaw_angle;
} ADCS_Estimated_Attitude_Angles_Struct;

typedef struct ADCS_Magnetic_Field_Vector_Struct {
    double x;
    double y;
    double z;
} ADCS_Magnetic_Field_Vector_Struct;

typedef struct ADCS_Fine_Sun_Vector_Struct {
    double x;
    double y;
    double z;
} ADCS_Fine_Sun_Vector_Struct;

typedef struct ADCS_Nadir_Vector_Struct {
    double x;
    double y;
    double z;
} ADCS_Nadir_Vector_Struct;

typedef struct ADCS_Quaternion_Error_Vector_Struct {
    double quaternion_error_q1;
    double quaternion_error_q2;
    double quaternion_error_q3;
} ADCS_Quaternion_Error_Vector_Struct;

typedef struct ADCS_Estimated_Gyro_Bias_Struct {
    double estimated_x_gyro_bias;
    double estimated_y_gyro_bias;
    double estimated_z_gyro_bias;
} ADCS_Estimated_Gyro_Bias_Struct;

typedef struct ADCS_Estimation_Innovation_Vector_Struct {
    double innovation_vector_x;
    double innovation_vector_y;
    double innovation_vector_z;
} ADCS_Estimation_Innovation_Vector_Struct;

typedef struct ADCS_Raw_Cam_Sensor_Struct {
    ADCS_Which_Cam_Sensor which_sensor;
    int16_t cam_centroid_x;
    int16_t cam_centroid_y;
    ADCS_Capture_Result cam_capture_status;
    ADCS_Detect_Result cam_detection_result;
} ADCS_Raw_Cam_Sensor_Struct;

typedef struct ADCS_Raw_CSS_1_to_6_Struct {
    uint8_t css1;
    uint8_t css2;
    uint8_t css3;
    uint8_t css4;
    uint8_t css5;
    uint8_t css6;
} ADCS_Raw_CSS_1_to_6_Struct;

typedef struct ADCS_Raw_CSS_7_to_10_Struct {
    uint8_t css7;
    uint8_t css8;
    uint8_t css9;
    uint8_t css10;
} ADCS_Raw_CSS_7_to_10_Struct;

typedef struct ADCS_CubeControl_Current_Struct {
    double cubecontrol_3v3_current;
    double cubecontrol_5v_current;
    double cubecontrol_vbat_current;
} ADCS_CubeControl_Current_Struct;

typedef struct ADCS_Raw_GPS_Status_Struct {
    ADCS_GPS_Solution_Status gps_solution_status;
    uint8_t num_tracked_satellites;
    uint8_t num_used_satellites;
    uint8_t counter_xyz_log;
    uint8_t counter_range_log;
    uint8_t response_message_gps_log;
} ADCS_Raw_GPS_Status_Struct;

typedef struct {
    uint16_t gps_reference_week;
    double gps_time; // in seconds
} ADCS_Raw_GPS_Time_Struct;

typedef struct ADCS_Raw_GPS_Struct {
    ADCS_GPS_Axis axis;
    int32_t ecef_position;    
    int16_t ecef_velocity;   
} ADCS_Raw_GPS_Struct;

typedef struct ADCS_Measurements_Struct {
    double magnetic_field_x;
    double magnetic_field_y;
    double magnetic_field_z;
    double coarse_sun_x;
    double coarse_sun_y;
    double coarse_sun_z;
    double sun_x;
    double sun_y;
    double sun_z;
    double nadir_x;
    double nadir_y;
    double nadir_z;
    double x_angular_rate;
    double y_angular_rate;
    double z_angular_rate;
    double x_wheel_speed;
    double y_wheel_speed;
    double z_wheel_speed;
    double star1_body_x;
    double star1_body_y;
    double star1_body_z;
    double star1_orbit_x;
    double star1_orbit_y;
    double star1_orbit_z;
    double star2_body_x;
    double star2_body_y;
    double star2_body_z;
    double star2_orbit_x;
    double star2_orbit_y;
    double star2_orbit_z;
    double star3_body_x;
    double star3_body_y;
    double star3_body_z;
    double star3_orbit_x;
    double star3_orbit_y;
    double star3_orbit_z;
} ADCS_Measurements_Struct;

#endif /* INC_ADCS_TYPES_H_ */