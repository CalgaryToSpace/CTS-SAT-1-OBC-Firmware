#ifndef __INCLUDE_GUARD_TEMPERATURE_SENSOR_H__
#define __INCLUDE_GUARD_TEMPERATURE_SENSOR_H__

#include <stdint.h>

static const float OBC_TEMP_SENSOR_nine_bit_precision_coefficient = 2;
static const float OBC_TEMP_SENSOR_ten_bit_precision_coefficient = 4;
static const float OBC_TEMP_SENSOR_eleven_bit_precision_coefficient = 8;
static const float OBC_TEMP_SENSOR_twelve_bit_precision_coefficient = 16;

static const uint16_t OBC_TEMP_SENSOR_nine_bit_scaling_factor = 10;
static const uint16_t OBC_TEMP_SENSOR_ten_bit_scaling_factor = 100;
static const uint16_t OBC_TEMP_SENSOR_eleven_bit_scaling_factor = 1000;
static const uint16_t OBC_TEMP_SENSOR_twelve_bit_scaling_factor = 10000;

typedef enum  
{
    OBC_TEMP_SENSOR_TWELVE_BIT_PRECISION_INSIGNIFICANT_BYTES = 4,
    OBC_TEMP_SENSOR_ELEVEN_BIT_PRECISION_INSIGNIFICANT_BYTES,
    OBC_TEMP_SENSOR_TEN_BIT_PRECISION_INSIGNIFICANT_BYTES,
    OBC_TEMP_SENSOR_NINE_BIT_PRECISION_INSIGNIFICANT_BYTES,
} Temperature_Sensor_Data_Precision_Insignificant_Bytes_t;

// used for unit tests
struct Set_Precision_Data
{
    uint16_t precision_scaling_factor;
    Temperature_Sensor_Data_Precision_Insignificant_Bytes_t precision_insignificant_bits;
    float precision_coefficient;
    uint8_t config_write_data; // data to be written to the config register.
    uint32_t conversion_delay_ms; // time it takes for sensor to convert to specific precision
};


uint8_t OBC_TEMP_SENSOR__read_temperature(int32_t *result);
int32_t OBC_TEMP_SENSOR__convert_raw_to_deg_c(uint8_t raw_bytes[], uint8_t precision_coefficient, 
                                Temperature_Sensor_Data_Precision_Insignificant_Bytes_t precision_insignificant_bits, 
                                uint16_t precision_scaling_factor);
uint8_t OBC_TEMP_SENSOR__set_temp_precision(uint8_t arg_precision, uint32_t* temp_precision_conversion_delay, uint32_t* temp_scaling_factor);
uint8_t OBC_TEMP_SENSOR__configure_precision_values(uint8_t arg_precision, struct Set_Precision_Data* precision_data);
int8_t OBC_TEMP_SENSOR__get_temp_precision(void);

#endif