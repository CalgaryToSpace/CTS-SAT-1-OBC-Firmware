#ifndef __INCLUDE_GUARD_TEMPERATURE_SENSOR_H__
#define __INCLUDE_GUARD_TEMPERATURE_SENSOR_H__

#include <stdint.h>

typedef enum  
{
    ONE_BIT_PRECISION = 0,
    TWO_BIT_PRECISION,
    THREE_BIT_PRECISION,
    FOUR_BIT_PRECISION,
} Temperature_Sensor_Data_Precision_t;

uint8_t read_temperature(float *result);

float calculate_decimal_portion(uint8_t bits, Temperature_Sensor_Data_Precision_t precision);

#endif