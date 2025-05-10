#include "obc_systems/adc_vbat_monitor.h"

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_adc.h"
#include "main.h"


/// @brief Read the voltage reported by the ADC at the VBAT_MONITOR pin.
/// @return The voltage in mV. -9999 if error.
/// @note This does not apply the voltage divider factor. The voltage divider is 11:1, so the actual battery voltage is 11 times this value.
static double OBC_read_vbat_monitor_with_adc_mV() {
    const HAL_StatusTypeDef start_result = HAL_ADC_Start(&hadc1);
    if (start_result != HAL_OK) {
        return -9999;
    }

    const HAL_StatusTypeDef poll_result = HAL_ADC_PollForConversion(&hadc1, 1000);
    if (poll_result != HAL_OK) {
        return -9999;
    }

    const uint32_t raw_value = HAL_ADC_GetValue(&hadc1);

    // Ratio here: * 3300mV / 4095 = 0.80566mV per ADC step.
    const double vbat_monitor_mV = ((double)raw_value) * 0.80566;

    return vbat_monitor_mV;
}

/// @brief Read the battery voltage using the ADC.
/// @return The battery voltage in mV. -9999 if error.
/// @note This applies the voltage divider factor (11:1) to the voltage read from the ADC.
int16_t OBC_read_vbat_with_adc_mV() {
    const double vbat_monitor_mV = OBC_read_vbat_monitor_with_adc_mV();

    if (vbat_monitor_mV == -9999) {
        return -9999;
    }

    // Apply the voltage divider factor (11:1).
    // Voltage divider is (1k on bottom, 10k on top).
    const int16_t vbat_mV = vbat_monitor_mV * 11.0;
    return vbat_mV;
}
