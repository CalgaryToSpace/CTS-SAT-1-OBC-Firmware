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
#include <time.h>

// Bit 7 of Telecommand and Telemetry Request - Section 4.1 of Firmware Manual
static const uint8_t ADCS_TELECOMMAND = 0;
static const uint8_t ADCS_TELEMETRY_REQUEST = 1;

// ADCS Magic Number defined in Section 5.1 Table 10 of Firmware Manual
static const uint8_t ADCS_MAGIC_NUMBER = 0x5A; // Used to make sure that the reset command is valid

// I2C Write and Read Addresses - Section 5.3 Table 5 of Firmware Manual
static const uint8_t ADCS_i2c_WRITE_ADDRESS = 0xAE;
static const uint8_t ADCS_i2c_READ_ADDRESS = 0xAF;
static const uint8_t ADCS_i2c_ADDRESS = 0x57;

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
typedef enum ADCS_error_flag_enum_t {
    ADCS_ERROR_FLAG_NONE = 0,
    ADCS_ERROR_FLAG_INVALID_ID = 1,
    ADCS_ERROR_FLAG_WRONG_LENGTH = 2,
    ADCS_ERROR_FLAG_INVALID_PARAMS = 3,
    ADCS_ERROR_FLAG_CRC = 4
} ADCS_error_flag_enum_t;

// ADCS Run Modes - Section 5.3.1 Table 75 of Firmware Manual
typedef enum ADCS_run_mode_enum_t {
    ADCS_RUN_MODE_OFF = 0,
    ADCS_RUN_MODE_ENABLED = 1,
    ADCS_RUN_MODE_TRIGGERED = 2,
    ADCS_RUN_MODE_SIMULATION = 3
} ADCS_run_mode_enum_t;

// ADCS Reset Causes - Section 6.1.2 Table 28 of Firmware Manual
typedef enum ADCS_reset_cause_enum_t {
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
} ADCS_reset_cause_enum_t;

// ADCS Boot Causes - Section 6.1.2 Table 29 of Firmware Manual
typedef enum ADCS_boot_cause_enum_t {
    ADCS_BOOT_CAUSE_UNEXPECTED = 0,
    ADCS_BOOT_CAUSE_NOT_USED_1 = 1,
    ADCS_BOOT_CAUSE_COMMUNICATIONS_TIMEOUT = 2,
    ADCS_BOOT_CAUSE_COMMANDED = 3,
    ADCS_BOOT_CAUSE_NOT_USED_2 = 4,
    ADCS_BOOT_CAUSE_SRAM_LATCHUP = 5
} ADCS_boot_cause_enum_t;

// ADCS Running Programs - Section 6.1.2 Table 30 of Firmware Manual
typedef enum ADCS_running_program_enum_t {
    ADCS_RUNNING_PROGRAM_ADCS = 1,
    ADCS_RUNNING_PROGRAM_BOOTLOADER = 2
} ADCS_running_program_enum_t;

// ADCS Control Modes - Section 6.3.1 Table 77 of Firmware Manual
typedef enum ADCS_control_mode_enum_t {
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
} ADCS_control_mode_enum_t;

typedef enum ADCS_estimation_mode_enum_t {
    ADCS_ESTIMATION_MODE_NONE = 0,
    ADCS_ESTIMATION_MODE_MEMS_RATE_SENSING = 1,
    ADCS_ESTIMATION_MODE_MAGNETOMETER_RATE_FILTER = 2,
    ADCS_ESTIMATION_MODE_MAGNETOMETER_RATE_FILTER_WITH_PITCH_ESTIMATION = 3,
    ADCS_ESTIMATION_MODE_MAGNETOMETER_AND_FINE_SUN_TRIAD_ALGORITHM = 4,
    ADCS_ESTIMATION_MODE_FULL_STATE_extended_kalman_filter = 5,
    ADCS_ESTIMATION_MODE_MEMS_GYRO_extended_kalman_filter = 6,
    ADCS_ESTIMATION_MODE_USER_CODED_ESTIMATION_MODE = 7
} ADCS_estimation_mode_enum_t;

typedef enum ADCS_power_select_enum_t {
    ADCS_POWER_SELECT_OFF = 0,
    ADCS_POWER_SELECT_ON = 1,
    ADCS_POWER_SELECT_SAME = 2
} ADCS_power_select_enum_t;

typedef enum ADCS_magnetometer_mode_enum_t {
    ADCS_MAGNETOMETER_MODE_MAIN_SIGNAL = 0,
    ADCS_MAGNETOMETER_MODE_REDUNDANT_SIGNAL = 1,
    ADCS_MAGNETOMETER_MODE_MAIN_MOTOR = 2,
    ADCS_MAGNETOMETER_MODE_NONE = 3
} ADCS_magnetometer_mode_enum_t;

typedef enum ADCS_augmented_sgp4_filter_enum_t {
    ADCS_Augmented_SGP4_FILTER_LOWPASS = 0,
    ADCS_Augmented_SGP4_FILTER_AVERAGE = 1
} ADCS_augmented_sgp4_filter_enum_t;

typedef enum ADCS_axis_select_enum_t {
    ADCS_AXIS_SELECT_POSITIVE_X = 0,
    ADCS_AXIS_SELECT_NEGATIVE_X = 1,
    ADCS_AXIS_SELECT_POSITIVE_Y = 2,
    ADCS_AXIS_SELECT_NEGATIVE_Y = 3,
    ADCS_AXIS_SELECT_POSITIVE_Z = 4,
    ADCS_AXIS_SELECT_NEGATIVE_Z = 5,
    ADCS_AXIS_SELECT_NOT_USED = 6
} ADCS_axis_select_enum_t;

typedef enum ADCS_capture_result_enum_t {
    ADCS_CAPTURE_RESULT_STARTUP = 0,
    ADCS_CAPTURE_RESULT_PENDING = 1,
    ADCS_CAPTURE_RESULT_SUCCESS = 2,
    ADCS_CAPTURE_RESULT_SUCCESS_SHIFT = 3,
    ADCS_CAPTURE_RESULT_TIMEOUT = 4,
    ADCS_CAPTURE_RESULT_SRAM_ERROR = 5
} ADCS_capture_result_enum_t;

typedef enum ADCS_detect_result_enum_t {
    ADCS_DETECT_RESULT_STARTUP = 0,
    ADCS_DETECT_RESULT_NO_DETECT = 1,
    ADCS_DETECT_RESULT_PENDING = 2,
    ADCS_DETECT_RESULT_TOO_MANY_EDGES = 3,
    ADCS_DETECT_RESULT_TOO_FEW_EDGES = 4,
    ADCS_DETECT_RESULT_BAD_FIT = 5,
    ADCS_DETECT_RESULT_SUN_NOT_FOUND = 6,
    ADCS_DETECT_RESULT_SUCCESS = 7
} ADCS_detect_result_enum_t;

typedef enum ADCS_which_cam_sensor_enum_t {
    ADCS_WHICH_CAM_SENSOR_NONE = 0,
    ADCS_WHICH_CAM_SENSOR_CAM1 = 1,
    ADCS_WHICH_CAM_SENSOR_CAM2 = 2
} ADCS_which_cam_sensor_enum_t;

typedef enum ADCS_gps_solution_status_enum_t {
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
} ADCS_gps_solution_status_enum_t;

typedef enum ADCS_gps_axis_enum_t {
    ADCS_GPS_AXIS_X = 0,
    ADCS_GPS_AXIS_Y = 1,
    ADCS_GPS_AXIS_Z = 2
} ADCS_gps_axis_enum_t;


typedef enum ADCS_file_type_enum_t {
    ADCS_FILE_TYPE_TELEMETRY_LOG = 2,
    ADCS_FILE_TYPE_JPG_IMAGE = 3,    
    ADCS_FILE_TYPE_BMP_IMAGE = 4,    
    ADCS_FILE_TYPE_INDEX = 15        
} ADCS_file_type_enum_t;

typedef enum ADCS_current_execution_point_enum_t {
    ADCS_CURRENT_EXECUTION_POINT_BUSY_INITIALIZATION = 0,
    ADCS_CURRENT_EXECUTION_POINT_IDLE = 1,
    ADCS_CURRENT_EXECUTION_POINT_SENSOR_ACTUATOR_COMMS= 2,
    ADCS_CURRENT_EXECUTION_POINT_ADCS_UPDATE = 3,
    ADCS_CURRENT_EXECUTION_POINT_PERIPHERAL_POWER_COMMANDS = 4,
    ADCS_CURRENT_EXECUTION_POINT_CPU_TEMPERATURE_SAMPLING = 5,
    ADCS_CURRENT_EXECUTION_POINT_IMAGE_DOWNLOAD = 6,
    ADCS_CURRENT_EXECUTION_POINT_IMAGE_COMPRESSION = 7,
    ADCS_CURRENT_EXECUTION_POINT_SAVING_IMAGE_TO_SD = 8,
    ADCS_CURRENT_EXECUTION_POINT_LOGGING = 9,
    ADCS_CURRENT_EXECUTION_POINT_LOG_FILE_COMPRESSION = 10,
    ADCS_CURRENT_EXECUTION_POINT_SAVING_LOG_TO_SD = 11,
    ADCS_CURRENT_EXECUTION_POINT_WRITING_TO_FLASH = 12
} ADCS_current_execution_point_enum_t;

typedef enum ADCS_asgp4_mode_enum_t {
    ADCS_ASGP4_MODE_OFF = 0,
    ADCS_ASGP4_MODE_TRIGGER = 1,
    ADCS_ASGP4_MODE_BACKGROUND = 2,
    ADCS_ASGP4_MODE_AUGMENT = 3,
} ADCS_asgp4_mode_enum_t;

typedef enum ADCS_camera_select_enum_t {
    ADCS_CAMERA_SELECT_1 = 0,
    ADCS_CAMERA_SELECT_2 = 1,
    ADCS_CAMERA_SELECT_STAR = 2,
} ADCS_camera_select_enum_t;

typedef enum ADCS_image_size_enum_t {
    ADCS_IMAGE_SIZE_1024_X_1024_PX = 0,
    ADCS_IMAGE_SIZE_512_X_512_PX = 1,
    ADCS_IMAGE_SIZE_256_X_256_PX = 2,
    ADCS_IMAGE_SIZE_128_X_128_PX = 3,
    ADCS_IMAGE_SIZE_64_X_64_PX = 4,
} ADCS_image_size_enum_t;

typedef enum ADCS_sd_log_destination_enum_t {
    ADCS_SD_LOG_DESTINATION_PRIMARY_SD = 0,
    ADCS_SD_LOG_DESTINATION_SECONDARY_SD = 1,
} ADCS_sd_log_destination_enum_t;

typedef enum ADCS_commissioning_step_enum_t {
    ADCS_COMMISSIONING_STEP_DETERMINE_INITIAL_ANGULAR_RATES = 1, 
    ADCS_COMMISSIONING_STEP_INITIAL_DETUMBLING = 2, 
    ADCS_COMMISSIONING_STEP_CONTINUED_DETUMBLING_TO_Y_THOMSON = 3, 
    ADCS_COMMISSIONING_STEP_MAGNETOMETER_DEPLOYMENT = 4, 
    ADCS_COMMISSIONING_STEP_MAGNETOMETER_CALIBRATION = 5, 
    ADCS_COMMISSIONING_STEP_ANGULAR_RATE_AND_PITCH_ANGLE_ESTIMATION = 6, 
    ADCS_COMMISSIONING_STEP_Y_WHEEL_RAMP_UP_TEST = 7, 
    ADCS_COMMISSIONING_STEP_INITIAL_Y_MOMENTUM_ACTIVATION = 8, 
    ADCS_COMMISSIONING_STEP_CONTINUED_Y_MOMENTUM_ACTIVATION_AND_MAGNETOMETER_EKF = 9, 
    ADCS_COMMISSIONING_STEP_CUBESENSE_SUN_NADIR = 10,
    ADCS_COMMISSIONING_STEP_EKF_ACTIVATION_SUN_AND_NADIR = 11,
    ADCS_COMMISSIONING_STEP_CUBESTAR_STAR_TRACKER = 12,
    ADCS_COMMISSIONING_STEP_EKF_ACTIVATION_WITH_STAR_VECTOR_MEASUREMENTS = 13,
    ADCS_COMMISSIONING_STEP_ZERO_BIAS_3_AXIS_REACTION_WHEEL_CONTROL = 14,
    ADCS_COMMISSIONING_STEP_EKF_WITH_RATE_GYRO_STAR_TRACKER_MEASUREMENTS = 15,
    ADCS_COMMISSIONING_STEP_SUN_TRACKING_3_AXIS_CONTROL = 16,
    ADCS_COMMISSIONING_STEP_GROUND_TARGET_TRACKING_CONTROLLER = 17,
    ADCS_COMMISSIONING_STEP_GPS_RECEIVER = 18
} ADCS_commissioning_step_enum_t;

/* Command Structs */

typedef struct ADCS_cmd_ack_struct_t {
    uint8_t last_id;
    bool processed:1; // 1-bit bool
    ADCS_error_flag_enum_t error_flag;
    uint8_t error_index;
} ADCS_cmd_ack_struct_t;

typedef struct ADCS_id_struct_t {
    uint8_t node_type;
    uint8_t interface_version;
    uint8_t major_firmware_version;
    uint8_t minor_firmware_version;
    uint16_t seconds_since_startup;
    uint16_t ms_past_second;
} ADCS_id_struct_t;

typedef struct ADCS_boot_running_status_struct_t {
    ADCS_reset_cause_enum_t reset_cause;
    ADCS_boot_cause_enum_t boot_cause;
    uint16_t boot_counter;
    ADCS_running_program_enum_t boot_program_index;
    uint8_t major_firmware_version;
    uint8_t minor_firmware_version;
} ADCS_boot_running_status_struct_t;

typedef struct ADCS_comms_status_struct_t {
    uint16_t cmd_counter;
    uint16_t tlm_counter;
    bool cmd_buffer_overrun:1; // 1-bit bool
    bool i2c_tlm_error:1; // 1-bit bool
    bool i2c_cmd_error:1; // 1-bit bool
} ADCS_comms_status_struct_t;

typedef struct ADCS_angular_rates_struct_t {
    int32_t x_rate_mdeg_per_sec;
    int32_t y_rate_mdeg_per_sec;
    int32_t z_rate_mdeg_per_sec;
} ADCS_angular_rates_struct_t;

typedef struct ADCS_llh_position_struct_t {
    int32_t latitude_mdeg;
    int32_t longitude_mdeg;
    uint32_t altitude_meters;
} ADCS_llh_position_struct_t;

typedef struct ADCS_Power_Control_struct_t{
    ADCS_power_select_enum_t cube_control_signal;
    ADCS_power_select_enum_t cube_control_motor;
    ADCS_power_select_enum_t cube_sense1;
    ADCS_power_select_enum_t cube_sense2;
    ADCS_power_select_enum_t cube_star_power;
    ADCS_power_select_enum_t cube_wheel1_power;
    ADCS_power_select_enum_t cube_wheel2_power;
    ADCS_power_select_enum_t cube_wheel3_power;
    ADCS_power_select_enum_t motor_power;
    ADCS_power_select_enum_t gps_power;
} ADCS_power_control_struct_t;

typedef struct ADCS_Set_Unix_Time_Save_Mode_struct_t{
    bool save_now:1; // 1-bit bool
    bool save_on_update:1; // 1-bit bool
    bool save_periodic:1; // 1-bit bool
    uint8_t period;
} ADCS_set_unix_time_save_mode_struct_t;

typedef struct ADCS_orbit_params_struct_t {
    double inclination_deg;
    double eccentricity;
    double ascending_node_right_ascension_deg;
    double perigee_argument_deg;
    double b_star_drag_term;
    double mean_motion_orbits_per_day;
    double mean_anomaly_deg;
    double epoch_year_point_day;
} ADCS_orbit_params_struct_t;

typedef struct ADCS_rated_sensor_rates_struct_t {
    int32_t x_mdeg_per_sec; 
    int32_t y_mdeg_per_sec;
    int32_t z_mdeg_per_sec; 
} ADCS_rated_sensor_rates_struct_t;

typedef struct ADCS_wheel_speed_struct_t {
    bool actual_wheel_speed:1; // 1-bit bool
    int16_t x_rpm; 
    int16_t y_rpm;
    int16_t z_rpm; 
} ADCS_wheel_speed_struct_t;

typedef struct ADCS_magnetorquer_command_struct_t {
    int32_t x_ms; 
    int32_t y_ms;
    int32_t z_ms; 
} ADCS_magnetorquer_command_struct_t;

typedef struct ADCS_raw_magnetometer_values_struct_t {
    int16_t x_raw; 
    int16_t y_raw;
    int16_t z_raw; 
} ADCS_raw_magnetometer_values_struct_t;

typedef struct ADCS_fine_angular_rates_struct_t {
    int16_t x_mdeg_per_sec; 
    int16_t y_mdeg_per_sec;
    int16_t z_mdeg_per_sec; 
} ADCS_fine_angular_rates_struct_t;

typedef struct ADCS_magnetometer_config_struct_t {
    int32_t mounting_transform_alpha_angle_mdeg_per_sec;
    int32_t mounting_transform_beta_angle_mdeg_per_sec;
    int32_t mounting_transform_gamma_angle_mdeg_per_sec;
    int16_t channel_1_offset_mdeg_per_sec;
    int16_t channel_2_offset_mdeg_per_sec;
    int16_t channel_3_offset_mdeg_per_sec;
    int16_t sensitivity_matrix_s11_mdeg_per_sec;
    int16_t sensitivity_matrix_s22_mdeg_per_sec;
    int16_t sensitivity_matrix_s33_mdeg_per_sec;
    int16_t sensitivity_matrix_s12_mdeg_per_sec;
    int16_t sensitivity_matrix_s13_mdeg_per_sec;
    int16_t sensitivity_matrix_s21_mdeg_per_sec;
    int16_t sensitivity_matrix_s23_mdeg_per_sec;
    int16_t sensitivity_matrix_s31_mdeg_per_sec;
    int16_t sensitivity_matrix_s32_mdeg_per_sec;
} ADCS_magnetometer_config_struct_t;

typedef struct ADCS_commanded_angles_struct_t {
    int32_t x_mdeg; 
    int32_t y_mdeg;
    int32_t z_mdeg; 
} ADCS_commanded_angles_struct_t;

typedef struct ADCS_estimation_params_struct_t {
    float magnetometer_rate_filter_system_noise;
    float extended_kalman_filter_system_noise;
    float coarse_sun_sensor_measurement_noise;
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
    ADCS_magnetometer_mode_enum_t magnetometer_mode;
    ADCS_magnetometer_mode_enum_t magnetometer_selection_for_raw_magnetometer_telemetry;
    bool automatic_estimation_transition_due_to_rate_sensor_errors:1; // 1-bit bool
    bool wheel_30s_power_up_delay:1; // 1-bit bool
    uint8_t cam1_and_cam2_sampling_period;
} ADCS_estimation_params_struct_t;

typedef struct ADCS_augmented_sgp4_params_struct_t {
    int16_t incl_coefficient_milli;
    int16_t raan_coefficient_milli;
    int16_t ecc_coefficient_milli;
    int16_t aop_coefficient_milli;
    int16_t time_coefficient_milli;
    int16_t pos_coefficient_milli;
    int32_t maximum_position_error_milli;
    ADCS_augmented_sgp4_filter_enum_t augmented_sgp4_filter;
    int64_t xp_coefficient_nano;
    int64_t yp_coefficient_nano;
    uint8_t gps_roll_over;
    int32_t position_sd_milli;
    int16_t velocity_sd_milli;
    uint8_t min_satellites;
    int16_t time_gain_milli;
    int16_t max_lag_milli;
    uint16_t min_samples;
} ADCS_augmented_sgp4_params_struct_t;

typedef struct ADCS_tracking_controller_target_struct_t {
    float longitude_degrees;
    float latitude_degrees;
    float altitude_meters;
} ADCS_tracking_controller_target_struct_t;

typedef struct ADCS_rate_gyro_config_struct_t {
    ADCS_axis_select_enum_t gyro1; 
    ADCS_axis_select_enum_t gyro2; 
    ADCS_axis_select_enum_t gyro3; 
    int16_t x_rate_offset_mdeg_per_sec; 
    int16_t y_rate_offset_mdeg_per_sec; 
    int16_t z_rate_offset_mdeg_per_sec; 
    uint8_t rate_sensor_mult;
} ADCS_rate_gyro_config_struct_t;

typedef struct ADCS_estimated_attitude_angles_struct_t {
    int32_t estimated_roll_angle_mdeg;
    int32_t estimated_pitch_angle_mdeg;
    int32_t estimated_yaw_angle_mdeg;
} ADCS_estimated_attitude_angles_struct_t;

typedef struct ADCS_magnetic_field_vector_struct_t {
    int32_t x_nT;
    int32_t y_nT;
    int32_t z_nT;
} ADCS_magnetic_field_vector_struct_t;

typedef struct ADCS_fine_sun_vector_struct_t {
    int32_t x_micro;
    int32_t y_micro;
    int32_t z_micro;
} ADCS_fine_sun_vector_struct_t;

typedef struct ADCS_nadir_vector_struct_t {
    int32_t x_micro;
    int32_t y_micro;
    int32_t z_micro;
} ADCS_nadir_vector_struct_t;

typedef struct ADCS_quaternion_error_vector_struct_t {
    int32_t quaternion_error_q1_micro;
    int32_t quaternion_error_q2_micro;
    int32_t quaternion_error_q3_micro;
} ADCS_quaternion_error_vector_struct_t;

typedef struct ADCS_estimated_gyro_bias_struct_t {
    int32_t estimated_x_gyro_bias_mdeg_per_sec;
    int32_t estimated_y_gyro_bias_mdeg_per_sec;
    int32_t estimated_z_gyro_bias_mdeg_per_sec;
} ADCS_estimated_gyro_bias_struct_t;

typedef struct ADCS_estimation_innovation_vector_struct_t {
    int32_t innovation_vector_x_micro;
    int32_t innovation_vector_y_micro;
    int32_t innovation_vector_z_micro;
} ADCS_estimation_innovation_vector_struct_t;

typedef struct ADCS_raw_cam_sensor_struct_t {
    ADCS_which_cam_sensor_enum_t which_sensor;
    int16_t cam_centroid_x;
    int16_t cam_centroid_y;
    ADCS_capture_result_enum_t cam_capture_status;
    ADCS_detect_result_enum_t cam_detection_result;
} ADCS_raw_cam_sensor_struct_t;

typedef struct ADCS_raw_coarse_sun_sensor_1_to_6_struct_t {
    uint8_t coarse_sun_sensor_1;
    uint8_t coarse_sun_sensor_2;
    uint8_t coarse_sun_sensor_3;
    uint8_t coarse_sun_sensor_4;
    uint8_t coarse_sun_sensor_5;
    uint8_t coarse_sun_sensor_6;
} ADCS_raw_coarse_sun_sensor_1_to_6_struct_t;

typedef struct ADCS_raw_coarse_sun_sensor_7_to_10_struct_t {
    uint8_t coarse_sun_sensor_7;
    uint8_t coarse_sun_sensor_8;
    uint8_t coarse_sun_sensor_9;
    uint8_t coarse_sun_sensor_10;
} ADCS_raw_coarse_sun_sensor_7_to_10_struct_t;

typedef struct ADCS_cubecontrol_current_struct_t {
    double cubecontrol_3v3_current_mA;
    double cubecontrol_5v_current_mA;
    double cubecontrol_vbat_current_mA;
} ADCS_cubecontrol_current_struct_t;

typedef struct ADCS_raw_gps_status_struct_t {
    ADCS_gps_solution_status_enum_t gps_solution_status;
    uint8_t num_tracked_satellites;
    uint8_t num_used_satellites;
    uint8_t counter_xyz_log;
    uint8_t counter_range_log;
    uint8_t response_message_gps_log;
} ADCS_raw_gps_status_struct_t;

typedef struct ADCS_raw_gps_time_struct_t {
    uint16_t gps_reference_week;
    uint32_t gps_time_ms; // in seconds
} ADCS_raw_gps_time_struct_t;

typedef struct ADCS_raw_gps_struct_t {
    ADCS_gps_axis_enum_t axis;
    int32_t ecef_position_meters;    
    int16_t ecef_velocity_meters_per_sec;   
} ADCS_raw_gps_struct_t;

typedef struct ADCS_measurements_struct_t {
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
    int32_t x_angular_rate_mdeg_per_sec;
    int32_t y_angular_rate_mdeg_per_sec;
    int32_t z_angular_rate_mdeg_per_sec;
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
} ADCS_measurements_struct_t;

typedef struct ADCS_file_info_struct_t {       
    // reordered to be slightly less memory-intensive
    uint16_t file_crc16;      
    uint8_t file_counter;
    ADCS_file_type_enum_t file_type;
    bool busy_updating:1; // 1-bit bool          
    uint32_t file_size;      
    time_t file_date_time; 
} ADCS_file_info_struct_t;

typedef struct ADCS_download_block_ready_struct_t {
    bool ready:1; // 1-bit bool   
    bool parameter_error:1; // 1-bit bool
    uint16_t block_crc16;   
    uint16_t block_length;  
} ADCS_download_block_ready_struct_t; 

typedef struct ADCS_sd_card_format_erase_progress_struct_t {
    bool format_busy:1; // 1-bit bool
    bool erase_all_busy:1; // 1-bit bool
} ADCS_sd_card_format_erase_progress_struct_t;

typedef struct ADCS_file_download_buffer_struct_t {
    uint16_t packet_counter; 
    uint8_t file_bytes[20];  
} ADCS_file_download_buffer_struct_t;
typedef struct ADCS_acp_execution_state_struct_t {
    uint16_t time_since_iteration_start_ms;
    ADCS_current_execution_point_enum_t current_execution_point;
} ADCS_acp_execution_state_struct_t;

typedef struct ADCS_current_state_1_struct_t {
    ADCS_estimation_mode_enum_t estimation_mode;
    ADCS_control_mode_enum_t control_mode;  
    ADCS_run_mode_enum_t run_mode;  
    ADCS_asgp4_mode_enum_t asgp4_mode; 
    bool cubecontrol_signal_enabled:1; // 1-bit bool
    bool cubecontrol_motor_enabled:1; // 1-bit bool
    bool cubesense1_enabled:1; // 1-bit bool
    bool cubesense2_enabled:1; // 1-bit bool
    bool cubewheel1_enabled:1; // 1-bit bool
    bool cubewheel2_enabled:1; // 1-bit bool
    bool cubewheel3_enabled:1; // 1-bit bool
    bool cubestar_enabled:1; // 1-bit bool
    bool gps_receiver_enabled:1; // 1-bit bool
    bool gps_lna_power_enabled:1; // 1-bit bool
    bool motor_driver_enabled:1; // 1-bit bool
    bool sun_above_local_horizon:1; // 1-bit bool
    bool cubesense1_comm_error:1; // 1-bit bool
    bool cubesense2_comm_error:1; // 1-bit bool
    bool cubecontrol_signal_comm_error:1; // 1-bit bool
    bool cubecontrol_motor_comm_error:1; // 1-bit bool
    bool cubewheel1_comm_error:1; // 1-bit bool
    bool cubewheel2_comm_error:1; // 1-bit bool
    bool cubewheel3_comm_error:1; // 1-bit bool
    bool cubestar_comm_error:1; // 1-bit bool
    bool magnetometer_range_error:1; // 1-bit bool
    bool cam1_sram_overcurrent_detected:1; // 1-bit bool
    bool cam1_3v3_overcurrent_detected:1; // 1-bit bool
    bool cam1_sensor_busy_error:1; // 1-bit bool
    bool cam1_sensor_detection_error:1; // 1-bit bool
    bool sun_sensor_range_error:1; // 1-bit bool
    bool cam2_sram_overcurrent_detected:1; // 1-bit bool
    bool cam2_3v3_overcurrent_detected:1; // 1-bit bool
    bool cam2_sensor_busy_error:1; // 1-bit bool
    bool cam2_sensor_detection_error:1; // 1-bit bool
    bool nadir_sensor_range_error:1; // 1-bit bool
    bool rate_sensor_range_error:1; // 1-bit bool
    bool wheel_speed_range_error:1; // 1-bit bool
    bool coarse_sun_sensor_error:1; // 1-bit bool
    bool startracker_match_error:1; // 1-bit bool
    bool startracker_overcurrent_detected:1; // 1-bit bool
} ADCS_current_state_1_struct_t;

typedef struct ADCS_raw_star_tracker_struct_t {
    uint8_t num_stars_detected;
    uint8_t star_image_noise;
    uint8_t invalid_stars;
    uint8_t num_stars_identified;
    uint8_t identification_mode;
    uint8_t image_dark_value;
    uint16_t sample_period;
    bool image_capture_success:1; // 1-bit bool
    bool detection_success:1; // 1-bit bool
    bool identification_success:1; // 1-bit bool
    bool attitude_success:1; // 1-bit bool
    bool processing_time_error:1; // 1-bit bool
    bool tracking_module_enabled:1; // 1-bit bool
    bool prediction_enabled:1; // 1-bit bool
    bool comms_error:1; // 1-bit bool
    uint8_t star1_confidence;
    uint8_t star2_confidence;
    uint8_t star3_confidence;
    uint16_t magnitude_star1;
    uint16_t magnitude_star2;
    uint16_t magnitude_star3;
    uint16_t catalogue_star1;
    int16_t centroid_x_star1;
    int16_t centroid_y_star1;
    uint16_t catalogue_star2;
    int16_t centroid_x_star2;
    int16_t centroid_y_star2;
    uint16_t catalogue_star3;
    int16_t centroid_x_star3;
    int16_t centroid_y_star3;
    uint16_t capture_time_ms;
    uint16_t detection_time_ms;
    uint16_t identification_time_ms;
    int32_t x_axis_rate_micro;
    int32_t y_axis_rate_micro;
    int32_t z_axis_rate_micro;
    int32_t q0_micro;
    int32_t q1_micro;
    int32_t q2_micro;
} ADCS_raw_star_tracker_struct_t;

typedef struct ADCS_sd_log_config_struct {
    uint8_t which_log;
    uint8_t log_bitmask[10];
    uint16_t log_period_s; 
    ADCS_sd_log_destination_enum_t which_sd;
} ADCS_sd_log_config_struct;

#endif /* INC_ADCS_TYPES_H_ */