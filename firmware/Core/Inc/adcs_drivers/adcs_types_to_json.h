#ifndef INC_ADCS_TYPES_TO_JSON_H_
#define INC_ADCS_TYPES_TO_JSON_H_

#include "adcs_types.h"

/* Structure: (EPS_vpid_eng_t is the struct)
uint8_t EPS_vpid_eng_TO_json(
    const EPS_vpid_eng_t *data,
    char json_output_str[],
    uint16_t json_output_str_len
);
*/

uint8_t ADCS_CMD_Ack_Struct_TO_json(const ADCS_CMD_Ack_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_ID_Struct_TO_json(const ADCS_ID_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Boot_Running_Status_Struct_TO_json(const ADCS_Boot_Running_Status_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Comms_Status_Struct_TO_json(const ADCS_Comms_Status_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Angular_Rates_Struct_TO_json(const ADCS_Angular_Rates_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_LLH_Position_Struct_TO_json(const ADCS_LLH_Position_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Power_Control_Struct_TO_json(const ADCS_Power_Control_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Set_Unix_Time_Save_Mode_Struct_TO_json(const ADCS_Set_Unix_Time_Save_Mode_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Orbit_Params_Struct_TO_json(const ADCS_Orbit_Params_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Rated_Sensor_Rates_Struct_TO_json(const ADCS_Rated_Sensor_Rates_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Wheel_Speed_Struct_TO_json(const ADCS_Wheel_Speed_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Magnetorquer_Command_Struct_TO_json(const ADCS_Magnetorquer_Command_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Raw_Magnetometer_Values_Struct_TO_json(const ADCS_Raw_Magnetometer_Values_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Fine_Angular_Rates_Struct_TO_json(const ADCS_Fine_Angular_Rates_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Magnetometer_Config_Struct_TO_json(const ADCS_Magnetometer_Config_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Commanded_Angles_Struct_TO_json(const ADCS_Commanded_Angles_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Estimation_Params_Struct_TO_json(const ADCS_Estimation_Params_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_ASGP4_Params_Struct_TO_json(const ADCS_ASGP4_Params_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Tracking_Controller_Target_Struct_TO_json(const ADCS_Tracking_Controller_Target_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Rate_Gyro_Config_Struct_TO_json(const ADCS_Rate_Gyro_Config_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Estimated_Attitude_Angles_Struct_TO_json(const ADCS_Estimated_Attitude_Angles_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Magnetic_Field_Vector_Struct_TO_json(const ADCS_Magnetic_Field_Vector_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Fine_Sun_Vector_Struct_TO_json(const ADCS_Fine_Sun_Vector_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Nadir_Vector_Struct_TO_json(const ADCS_Nadir_Vector_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Quaternion_Error_Vector_Struct_TO_json(const ADCS_Quaternion_Error_Vector_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Estimated_Gyro_Bias_Struct_TO_json(const ADCS_Estimated_Gyro_Bias_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Estimation_Innovation_Vector_Struct_TO_json(const ADCS_Estimation_Innovation_Vector_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Raw_Cam_Sensor_Struct_TO_json(const ADCS_Raw_Cam_Sensor_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Raw_CSS_1_to_6_Struct_TO_json(const ADCS_Raw_CSS_1_to_6_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Raw_CSS_7_to_10_Struct_TO_json(const ADCS_Raw_CSS_7_to_10_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_CubeControl_Current_Struct_TO_json(const ADCS_CubeControl_Current_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Raw_GPS_Status_Struct_TO_json(const ADCS_Raw_GPS_Status_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Raw_GPS_Time_Struct_TO_json(const ADCS_Raw_GPS_Time_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Raw_GPS_Struct_TO_json(const ADCS_Raw_GPS_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_Measurements_Struct_TO_json(const ADCS_Measurements_Struct *data, char json_output_str[], uint16_t json_output_str_len);
uint8_t ADCS_generic_telemetry_uint8_array_TO_json(const uint8_t *data, const uint16_t data_length, char json_output_str[], uint16_t json_output_str_len);

#endif /* INC_ADCS_TYPES_TO_JSON_H_ */