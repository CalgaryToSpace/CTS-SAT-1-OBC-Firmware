/*
 * adcs_types.h
 *
 *  Created on: Oct 28, 2023
 *      Authors: Saksham Puri, Zachary Uy, Nadeem Moosa
 */

#ifndef INC_ADCS_TYPES_H_
#define INC_ADCS_TYPES_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32l4xx_hal.h"

// Bit 7 of Telecommand and Telemetry Request - Section 4.1 of Firmware Manual
static const uint8_t ADCS_TELECOMMAND = 0;
static const uint8_t ADCS_TELEMETRY_REQUEST = 1;

// ADCS Magic Number defined in Section 5.1 Table 10 of Firmware Manual
static const uint8_t ADCS_MAGIC_NUMBER = 0x5A; // Used to make sure that the reset command is valid

// I2C Write and Read Addresses - Section 5.3 Table 5 of Firmware Manual
static const uint8_t ADCS_I2C_WRITE_ADDRESS = 0xAE;
static const uint8_t ADCS_I2C_READ_ADDRESS = 0xAF;
static const uint8_t ADCS_I2C_ADDRESS = 0x57;

// Identifiers of UART Start of Message and End of Message - Section 4.2 of Firmware Manual
// 0x1F | 0x7F ....... | 0x1F 0xFF
static const uint8_t ADCS_UART_ESCAPE_BYTE = 0x1F;
static const uint8_t ADCS_UART_START_MESSAGE = 0x7F;
static const uint8_t ADCS_UART_END_MESSAGE = 0xFF;

// CRC defines
static const uint8_t ADCS_CRC_POLY = 0x91;

// define for timeout
static const uint16_t ADCS_HAL_TIMEOUT = 1000;

// epsilon for comparing doubles
static const uint8_t ADCS_TEST_EPSILON = 1e-6;

/* Enumerated Values */

// Telecommand Error Flags - Section 5.2.2 Figure 6 of Firmware Manual
typedef enum ADCS_Error_Flag {
	ADCS_ERROR_FLAG_NONE,
	ADCS_ERROR_FLAG_INVALID_ID,
	ADCS_ERROR_FLAG_WRONG_LENGTH,
	ADCS_ERROR_FLAG_INVALID_PARAMS,
	ADCS_ERROR_FLAG_CRC
} ADCS_Error_Flag;

// ADCS Run Modes - Section 5.3.1 Table 75 of Firmware Manual
typedef enum ADCS_Run_Mode {
	ADCS_RUN_MODE_OFF,
	ADCS_RUN_MODE_ENABLED,
	ADCS_RUN_MODE_TRIGGERED,
	ADCS_RUN_MODE_SIMULATION
} ADCS_Run_Mode;

// ADCS Reset Causes - Section 6.1.2 Table 28 of Firmware Manual
typedef enum ADCS_Reset_Cause {
    ADCS_POWER_ON_RESET,
    ADCS_BROWN_OUT_DETECTED_ON_REGULATED_POWER,
    ADCS_BROWN_OUT_DETECTED_ON_UNREGULATED_POWER,
    ADCS_EXTERNAL_WATCHDOG_RESET,
    ADCS_EXTERNAL_RESET,
    ADCS_WATCHDOG_RESET,
    ADCS_LOCKUP_SYSTEM_RESET,
    ADCS_LOCKUP_RESET,
    ADCS_SYSTEM_REQUEST_RESET,
    ADCS_BACKUP_BROWN_OUT,
    ADCS_BACKUP_MODE_RESET,
    ADCS_BACKUP_MODE_RESET_AND_BACKUP_BROWN_OUT_VDD_REGULATED,
    ADCS_BACKUP_MODE_RESET_AND_BACKUP_BROWN_OUT_VDD_REGULATED_AND_BROWN_OUT_REGULATED,
    ADCS_BACKUP_MODE_RESET_AND_WATCHDOG_RESET,
    ADCS_BACKUP_BROWN_OUT_BUVIN_AND_SYSTEM_REQUEST_RESET,
    ADCS_UNKNOWN_RESET_CAUSE
} ADCS_Reset_Cause;

// ADCS Boot Causes - Section 6.1.2 Table 29 of Firmware Manual
typedef enum ADCS_Boot_Cause {
    ADCS_BOOT_CAUSE_UNEXPECTED,
    ADCS_BOOT_CAUSE_NOT_USED_1,
    ADCS_BOOT_CAUSE_COMMUNICATIONS_TIMEOUT,
    ADCS_BOOT_CAUSE_COMMANDED,
    ADCS_BOOT_CAUSE_NOT_USED_2,
    ADCS_BOOT_CAUSE_SRAM_LATCHUP
} ADCS_Boot_Cause;

// ADCS Running Programs - Section 6.1.2 Table 30 of Firmware Manual
typedef enum ADCS_Running_Program {
    ADCS_RUNNING_ADCS = 1,
    ADCS_RUNNING_BOOTLOADER = 2
} ADCS_Running_Program;

// ADCS Control Modes - Section 6.3.1 Table 77 of Firmware Manual
typedef enum ADCS_Control_Mode {
    ADCS_CONTROL_MODE_NONE,
    ADCS_CONTROL_MODE_DETUMBLING,
    ADCS_CONTROL_MODE_Y_THOMSON_SPIN,
    ADCS_CONTROL_MODE_Y_WHEEL_MOMENTUM_STABILIZED_INITIAL_PITCH_ACQUISITION,
    ADCS_CONTROL_MODE_Y_WHEEL_MOMENTUM_STABILIZED_STEADY_STATE,
    ADCS_CONTROL_MODE_XYZ_WHEEL,
    ADCS_CONTROL_MODE_RWHEEL_SUN_TRACKING,
    ADCS_CONTROL_MODE_RWHEEL_TARGET_TRACKING,
    ADCS_CONTROL_MODE_VERY_FAST_SPIN_DETUMBLING,
    ADCS_CONTROL_MODE_FAST_SPIN_DETUMBLING,
    ADCS_CONTROL_MODE_USER_SPECIFIC_1,
    ADCS_CONTROL_MODE_USER_SPECIFIC_2,
    ADCS_CONTROL_MODE_STOP_R_WHEELS,
    ADCS_CONTROL_MODE_USER_CODED,
    ADCS_CONTROL_MODE_SUN_TRACKING_YAW_OR_ROLL_ONLY_WHEEL,
    ADCS_CONTROL_MODE_TARGET_TRACKING_YAW_ONLY_WHEEL
} ADCS_Control_Mode;

typedef enum ADCS_Estimation_Mode {
    ADCS_ESTIMATION_MODE_NONE,
    ADCS_ESTIMATION_MODE_MEMS_RATE_SENSING,
    ADCS_ESTIMATION_MODE_MAGNETOMETER_RATE_FILTER,
    ADCS_ESTIMATION_MODE_MAGNETOMETER_RATE_FILTER_WITH_PITCH_ESTIMATION,
    ADCS_ESTIMATION_MODE_MAGNETOMETER_AND_FINE_SUN_TRIAD_ALGORITHM,
    ADCS_ESTIMATION_MODE_FULL_STATE_EKF,
    ADCS_ESTIMATION_MODE_MEMS_GYRO_EKF,
    ADCS_ESTIMATION_MODE_USER_CODED_ESTIMATION_MODE
} ADCS_Estimation_Mode;

typedef enum ADCS_Power_Select {
	ADCS_POWER_SELECT_OFF = 0,
	ADCS_POWER_SELECT_ON = 1,
	ADCS_POWER_SELECT_SAME = 2
} ADCS_Power_Select;

typedef enum ADCS_Magnetometer_Mode {
	ADCS_MAGNETOMETER_MODE_MAIN_SIGNAL,
	ADCS_MAGNETOMETER_MODE_REDUNDANT_SIGNAL,
	ADCS_MAGNETOMETER_MODE_MAIN_MOTOR,
	ADCS_MAGNETOMETER_MODE_NONE
} ADCS_Magnetometer_Mode;

typedef enum ADCS_ASGP4_Filter {
    ADCS_ASGP4_FILTER_LOWPASS,
    ADCS_ASGP4_FILTER_AVERAGE
} ADCS_ASGP4_Filter;

typedef enum ADCS_Axis_Select {
    ADCS_POSITIVE_X,    
    ADCS_NEGATIVE_X,    
    ADCS_POSITIVE_Y,
    ADCS_NEGATIVE_Y,
    ADCS_POSITIVE_Z,
    ADCS_NEGATIVE_Z,
    ADCS_NOT_USED
} ADCS_Axis_Select;

typedef enum ADCS_Capture_Result {
    ADCS_CAPTURE_RESULT_STARTUP,
    ADCS_CAPTURE_RESULT_PENDING,
    ADCS_CAPTURE_RESULT_SUCCESS,
    ADCS_CAPTURE_RESULT_SUCCESS_SHIFT,
    ADCS_CAPTURE_RESULT_TIMEOUT,
    ADCS_CAPTURE_RESULT_SRAM_ERROR
} ADCS_Capture_Result;

typedef enum ADCS_Detect_Result {
    ADCS_DETECT_RESULT_STARTUP,
    ADCS_DETECT_RESULT_NO_DETECT,
    ADCS_DETECT_RESULT_PENDING,
    ADCS_DETECT_RESULT_TOO_MANY_EDGES,
    ADCS_DETECT_RESULT_TOO_FEW_EDGES,
    ADCS_DETECT_RESULT_BAD_FIT,
    ADCS_DETECT_RESULT_SUN_NOT_FOUND,
    ADCS_DETECT_RESULT_SUCCESS
} ADCS_Detect_Result;

typedef enum ADCS_Which_Cam_Sensor {
    ADCS_CAM_NONE,
    ADCS_CAM1_SENSOR,
    ADCS_CAM2_SENSOR
} ADCS_Which_Cam_Sensor;

typedef enum ADCS_GPS_Solution_Status {
    ADCS_GPS_SOLUTION_STATUS_SOLUTION_COMPUTED,
    ADCS_GPS_SOLUTION_STATUS_INSUFFICIENT_OBSERVATIONS,
    ADCS_GPS_SOLUTION_STATUS_NO_CONVERGENCE,
    ADCS_GPS_SOLUTION_STATUS_SINGULARITY_AT_PARAMETERS_MATRIX,
    ADCS_GPS_SOLUTION_STATUS_COVARIANCE_TRACE_EXCEEDS_MAXIMUM,
    ADCS_GPS_SOLUTION_STATUS_NOT_YET_CONVERGED_FROM_COLD_START,
    ADCS_GPS_SOLUTION_STATUS_HEIGHT_OR_VELOCITY_LIMITS_EXCEEDED,
    ADCS_GPS_SOLUTION_STATUS_VARIANCE_EXCEEDS_LIMITS,
    ADCS_GPS_SOLUTION_STATUS_LARGE_RESIDUALS,
    ADCS_GPS_SOLUTION_STATUS_CALCULATING_COMPARISON_TO_USER_PROVIDED,
    ADCS_GPS_SOLUTION_STATUS_FIXED_POSITION_INVALID, 
    ADCS_GPS_SOLUTION_STATUS_POSITION_TYPE_UNAUTHORIZED 
} ADCS_GPS_Solution_Status;

typedef enum ADCS_GPS_Axis {
    ADCS_GPS_X,
    ADCS_GPS_Y,
    ADCS_GPS_Z
} ADCS_GPS_Axis;

/* Structs */

typedef struct ADCS_CMD_Ack_Struct {
	uint8_t last_id;
	bool processed;
	enum ADCS_Error_Flag error_flag;
	uint8_t error_index;
} ADCS_CMD_Ack_Struct;

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
    uint16_t cmd_counter;
    uint16_t tlm_counter;
    bool cmd_buffer_overrun;
    bool i2c_tlm_error;
    bool i2c_cmd_error;
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