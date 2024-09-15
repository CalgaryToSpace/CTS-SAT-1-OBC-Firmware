#ifndef __INCLUDE_GUARD_TEMPERATURE_SENSOR_H__
#define __INCLUDE_GUARD_TEMPERATURE_SENSOR_H__

#include <stdint.h>

typedef enum  
{
    ONE_BIT_PRECISION = 0,
    TWO_BIT_PRECISION,
    THREE_BIT_PRECISION,
    FOUR_BIT_PRECISION,
} DataPrecision;

int read_temperature(float *result);

float calculate_decimal_portion(uint8_t bits, DataPrecision precision);

float calculate_integer_portion(uint8_t bits);

#endif