#ifndef __INCLUDE_GUARD_TEMPERATURE_SENSOR_H__
#define __INCLUDE_GUARD_TEMPERATURE_SENSOR_H__

#include <stdint.h>

typedef enum  
{
    TEMP_SENSOR_ONE_BIT_PRECISION = 0,
    TEMP_SENSOR_TWO_BIT_PRECISION,
    TEMP_SENSOR_THREE_BIT_PRECISION,
    TEMP_SENSOR_FOUR_BIT_PRECISION,
} Temperature_Sensor_Data_Precision_t;

uint8_t read_temperature(float *result);

float calculate_decimal_portion(uint8_t bits, Temperature_Sensor_Data_Precision_t precision);

#endif