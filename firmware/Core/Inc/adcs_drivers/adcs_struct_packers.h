#ifndef INC_ADCS_STRUCT_PACKERS_H_
#define INC_ADCS_STRUCT_PACKERS_H_

#include "adcs_types.h"

// ADCS packer functions
uint8_t ADCS_Pack_to_Ack(uint8_t* data_received, ADCS_CMD_Ack_Struct *result);
uint8_t ADCS_Pack_to_Identification(uint8_t* data_received, ADCS_ID_Struct *result);
uint8_t ADCS_Pack_to_Program_Status(uint8_t* data_received, ADCS_Boot_Running_Status_Struct *result);
uint8_t ADCS_Pack_to_Comms_Status(uint8_t* data_received, ADCS_Comms_Status_Struct *result);
uint8_t ADCS_Pack_to_Power_Control(uint8_t* data_received, ADCS_Power_Control_Struct *result);
uint8_t ADCS_Pack_to_Angular_Rates(uint8_t* data_received, ADCS_Angular_Rates_Struct *result);
uint8_t ADCS_Pack_to_LLH_Position(uint8_t* data_received, ADCS_LLH_Position_Struct *result);
uint8_t ADCS_Pack_to_Unix_Time_Save_Mode(uint8_t* data_received, ADCS_Set_Unix_Time_Save_Mode_Struct *result);
uint8_t ADCS_Pack_to_Orbit_Params(uint8_t* data_received, ADCS_Orbit_Params_Struct *result);
uint8_t ADCS_Pack_to_Rated_Sensor_Rates(uint8_t* data_received, ADCS_Rated_Sensor_Rates_Struct *result);
uint8_t ADCS_Pack_to_Wheel_Speed(uint8_t* data_received, ADCS_Wheel_Speed_Struct *result);
uint8_t ADCS_Pack_to_Magnetorquer_Command(uint8_t* data_received, ADCS_Magnetorquer_Command_Struct *result);
uint8_t ADCS_Pack_to_Raw_Magnetometer_Values(uint8_t* data_received, ADCS_Raw_Mag_TLM_Struct *result);
uint8_t ADCS_Pack_to_Fine_Angular_Rates(uint8_t* data_received, ADCS_Fine_Angular_Rates_Struct *result);
uint8_t ADCS_Pack_to_Magnetometer_Config(uint8_t* data_received, ADCS_Magnetometer_Config_Struct *result);
uint8_t ADCS_Pack_to_Commanded_Attitude_Angles(uint8_t* data_received, ADCS_Commanded_Angles_Struct *result);
uint8_t ADCS_Pack_to_Estimation_Params(uint8_t* data_received, ADCS_Estimation_Params_Struct *result);
uint8_t ADCS_Pack_to_ASGP4_Params(uint8_t* data_received, ADCS_ASGP4_Params_Struct *result);
uint8_t ADCS_Pack_to_Tracking_Controller_Target_Reference(uint8_t* data_received, ADCS_Tracking_Controller_Target_Struct *result);
uint8_t ADCS_Pack_to_Rate_Gyro_Config(uint8_t* data_received, ADCS_Rate_Gyro_Config_Struct *result);
uint8_t ADCS_Pack_to_Estimated_Attitude_Angles(uint8_t *data_received, ADCS_Estimated_Attitude_Angles_Struct *angles);
uint8_t ADCS_Pack_to_Magnetic_Field_Vector(uint8_t *data_received, ADCS_Magnetic_Field_Vector_Struct *vector_components);
uint8_t ADCS_Pack_to_Fine_Sun_Vector(uint8_t *data_received, ADCS_Fine_Sun_Vector_Struct *vector_components);
uint8_t ADCS_Pack_to_Nadir_Vector(uint8_t *data_received, ADCS_Nadir_Vector_Struct *vector_components);
uint8_t ADCS_Pack_to_Commanded_Wheel_Speed(uint8_t *data_received, ADCS_Wheel_Speed_Struct *result);
uint8_t ADCS_Pack_to_IGRF_Magnetic_Field_Vector(uint8_t *data_received, ADCS_Magnetic_Field_Vector_Struct *vector_components);
uint8_t ADCS_Pack_to_Quaternion_Error_Vector(uint8_t *data_received, ADCS_Quaternion_Error_Vector_Struct *result);
uint8_t ADCS_Pack_to_Estimated_Gyro_Bias(uint8_t* data_received, ADCS_Estimated_Gyro_Bias_Struct *result);
uint8_t ADCS_Pack_to_Estimation_Innovation_Vector(uint8_t* data_received, ADCS_Estimation_Innovation_Vector_Struct* result);
uint8_t ADCS_Pack_to_Raw_Cam1_Sensor(uint8_t* data_received, ADCS_Raw_Cam_Sensor_Struct* result);
uint8_t ADCS_Pack_to_Raw_Cam2_Sensor(uint8_t* data_received, ADCS_Raw_Cam_Sensor_Struct* result);
uint8_t ADCS_Pack_to_Raw_CSS_1_to_6(uint8_t* data_received, ADCS_Raw_CSS_1_to_6_Struct* result);
uint8_t ADCS_Pack_to_Raw_CSS_7_to_10(uint8_t* data_received, ADCS_Raw_CSS_7_to_10_Struct* result);
uint8_t ADCS_Pack_to_CubeControl_Current(uint8_t* data_received, ADCS_CubeControl_Current_Struct* result);
uint8_t ADCS_Pack_to_Raw_GPS_Status(uint8_t* data_received, ADCS_Raw_GPS_Status_Struct* result);
uint8_t ADCS_Pack_to_Raw_GPS_Time(uint8_t* data_received, ADCS_Raw_GPS_Time_Struct* result);
uint8_t ADCS_Pack_to_Raw_GPS(ADCS_GPS_Axis axis, uint8_t *data_received, ADCS_Raw_GPS_Struct *result);
uint8_t ADCS_Pack_to_Measurements(uint8_t* telemetry_data, ADCS_Measurements_Struct* measurements);

#endif /* INC_ADCS_STRUCT_PACKERS_H_ */