/*
 * adcs_types.h
 *
 *  Created on: Oct 28, 2023
 *      Authors: Saksham Puri, Zachary Uy, Nadeem Moosa
 */

#include "main.h"
#include "adcs_ids.h"
#include <stdbool.h>

#ifndef INC_ADCS_TYPES_H_
#define INC_ADCS_TYPES_H_

//Bit 7 of Telecommand and Telemetry Request - Section 4.1 of Firmware Manual
#define ADCS_TELECOMMAND 0
#define ADCS_TELEMETRY_REQUEST 1

//ADCS Magic Number defined in Section 5.1 Table 10 of Firmware Manual
#define ADCS_MAGIC_NUMBER 0x5A // Used to make sure that the reset command is valid

//I2C Write and Read Addresses - Section 5.3 Table 5 of Firmware Manual
#define ADCS_I2C_WRITE 0xAE
#define ADCS_I2C_READ 0xAF
#define ADCS_I2C_ADDRESS 0x57

//Identifiers of UART Start of Message and End of Message - Section 4.2 of Firmware Manual
// 0x1F | 0x7F ....... | 0x1F 0xFF
#define ADCS_ESC_CHARACTER 0x1F
#define ADCS_START_MESSAGE 0x7F
#define ADCS_END_MESSAGE 0xFF

// CRC defines
#define CRC_POLY 0x91

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

//ADCS Boot Causes - Section 6.1.2 Table 29 of Firmware Manual
typedef enum ADCS_Boot_Cause {
    ADCS_Boot_Cause_Unexpected,
    ADCS_Boot_Cause_Not_Used_1,
    ADCS_Boot_Cause_Communications_Timeout,
    ADCS_Boot_Cause_Commanded,
    ADCS_Boot_Cause_Not_Used_2,
    ADCS_Boot_Cause_SRAM_Latchup
} ADCS_Boot_Cause;

//ADCS Running Programs - Section 6.1.2 Table 30 of Firmware Manual
typedef enum ADCS_Running_Program {
    ADCS_Running_ADCS = 1,
    ADCS_Running_Bootloader = 2
} ADCS_Running_Program;

//ADCS Control Modes - Section 6.3.1 Table 77 of Firmware Manual
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
	ADCS_Power_Select_Off,
	ADCS_Power_Select_On,
	ADCS_Power_Select_Same
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

// TODO: enums

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
	uint8_t x; 
	uint8_t y;
	uint8_t z; 
} ADCS_Wheel_Speed_Struct;

typedef struct ADCS_Magnetorquer_Command_Struct {
	double x; 
	double y;
	double z; 
} ADCS_Magnetorquer_Command_Struct;

typedef struct ADCS_Raw_Mag_TLM_Struct {
	uint8_t x; 
	uint8_t y;
	uint8_t z; 
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

// TODO: structs

/* Function Definitions */

#define ADCS_I2C_HANDLE &hi2c1
#define ADCS_INCLUDE_CHECKSUM 1
#define ADCS_NO_CHECKSUM 0
#define WRITE_STRUCT_TO_MEMORY(struct_to_write) // memory module function: write struct to memory
void switch_order(uint8_t *array, uint16_t value, int index);
void switch_order_32(uint8_t *array, uint32_t value, int index);

// TC/TLM functions (basic communication)
void I2C_telecommand_wrapper(I2C_HandleTypeDef *hi2c, uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum);
void I2C_telemetry_wrapper(I2C_HandleTypeDef *hi2c, uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum);
void send_I2C_telecommand(I2C_HandleTypeDef *hi2c, uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum);
uint8_t send_I2C_telemetry_request(I2C_HandleTypeDef *hi2c, uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum);

// CRC functions
void COMMS_Crc8Init();
uint8_t COMMS_Crc8Checksum(uint8_t* buffer, uint16_t len);

// UART debug functions
int I2C_Scan(void);
uint8_t send_UART_telecommand(UART_HandleTypeDef *huart, uint8_t id, uint8_t* data, uint32_t data_length);
void PRINT_STRING_UART(UART_HandleTypeDef *huart, void *string);
void PRINT_NEW_LINE(UART_HandleTypeDef *huart);

// ADCS functions
void ADCS_Reset(I2C_HandleTypeDef *hi2c);
void ADCS_Identification(I2C_HandleTypeDef *hi2c);
void ADCS_Program_Status(I2C_HandleTypeDef *hi2c);
void ADCS_Communication_Status(I2C_HandleTypeDef *hi2c);
void ADCS_Deploy_Magnetometer(I2C_HandleTypeDef *hi2c, uint8_t deploy_timeout);
void ADCS_Set_Run_Mode(I2C_HandleTypeDef *hi2c, ADCS_Run_Mode mode);
void ADCS_Clear_Errors(I2C_HandleTypeDef *hi2c);
void ADCS_Attitude_Control_Mode(I2C_HandleTypeDef *hi2c, ADCS_Control_Mode mode, uint16_t timeout);
void ADCS_Stop_Reaction_Wheels(I2C_HandleTypeDef *hi2c);
void ADCS_Attitude_Estimation_Mode(I2C_HandleTypeDef *hi2c, ADCS_Estimation_Mode mode);
ADCS_TC_Ack_Struct ADCS_TC_Ack(I2C_HandleTypeDef *hi2c);
void ADCS_Run_Once(I2C_HandleTypeDef *hi2c);
void ADCS_Set_Magnetometer_Mode(I2C_HandleTypeDef *hi2c, ADCS_Magnetometer_Mode mode);
void ADCS_Set_Magnetorquer_Output(I2C_HandleTypeDef *hi2c, double x_duty, double y_duty, double z_duty);
void ADCS_Set_Wheel_Speed(I2C_HandleTypeDef *hi2c, uint16_t x_speed, uint16_t y_speed, uint16_t z_speed);
void ADCS_Save_Config(I2C_HandleTypeDef *hi2c);
void ADCS_Estimate_Angular_Rates(I2C_HandleTypeDef *hi2c);
void ADCS_Get_LLH_Position(I2C_HandleTypeDef *hi2c);
void ADCS_Get_Power_Control(I2C_HandleTypeDef *hi2c);
void ADCS_Set_Power_Control(I2C_HandleTypeDef *hi2c, ADCS_Power_Select cube_control_signal, ADCS_Power_Select cube_control_motor, ADCS_Power_Select cube_sense1,
        ADCS_Power_Select cube_sense2, ADCS_Power_Select cube_star_power, ADCS_Power_Select cube_wheel1_power,
        ADCS_Power_Select cube_wheel2_power, ADCS_Power_Select cube_wheel3_power, ADCS_Power_Select motor_power,
        ADCS_Power_Select gps_power);
void ADCS_Set_Magnetometer_Config(I2C_HandleTypeDef *hi2c,
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
void ADCS_Bootloader_Clear_Errors(I2C_HandleTypeDef *hi2c);
void ADCS_Set_Unix_Time_Save_Mode(I2C_HandleTypeDef *hi2c, bool save_now, bool save_on_update, bool save_periodic, uint8_t period);
void ADCS_Get_Unix_Time_Save_Mode(I2C_HandleTypeDef *hi2c);
void ADCS_Set_SGP4_Orbit_Params(I2C_HandleTypeDef *hi2c, double inclination, double eccentricity, double ascending_node_right_ascension, double perigee_argument, double b_star_drag_term, double mean_motion, double mean_anomaly, double epoch);
void ADCS_Get_SGP4_Orbit_Params(I2C_HandleTypeDef *hi2c);
void ADCS_Save_Orbit_Params(I2C_HandleTypeDef *hi2c);
void ADCS_Rate_Sensor_Rates(I2C_HandleTypeDef *hi2c);
void ADCS_Get_Wheel_Speed(I2C_HandleTypeDef *hi2c);
void ADCS_Get_Magnetorquer_Command_Time(I2C_HandleTypeDef *hi2c);
void ADCS_Get_Raw_Magnetometer_Values(I2C_HandleTypeDef *hi2c);
void ADCS_Fine_Angular_Rates(I2C_HandleTypeDef *hi2c);
void ADCS_Estimate_Fine_Angular_Rates(I2C_HandleTypeDef *hi2c);
void ADCS_Get_Magnetometer_Config(I2C_HandleTypeDef *hi2c);
void ADCS_Get_Commanded_Attitude_Angles(I2C_HandleTypeDef *hi2c);
void ADCS_Set_Commanded_Attitude_Angles(I2C_HandleTypeDef *hi2c, double x, double y, double z);
void ADCS_Set_Estimation_Params(I2C_HandleTypeDef *hi2c, 
								float magnetometer_rate_filter_system_noise, 
                                float ekf_system_noise, 
                                float css_measurement_noise, 
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
void ADCS_Get_Estimation_Params(I2C_HandleTypeDef *hi2c);
void ADCS_Set_ASGP4_Params(I2C_HandleTypeDef *hi2c,
                           double incl_coefficient,
                           double raan_coefficient,
                           double ecc_coefficient,
                           double aop_coefficient,
                           double time_coefficient,
                           double pos_coefficient,
                           double maximum_position_error,
                           ADCS_ASGP4_Filter asgp4_filter,
                           double xp_coefficient,
                           double yp_coefficient,
                           uint8_t gps_roll_over,
                           double position_sd,
                           double velocity_sd,
                           uint8_t min_satellites,
                           double time_gain,
                           double max_lag,
                           uint16_t min_samples);
void ADCS_Get_ASGP4_Params(I2C_HandleTypeDef *hi2c);
void ADCS_Set_Tracking_Controller_Target_Reference(I2C_HandleTypeDef *hi2c, float lon, float lat, float alt);
void ADCS_Get_Tracking_Controller_Target_Reference(I2C_HandleTypeDef *hi2c);
void ADCS_Set_Rate_Gyro_Config(I2C_HandleTypeDef *hi2c, ADCS_Axis_Select gyro1, ADCS_Axis_Select gyro2, ADCS_Axis_Select gyro3, double x_rate_offset, double y_rate_offset, double z_rate_offset, uint8_t rate_sensor_mult);
void ADCS_Get_Rate_Gyro_Config(I2C_HandleTypeDef *hi2c);
// TODO: prototypes

#endif /* INC_ADCS_TYPES_H_ */
