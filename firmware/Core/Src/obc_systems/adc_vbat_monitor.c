#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_adc.h"
#include "main.h"



double return_VBAT_Voltage(){

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 1000);
    double v_raw =  (HAL_ADC_GetValue(&hadc1) *3.3) / 4095.0; 

    return v_raw * 1000;
}

float voltage_VBAT(float voltage){ return voltage * 11;}