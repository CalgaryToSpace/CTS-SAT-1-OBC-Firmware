#include "telecommands/obc_systems_telecommand_defs.h"
#include "obc_systems/obc_temperature_sensor.h"
#include "obc_systems/adc_vbat_monitor.h"
#include "debug_tools/debug_uart.h"

#include "stm32l4xx_hal_rcc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/// @brief  Reads the temperature from the STDS75DS2F and stores it in the provided variable temperature.
///         Temperature range is -55 to 125 degrees celsius with +/- 3 degrees celsius accuracy over the whole range.
/// @param args_str
/// - Arg 0: Precision we want the temperature to be (9-12 bits).
/// @return 0 if successful, 1 if error.
/// @note There are better ways to get the temperature.
uint8_t TCMDEXEC_obc_read_temperature_complex(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    int32_t temperature;
    uint32_t temp_precision_conversion_delay_ms;
    uint32_t temp_scaling_factor;

    // Convert user requested precision.
    const uint8_t temp_precision = atoi(args_str);

    const uint8_t status = OBC_TEMP_SENSOR_set_temp_precision(
        temp_precision, &temp_precision_conversion_delay_ms, &temp_scaling_factor
    );
    switch (status)
    {
        case 0:
            break;
        case 1:
        {
            snprintf(response_output_buf, response_output_buf_len, "Error writing config register.");
            return 1;
        }
        case 2:
        {
            snprintf(
                response_output_buf, response_output_buf_len,
                "Invalid temperature precision provided. Arg must be in range 9-12."
            );
            return 2;
        }
    }

    // Give time for sensor to convert value.
    HAL_Delay(temp_precision_conversion_delay_ms);

    // Read temperature.
    uint8_t success_result = OBC_TEMP_SENSOR_read_temperature(&temperature);

    // Handle result.
    if (success_result != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Temperature read fail!");
        return 3;
    }

    snprintf(
        response_output_buf, response_output_buf_len,
        "Temperature(C * %lu): %ld",
        temp_scaling_factor,
        temperature
    );

    return 0;
}

/// @brief Reads the temperature from the STDS75DS2F in centi-Celsius.
/// @return 0 if successful, >0 if error.
/// @note Temperature range is -55 to 125 degrees celsius with +/- 3 degrees celsius accuracy over the whole range.
uint8_t TCMDEXEC_obc_read_temperature(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    const int32_t temp_cC = OBC_TEMP_SENSOR_get_temperature_cC();
    if (temp_cC == OBC_TEMP_SENSOR_ERROR_TEMPERATURE_CC) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "Temperature read fail! Temperature: %" PRIi32 " cC",
            temp_cC
        );
        return 1;
    }

    snprintf(
        response_output_buf, response_output_buf_len,
        "OBC Temperature: %" PRIi32 " cC",
        temp_cC
    );

    return 0;
}

/// @brief Read the battery voltage using the ADC. Returns the result in mV. Voltage divider ratio is already applied.
/// @param args_str No arguments.
/// @return 
uint8_t TCMDEXEC_obc_adc_read_vbat_voltage(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    // Read the voltage from the ADC.
    const int16_t vbat_mV = OBC_read_vbat_with_adc_mV();

    if (vbat_mV == -9999) {
        snprintf(response_output_buf, response_output_buf_len, "VBAT ADC read error.");
        return 1;
    }
    
    snprintf(response_output_buf, response_output_buf_len, "{\"vbat_voltage_mV\":%d}", vbat_mV);
    return 0;
}

/// @brief This is a version of the `SystemClock_Config` function in main.c, generated with the HSE SYSCLK config.
static uint8_t SystemClock_Config_with_hse(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  uint8_t err;

  /** Configure the main internal regulator output voltage
  */
  if ((err=HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1)) != HAL_OK)
  {
    return 10 + err;
  }

  DEBUG_uart_print_str("HAL_PWREx_ControlVoltageScaling() successful.\n");
  HAL_Delay(500);
  DEBUG_uart_print_str("Waited.\n");

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI
                              |RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if ((err=HAL_RCC_OscConfig(&RCC_OscInitStruct)) != HAL_OK)
  {
    return 20 + err;
  }

  DEBUG_uart_print_str("HAL_RCC_OscConfig() successful.\n");
  HAL_Delay(500);
  DEBUG_uart_print_str("Waited.\n");

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if ((err=HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1)) != HAL_OK)
  {
    return 30 + err;
  }

  DEBUG_uart_print_str("HAL_RCC_ClockConfig() successful.\n");
  HAL_Delay(500);
  DEBUG_uart_print_str("Waited.\n");

  return 0; // Success.
}

/// @brief Change the STM32 SYSCLK clock from the HSI to the HSE.
/// @param args_str No arguments.
/// @return 0 on success.
/// @details On bootup, the OBC will use the HSI as its clock, as the HSE may have a PCB design
///         defect, where the output capacitance of the HSE clock is too high.
///         Running this command will switch the SYSCLK from the High-Speed Internal (HSI) at 16 MHz
///         to the probably-fine-but-maybe-questionable High-Speed External (HSE) clock at 25 MHz.
///         The main benefit is that the HSE clock is a Temperature-Compensated Crystal Oscillator (TXCO),
///         and thus maintains the system clock much more reliably.
/// @note To switch back to the HSI, simply reboot.
/// @note If the HSE fails, the Independent Watchdog will reset. Thus, this reconfiguration is low-risk.
uint8_t TCMDEXEC_obc_set_stm32_sysclk_to_hse(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    const uint32_t SystemCoreClock_at_start = SystemCoreClock;

    if (HSE_VALUE != 25000000) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "HSE_VALUE must be 25 MHz, configured by the IOC file!"
        );
        return 1;
    }

    HAL_Delay(500); // Wait so we don't pet the watchdog right away.

    const uint8_t config_err = SystemClock_Config_with_hse();
    if (config_err != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "SystemClock_Config_with_hse() failed - error %d",
            config_err
        );
        return config_err;
    }

    HAL_Delay(500);

    // Configure the source of time base considering new system clocks settings.
    SystemCoreClockUpdate();

    DEBUG_uart_print_str("SystemCoreClockUpdate() ran.\n");
    const HAL_StatusTypeDef init_tick_status = HAL_InitTick(uwTickPrio);
    if(init_tick_status != HAL_OK) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "HAL_InitTick() failed - HAL error %d",
            init_tick_status
        );
        return 60 + init_tick_status;
    }

    // Reconfigure SysTick for the new clock speed.
    // Critical to make osDelay() work at the new rate (related to HAL_TIM_PeriodElapsedCallback).
    const uint32_t err = HAL_SYSTICK_Config(SystemCoreClock / configTICK_RATE_HZ);
    if (err != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "HAL_SYSTICK_Config() failed - error %ld",
            err
        );
        return 70 + err;
    }

    snprintf(
        response_output_buf, response_output_buf_len,
        "Clock update successful. SystemCoreClock (Hz): %ld -> %ld",
        SystemCoreClock_at_start,
        SystemCoreClock
    );
    
    return 0; // Success.
}
