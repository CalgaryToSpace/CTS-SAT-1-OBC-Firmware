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

/* Enumerated Values */

// Telecommand Error Flags - Section 5.2.2 Figure 6 of Firmware Manual
typedef enum ADCS_Error_Flag {
	ADCS_ERROR_FLAG_NONE = 0,
	ADCS_ERROR_FLAG_INVALID_ID = 1,
	ADCS_ERROR_FLAG_WRONG_LENGTH = 2,
	ADCS_ERROR_FLAG_INVALID_PARAMS = 3,
	ADCS_ERROR_FLAG_CRC = 4
} ADCS_Error_Flag;

// ADCS Run Modes - Section 5.3.1 Table 75 of Firmware Manual
typedef enum ADCS_Run_Mode {
	ADCS_RUN_MODE_OFF = 0,
	ADCS_RUN_MODE_ENABLED = 1,
	ADCS_RUN_MODE_TRIGGERED = 2,
	ADCS_RUN_MODE_SIMULATION = 3
} ADCS_Run_Mode;

// ADCS Reset Causes - Section 6.1.2 Table 28 of Firmware Manual
typedef enum ADCS_Reset_Cause {
    ADCS_RESET_CAUSE_POWER_ON = 0,
    ADCS_RESET_CAUSE_BROWN_OUT_DETECTED_ON_REGULATED_POWER = 1,
    ADCS_RESET_CAUSE_BROWN_OUT_DETECTED_ON_UNREGULATED_POWER = 2,
    ADCS_RESET_CAUSE_EXTERNAL_WATCHDOG = 3,
    ADCS_RESET_CAUSE_EXTERNAL = 4,
    ADCS_RESET_CAUSE_WATCHDOG = 5,
    ADCS_RESET_CAUSE_LOCKUP_SYSTEM = 6,
    ADCS_RESET_CAUSE_LOCKUP = 7,
    ADCS_RESET_CAUSE_SYSTEM_REQUEST = 8,
    ADCS_RESET_CAUSE_BACKUP_BROWN_OUT = 9,
    ADCS_RESET_CAUSE_BACKUP_MODE = 10,
    ADCS_RESET_CAUSE_BACKUP_MODE_AND_BACKUP_BROWN_OUT_VDD_REGULATED = 11,
    ADCS_RESET_CAUSE_BACKUP_MODE_AND_BACKUP_BROWN_OUT_VDD_REGULATED_AND_BROWN_OUT_REGULATED = 12,
    ADCS_RESET_CAUSE_BACKUP_MODE_AND_WATCHDOG = 13,
    ADCS_RESET_CAUSE_BACKUP_BROWN_OUT_BUVIN_AND_SYSTEM_REQUEST = 14,
    ADCS_RESET_CAUSE_UNKNOWN = 15
} ADCS_Reset_Cause;

// ADCS Boot Causes - Section 6.1.2 Table 29 of Firmware Manual
typedef enum ADCS_Boot_Cause {
    ADCS_BOOT_CAUSE_UNEXPECTED = 0,
    ADCS_BOOT_CAUSE_NOT_USED_1 = 1,
    ADCS_BOOT_CAUSE_COMMUNICATIONS_TIMEOUT = 2,
    ADCS_BOOT_CAUSE_COMMANDED = 3,
    ADCS_BOOT_CAUSE_NOT_USED_2 = 4,
    ADCS_BOOT_CAUSE_SRAM_LATCHUP = 5
} ADCS_Boot_Cause;

// ADCS Running Programs - Section 6.1.2 Table 30 of Firmware Manual
typedef enum ADCS_Running_Program {
    ADCS_RUNNING_PROGRAM_ADCS = 1,
    ADCS_RUNNING_PROGRAM_BOOTLOADER = 2
} ADCS_Running_Program;

// ADCS Control Modes - Section 6.3.1 Table 77 of Firmware Manual
typedef enum ADCS_Control_Mode {
    ADCS_CONTROL_MODE_NONE = 0,
    ADCS_CONTROL_MODE_DETUMBLING = 1,
    ADCS_CONTROL_MODE_Y_THOMSON_SPIN = 2,
    ADCS_CONTROL_MODE_Y_WHEEL_MOMENTUM_STABILIZED_INITIAL_PITCH_ACQUISITION = 3,
    ADCS_CONTROL_MODE_Y_WHEEL_MOMENTUM_STABILIZED_STEADY_STATE = 4,
    ADCS_CONTROL_MODE_XYZ_WHEEL = 5,
    ADCS_CONTROL_MODE_RWHEEL_SUN_TRACKING = 6,
    ADCS_CONTROL_MODE_RWHEEL_TARGET_TRACKING = 7,
    ADCS_CONTROL_MODE_VERY_FAST_SPIN_DETUMBLING = 8,
    ADCS_CONTROL_MODE_FAST_SPIN_DETUMBLING = 9,
    ADCS_CONTROL_MODE_USER_SPECIFIC_1 = 10,
    ADCS_CONTROL_MODE_USER_SPECIFIC_2 = 11,
    ADCS_CONTROL_MODE_STOP_R_WHEELS = 12,
    ADCS_CONTROL_MODE_USER_CODED = 13,
    ADCS_CONTROL_MODE_SUN_TRACKING_YAW_OR_ROLL_ONLY_WHEEL = 14,
    ADCS_CONTROL_MODE_TARGET_TRACKING_YAW_ONLY_WHEEL = 15
} ADCS_Control_Mode;

typedef enum ADCS_Estimation_Mode {
    ADCS_ESTIMATION_MODE_NONE = 0,
    ADCS_ESTIMATION_MODE_MEMS_RATE_SENSING = 1,
    ADCS_ESTIMATION_MODE_MAGNETOMETER_RATE_FILTER = 2,
    ADCS_ESTIMATION_MODE_MAGNETOMETER_RATE_FILTER_WITH_PITCH_ESTIMATION = 3,
    ADCS_ESTIMATION_MODE_MAGNETOMETER_AND_FINE_SUN_TRIAD_ALGORITHM = 4,
    ADCS_ESTIMATION_MODE_FULL_STATE_EKF = 5,
    ADCS_ESTIMATION_MODE_MEMS_GYRO_EKF = 6,
    ADCS_ESTIMATION_MODE_USER_CODED_ESTIMATION_MODE = 7
} ADCS_Estimation_Mode;

typedef enum ADCS_Power_Select {
	ADCS_POWER_SELECT_OFF = 0,
	ADCS_POWER_SELECT_ON = 1,
	ADCS_POWER_SELECT_SAME = 2
} ADCS_Power_Select;

typedef enum ADCS_Magnetometer_Mode {
	ADCS_MAGNETOMETER_MODE_MAIN_SIGNAL = 0,
	ADCS_MAGNETOMETER_MODE_REDUNDANT_SIGNAL = 1,
	ADCS_MAGNETOMETER_MODE_MAIN_MOTOR = 2,
	ADCS_MAGNETOMETER_MODE_NONE = 3
} ADCS_Magnetometer_Mode;

typedef enum ADCS_ASGP4_Filter {
    ADCS_ASGP4_FILTER_LOWPASS = 0,
    ADCS_ASGP4_FILTER_AVERAGE = 1
} ADCS_ASGP4_Filter;

typedef enum ADCS_Axis_Select {
    ADCS_AXIS_SELECT_POSITIVE_X = 0,
    ADCS_AXIS_SELECT_NEGATIVE_X = 1,
    ADCS_AXIS_SELECT_POSITIVE_Y = 2,
    ADCS_AXIS_SELECT_NEGATIVE_Y = 3,
    ADCS_AXIS_SELECT_POSITIVE_Z = 4,
    ADCS_AXIS_SELECT_NEGATIVE_Z = 5,
    ADCS_AXIS_SELECT_NOT_USED = 6
} ADCS_Axis_Select;

typedef enum ADCS_Capture_Result {
    ADCS_CAPTURE_RESULT_STARTUP = 0,
    ADCS_CAPTURE_RESULT_PENDING = 1,
    ADCS_CAPTURE_RESULT_SUCCESS = 2,
    ADCS_CAPTURE_RESULT_SUCCESS_SHIFT = 3,
    ADCS_CAPTURE_RESULT_TIMEOUT = 4,
    ADCS_CAPTURE_RESULT_SRAM_ERROR = 5
} ADCS_Capture_Result;

typedef enum ADCS_Detect_Result {
    ADCS_DETECT_RESULT_STARTUP = 0,
    ADCS_DETECT_RESULT_NO_DETECT = 1,
    ADCS_DETECT_RESULT_PENDING = 2,
    ADCS_DETECT_RESULT_TOO_MANY_EDGES = 3,
    ADCS_DETECT_RESULT_TOO_FEW_EDGES = 4,
    ADCS_DETECT_RESULT_BAD_FIT = 5,
    ADCS_DETECT_RESULT_SUN_NOT_FOUND = 6,
    ADCS_DETECT_RESULT_SUCCESS = 7
} ADCS_Detect_Result;

typedef enum ADCS_Which_Cam_Sensor {
    ADCS_WHICH_CAM_SENSOR_NONE = 0,
    ADCS_WHICH_CAM_SENSOR_CAM1 = 1,
    ADCS_WHICH_CAM_SENSOR_CAM2 = 2
} ADCS_Which_Cam_Sensor;

typedef enum ADCS_GPS_Solution_Status {
    ADCS_GPS_SOLUTION_STATUS_SOLUTION_COMPUTED = 0,
    ADCS_GPS_SOLUTION_STATUS_INSUFFICIENT_OBSERVATIONS = 1,
    ADCS_GPS_SOLUTION_STATUS_NO_CONVERGENCE = 2,
    ADCS_GPS_SOLUTION_STATUS_SINGULARITY_AT_PARAMETERS_MATRIX = 3,
    ADCS_GPS_SOLUTION_STATUS_COVARIANCE_TRACE_EXCEEDS_MAXIMUM = 4,
    ADCS_GPS_SOLUTION_STATUS_NOT_YET_CONVERGED_FROM_COLD_START = 5,
    ADCS_GPS_SOLUTION_STATUS_HEIGHT_OR_VELOCITY_LIMITS_EXCEEDED = 6,
    ADCS_GPS_SOLUTION_STATUS_VARIANCE_EXCEEDS_LIMITS = 7,
    ADCS_GPS_SOLUTION_STATUS_LARGE_RESIDUALS = 8,
    ADCS_GPS_SOLUTION_STATUS_CALCULATING_COMPARISON_TO_USER_PROVIDED = 9,
    ADCS_GPS_SOLUTION_STATUS_FIXED_POSITION_INVALID = 10,
    ADCS_GPS_SOLUTION_STATUS_POSITION_TYPE_UNAUTHORIZED = 11 
} ADCS_GPS_Solution_Status;

typedef enum ADCS_GPS_Axis {
    ADCS_GPS_AXIS_X = 0,
    ADCS_GPS_AXIS_Y = 1,
    ADCS_GPS_AXIS_Z = 2
} ADCS_GPS_Axis;

/* Structs */

typedef struct ADCS_CMD_Ack_Struct {
	uint8_t last_id;
	bool processed:1; // 1-bit bool
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
    bool cmd_buffer_overrun:1; // 1-bit bool
    bool i2c_tlm_error:1; // 1-bit bool
    bool i2c_cmd_error:1; // 1-bit bool
} ADCS_Comms_Status_Struct;

typedef struct ADCS_Angular_Rates_Struct {
    int32_t x_rate_milli_deg_per_sec;
    int32_t y_rate_milli_deg_per_sec;
    int32_t z_rate_milli_deg_per_sec;
} ADCS_Angular_Rates_Struct;

typedef struct ADCS_LLH_Position_Struct {
    int32_t latitude_milli_deg;
    int32_t longitude_milli_deg;
    int32_t altitude_meters;
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
	bool save_now:1; // 1-bit bool
	bool save_on_update:1; // 1-bit bool
	bool save_periodic:1; // 1-bit bool
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
	int32_t x_milli_deg_per_sec; 
	int32_t y_milli_deg_per_sec;
	int32_t z_milli_deg_per_sec; 
} ADCS_Rated_Sensor_Rates_Struct;

typedef struct ADCS_Wheel_Speed_Struct {
    // TODO: Add bool for whether it's commanded or actual wheel speed
	int16_t x; 
	int16_t y;
	int16_t z; 
} ADCS_Wheel_Speed_Struct;

typedef struct ADCS_Magnetorquer_Command_Struct {
	int32_t x_ms; 
	int32_t y_ms;
	int32_t z_ms; 
} ADCS_Magnetorquer_Command_Struct;

typedef struct ADCS_Raw_Magnetometer_Values_Struct {
	int16_t x; 
	int16_t y;
	int16_t z; 
} ADCS_Raw_Magnetometer_Values_Struct;

typedef struct ADCS_Fine_Angular_Rates_Struct {
	int16_t x_milli_deg_per_sec; 
	int16_t y_milli_deg_per_sec;
	int16_t z_milli_deg_per_sec; 
} ADCS_Fine_Angular_Rates_Struct;

typedef struct ADCS_Magnetometer_Config_Struct {
    int32_t mounting_transform_alpha_angle_milli_deg_per_sec;
    int32_t mounting_transform_beta_angle_milli_deg_per_sec;
    int32_t mounting_transform_gamma_angle_milli_deg_per_sec;
    int16_t channel_1_offset_milli_deg_per_sec;
    int16_t channel_2_offset_milli_deg_per_sec;
    int16_t channel_3_offset_milli_deg_per_sec;
    int16_t sensitivity_matrix_s11_milli_deg_per_sec;
    int16_t sensitivity_matrix_s22_milli_deg_per_sec;
    int16_t sensitivity_matrix_s33_milli_deg_per_sec;
    int16_t sensitivity_matrix_s12_milli_deg_per_sec;
    int16_t sensitivity_matrix_s13_milli_deg_per_sec;
    int16_t sensitivity_matrix_s21_milli_deg_per_sec;
    int16_t sensitivity_matrix_s23_milli_deg_per_sec;
    int16_t sensitivity_matrix_s31_milli_deg_per_sec;
    int16_t sensitivity_matrix_s32_milli_deg_per_sec;
} ADCS_Magnetometer_Config_Struct;

typedef struct ADCS_Commanded_Angles_Struct {
	int32_t x_milli_deg; 
	int32_t y_milli_deg;
	int32_t z_milli_deg; 
} ADCS_Commanded_Angles_Struct;

typedef struct ADCS_Estimation_Params_Struct {
    float magnetometer_rate_filter_system_noise;
    float ekf_system_noise;
    float css_measurement_noise;
    float sun_sensor_measurement_noise;
    float nadir_sensor_measurement_noise;
    float magnetometer_measurement_noise;
    float star_tracker_measurement_noise;
    bool use_sun_sensor:1; // 1-bit bool
    bool use_nadir_sensor:1; // 1-bit bool
    bool use_css:1; // 1-bit bool
    bool use_star_tracker:1; // 1-bit bool
    bool nadir_sensor_terminator_test:1; // 1-bit bool
    bool automatic_magnetometer_recovery:1; // 1-bit bool
    ADCS_Magnetometer_Mode magnetometer_mode;
    ADCS_Magnetometer_Mode magnetometer_selection_for_raw_mtm_tlm;
    bool automatic_estimation_transition_due_to_rate_sensor_errors:1; // 1-bit bool
    bool wheel_30s_power_up_delay:1; // 1-bit bool
    uint8_t cam1_and_cam2_sampling_period;
} ADCS_Estimation_Params_Struct;

typedef struct ADCS_ASGP4_Params_Struct {
    int16_t incl_coefficient_milli;
    int16_t raan_coefficient_milli;
    int16_t ecc_coefficient_milli;
    int16_t aop_coefficient_milli;
    int16_t time_coefficient_milli;
    int16_t pos_coefficient_milli;
    int32_t maximum_position_error_milli;
    ADCS_ASGP4_Filter asgp4_filter;
    int64_t xp_coefficient_nano;
    int64_t yp_coefficient_nano;
    uint8_t gps_roll_over;
    int32_t position_sd_milli;
    int16_t velocity_sd_milli;
    uint8_t min_satellites;
    int16_t time_gain_milli;
    int16_t max_lag_milli;
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
    int16_t x_rate_offset_milli_deg_per_sec; 
    int16_t y_rate_offset_milli_deg_per_sec; 
    int16_t z_rate_offset_milli_deg_per_sec; 
    uint8_t rate_sensor_mult;
} ADCS_Rate_Gyro_Config_Struct;

typedef struct ADCS_Estimated_Attitude_Angles_Struct {
    int32_t estimated_roll_angle_milli_deg;
    int32_t estimated_pitch_angle_milli_deg;
    int32_t estimated_yaw_angle_milli_deg;
} ADCS_Estimated_Attitude_Angles_Struct;

typedef struct ADCS_Magnetic_Field_Vector_Struct {
    int32_t x_nT;
    int32_t y_nT;
    int32_t z_nT;
} ADCS_Magnetic_Field_Vector_Struct;

typedef struct ADCS_Fine_Sun_Vector_Struct {
    int32_t x_micro;
    int32_t y_micro;
    int32_t z_micro;
} ADCS_Fine_Sun_Vector_Struct;

typedef struct ADCS_Nadir_Vector_Struct {
    int32_t x_micro;
    int32_t y_micro;
    int32_t z_micro;
} ADCS_Nadir_Vector_Struct;

typedef struct ADCS_Quaternion_Error_Vector_Struct {
    int32_t quaternion_error_q1_micro;
    int32_t quaternion_error_q2_micro;
    int32_t quaternion_error_q3_micro;
} ADCS_Quaternion_Error_Vector_Struct;

typedef struct ADCS_Estimated_Gyro_Bias_Struct {
    int32_t estimated_x_gyro_bias_milli_deg_per_sec;
    int32_t estimated_y_gyro_bias_milli_deg_per_sec;
    int32_t estimated_z_gyro_bias_milli_deg_per_sec;
} ADCS_Estimated_Gyro_Bias_Struct;

typedef struct ADCS_Estimation_Innovation_Vector_Struct {
    int32_t innovation_vector_x_micro;
    int32_t innovation_vector_y_micro;
    int32_t innovation_vector_z_micro;
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
    double cubecontrol_3v3_current_mA;
    double cubecontrol_5v_current_mA;
    double cubecontrol_vbat_current_mA;
} ADCS_CubeControl_Current_Struct;

typedef struct ADCS_Raw_GPS_Status_Struct {
    ADCS_GPS_Solution_Status gps_solution_status;
    uint8_t num_tracked_satellites;
    uint8_t num_used_satellites;
    uint8_t counter_xyz_log;
    uint8_t counter_range_log;
    uint8_t response_message_gps_log;
} ADCS_Raw_GPS_Status_Struct;

typedef struct ADCS_Raw_GPS_Time_Struct {
    uint16_t gps_reference_week;
    double gps_time; // in seconds
} ADCS_Raw_GPS_Time_Struct;

typedef struct ADCS_Raw_GPS_Struct {
    ADCS_GPS_Axis axis;
    int32_t ecef_position_meters;    
    int16_t ecef_velocity_meters_per_sec;   
} ADCS_Raw_GPS_Struct;

typedef struct ADCS_Measurements_Struct {
    int32_t magnetic_field_x_nT;
    int32_t magnetic_field_y_nT;
    int32_t magnetic_field_z_nT;
    int32_t coarse_sun_x_micro;
    int32_t coarse_sun_y_micro;
    int32_t coarse_sun_z_micro;
    int32_t sun_x_micro;
    int32_t sun_y_micro;
    int32_t sun_z_micro;
    int32_t nadir_x_micro;
    int32_t nadir_y_micro;
    int32_t nadir_z_micro;
    int32_t x_angular_rate_milli_deg_per_sec;
    int32_t y_angular_rate_milli_deg_per_sec;
    int32_t z_angular_rate_milli_deg_per_sec;
    int16_t x_wheel_speed_rpm;
    int16_t y_wheel_speed_rpm;
    int16_t z_wheel_speed_rpm;
    int32_t star1_body_x_micro;
    int32_t star1_body_y_micro;
    int32_t star1_body_z_micro;
    int32_t star1_orbit_x_micro;
    int32_t star1_orbit_y_micro;
    int32_t star1_orbit_z_micro;
    int32_t star2_body_x_micro;
    int32_t star2_body_y_micro;
    int32_t star2_body_z_micro;
    int32_t star2_orbit_x_micro;
    int32_t star2_orbit_y_micro;
    int32_t star2_orbit_z_micro;
    int32_t star3_body_x_micro;
    int32_t star3_body_y_micro;
    int32_t star3_body_z_micro;
    int32_t star3_orbit_x_micro;
    int32_t star3_orbit_y_micro;
    int32_t star3_orbit_z_micro;
} ADCS_Measurements_Struct;

#endif /* INC_ADCS_TYPES_H_ */