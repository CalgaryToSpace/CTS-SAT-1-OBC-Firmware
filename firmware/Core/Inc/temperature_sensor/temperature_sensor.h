#ifndef __INCLUDE_GUARD_TEMPERATURE_SENSOR_H__
#define __INCLUDE_GUARD_TEMPERATURE_SENSOR_H__

#include <stdint.h>

static const float TEMP_SENSOR_nine_bit_precision_coefficient = 2;
static const float TEMP_SENSOR_ten_bit_precision_coefficient = 4;
static const float TEMP_SENSOR_eleven_bit_precision_coefficient = 8;
static const float TEMP_SENSOR_twelve_bit_precision_coefficient = 16;

typedef enum  
{
    TEMP_SENSOR_TWELVE_BIT_PRECISION_INSIGNIFICANT_BYTES = 4,
    TEMP_SENSOR_ELEVEN_BIT_PRECISION_INSIGNIFICANT_BYTES,
    TEMP_SENSOR_TEN_BIT_PRECISION_INSIGNIFICANT_BYTES,
    TEMP_SENSOR_NINE_BIT_PRECISION_INSIGNIFICANT_BYTES,
} Temperature_Sensor_Data_Precision_Insignificant_Bytes_t;


uint8_t read_temperature(int32_t *result);
int32_t TEMP_SENSOR_convert_raw_to_deg_c(uint8_t raw_bytes[], uint8_t TEMP_SENSOR_data_precision_coefficient, 
                                    Temperature_Sensor_Data_Precision_Insignificant_Bytes_t TEMP_SENSOR_data_precision_insignificant_bits);
int8_t TEMP_SENSOR__get_temp_precision(void);
uint8_t TEMP_SENSOR__set_temp_precision(uint8_t arg_precision, uint8_t did_user_set_precision);


#endif