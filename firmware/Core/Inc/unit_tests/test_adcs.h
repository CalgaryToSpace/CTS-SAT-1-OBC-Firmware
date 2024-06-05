#ifndef __INCLUDE_GUARD__ADCS_TEST_PROTOTYPES_H__
#define __INCLUDE_GUARD__ADCS_TEST_PROTOTYPES_H__

#include "adcs_types.h"

// TODO: ADCS test header
uint8_t TEST_EXEC__ADCS_Pack_to_Ack();
uint8_t TEST_EXEC__ADCS_Pack_to_Identification();
uint8_t TEST_EXEC__ADCS_Pack_to_Program_Status();
uint8_t TEST_EXEC__ADCS_Pack_to_Comms_Status();
uint8_t TEST_EXEC__ADCS_Pack_to_Power_Control();
uint8_t TEST_EXEC__ADCS_Pack_to_Angular_Rates();
uint8_t TEST_EXEC__ADCS_Pack_to_LLH_Position();
uint8_t TEST_EXEC__ADCS_Pack_to_Unix_Time_Save_Mode();
uint8_t TEST_EXEC__ADCS_Pack_to_Orbit_Params();
uint8_t TEST_EXEC__ADCS_Pack_to_Rated_Sensor_Rates();
uint8_t TEST_EXEC__ADCS_Pack_to_Wheel_Speed();
uint8_t TEST_EXEC__ADCS_Pack_to_Magnetorquer_Command_Time();
uint8_t TEST_EXEC__ADCS_Pack_to_Raw_Magnetometer_Values();
uint8_t TEST_EXEC__ADCS_Pack_to_Fine_Angular_Rates();
uint8_t TEST_EXEC__ADCS_Pack_to_Magnetometer_Config();
uint8_t TEST_EXEC__ADCS_Pack_to_Commanded_Attitude_Angles();
uint8_t TEST_EXEC__ADCS_Pack_to_Estimation_Params();
uint8_t TEST_EXEC__ADCS_Pack_to_ASGP4_Params();
uint8_t TEST_EXEC__ADCS_Pack_to_Tracking_Controller_Target_Reference();
uint8_t TEST_EXEC__ADCS_Pack_to_Rate_Gyro_Config();

#endif // __INCLUDE_GUARD__ADCS_TEST_PROTOTYPES_H__


