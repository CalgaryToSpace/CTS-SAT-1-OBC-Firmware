/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "rtos_tasks/rtos_task_helpers.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

// These externs expose peripherals for use across the project.
extern ADC_HandleTypeDef hadc1;
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c3;
extern I2C_HandleTypeDef hi2c4;

extern UART_HandleTypeDef hlpuart1;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

extern SPI_HandleTypeDef hspi1;

extern TIM_HandleTypeDef htim16;



/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

extern FREERTOS_task_info_struct_t FREERTOS_task_handles_array [];

extern const uint32_t FREERTOS_task_handles_array_size;

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PIN_LED_GP1_OUT_Pin GPIO_PIN_2
#define PIN_LED_GP1_OUT_GPIO_Port GPIOE
#define PIN_LED_GP2_OUT_Pin GPIO_PIN_3
#define PIN_LED_GP2_OUT_GPIO_Port GPIOE
#define PIN_LED_GP3_OUT_Pin GPIO_PIN_4
#define PIN_LED_GP3_OUT_GPIO_Port GPIOE
#define PIN_PC14_OSC32_IN_Pin GPIO_PIN_14
#define PIN_PC14_OSC32_IN_GPIO_Port GPIOC
#define PIN_ANTENNA_A_I2C2_SDA_Pin GPIO_PIN_0
#define PIN_ANTENNA_A_I2C2_SDA_GPIO_Port GPIOF
#define PIN_ANTENNA_A_I2C2_SCL_Pin GPIO_PIN_1
#define PIN_ANTENNA_A_I2C2_SCL_GPIO_Port GPIOF
#define PIN_PH0_OSC_IN_Pin GPIO_PIN_0
#define PIN_PH0_OSC_IN_GPIO_Port GPIOH
#define PIN_ANTENNA_B_I2C3_SCL_Pin GPIO_PIN_0
#define PIN_ANTENNA_B_I2C3_SCL_GPIO_Port GPIOC
#define PIN_ANTENNA_B_I2C3_SDA_Pin GPIO_PIN_1
#define PIN_ANTENNA_B_I2C3_SDA_GPIO_Port GPIOC
#define PIN_ADC_READ_VBAT_VOLTAGE_Pin GPIO_PIN_2
#define PIN_ADC_READ_VBAT_VOLTAGE_GPIO_Port GPIOC
#define PIN_CAMERA_MOSI_UART4_TX_Pin GPIO_PIN_0
#define PIN_CAMERA_MOSI_UART4_TX_GPIO_Port GPIOA
#define PIN_CAMERA_MISO_UART4_RX_Pin GPIO_PIN_1
#define PIN_CAMERA_MISO_UART4_RX_GPIO_Port GPIOA
#define PIN_CAM_EN_OUT_Pin GPIO_PIN_2
#define PIN_CAM_EN_OUT_GPIO_Port GPIOA
#define PIN_BOOM_DEPLOY_EN_1_OUT_Pin GPIO_PIN_3
#define PIN_BOOM_DEPLOY_EN_1_OUT_GPIO_Port GPIOA
#define PIN_BOOM_DEPLOY_EN_2_OUT_Pin GPIO_PIN_4
#define PIN_BOOM_DEPLOY_EN_2_OUT_GPIO_Port GPIOA
#define PIN_GNSS_PPS_IN_Pin GPIO_PIN_6
#define PIN_GNSS_PPS_IN_GPIO_Port GPIOA
#define PIN_GNSS_MOSI_USART3_TX_Pin GPIO_PIN_4
#define PIN_GNSS_MOSI_USART3_TX_GPIO_Port GPIOC
#define PIN_GNSS_MISO_USART3_RX_Pin GPIO_PIN_5
#define PIN_GNSS_MISO_USART3_RX_GPIO_Port GPIOC
#define PIN_MEM_NCS_FLASH_3_Pin GPIO_PIN_15
#define PIN_MEM_NCS_FLASH_3_GPIO_Port GPIOF
#define PIN_MEM_NCS_FLASH_2_Pin GPIO_PIN_0
#define PIN_MEM_NCS_FLASH_2_GPIO_Port GPIOG
#define PIN_MEM_NCS_FLASH_1_Pin GPIO_PIN_1
#define PIN_MEM_NCS_FLASH_1_GPIO_Port GPIOG
#define PIN_MEM_NCS_FLASH_0_Pin GPIO_PIN_7
#define PIN_MEM_NCS_FLASH_0_GPIO_Port GPIOE
#define PIN_MEM_NCS_FRAM_1_Pin GPIO_PIN_8
#define PIN_MEM_NCS_FRAM_1_GPIO_Port GPIOE
#define PIN_MEM_NCS_FRAM_0_Pin GPIO_PIN_9
#define PIN_MEM_NCS_FRAM_0_GPIO_Port GPIOE
#define PIN_MEM_SPI1_SCLK_Pin GPIO_PIN_13
#define PIN_MEM_SPI1_SCLK_GPIO_Port GPIOE
#define PIN_MEM_SPI1_MISO_Pin GPIO_PIN_14
#define PIN_MEM_SPI1_MISO_GPIO_Port GPIOE
#define PIN_MEM_SPI1_MOSI_Pin GPIO_PIN_15
#define PIN_MEM_SPI1_MOSI_GPIO_Port GPIOE
#define PIN_UHF_CTRL_OUT_Pin GPIO_PIN_12
#define PIN_UHF_CTRL_OUT_GPIO_Port GPIOB
#define PIN_REMOVE_BEFORE_FLIGHT_LOW_IS_FLYING_IN_Pin GPIO_PIN_14
#define PIN_REMOVE_BEFORE_FLIGHT_LOW_IS_FLYING_IN_GPIO_Port GPIOB
#define PIN_TEMP_SENSOR_I2C4_SCL_Pin GPIO_PIN_12
#define PIN_TEMP_SENSOR_I2C4_SCL_GPIO_Port GPIOD
#define PIN_TEMP_SENSOR_I2C4_SDA_Pin GPIO_PIN_13
#define PIN_TEMP_SENSOR_I2C4_SDA_GPIO_Port GPIOD
#define PIN_MPI_NEN_RX_MISO_OUT_Pin GPIO_PIN_4
#define PIN_MPI_NEN_RX_MISO_OUT_GPIO_Port GPIOG
#define PIN_MPI_EN_TX_MOSI_OUT_Pin GPIO_PIN_5
#define PIN_MPI_EN_TX_MOSI_OUT_GPIO_Port GPIOG
#define PIN_DEBUG_MOSI_LPUART1_TX_Pin GPIO_PIN_7
#define PIN_DEBUG_MOSI_LPUART1_TX_GPIO_Port GPIOG
#define PIN_DEBUG_MISO_LPUART1_RX_Pin GPIO_PIN_8
#define PIN_DEBUG_MISO_LPUART1_RX_GPIO_Port GPIOG
#define PIN_MPI_MOSI_USART1_TX_Pin GPIO_PIN_9
#define PIN_MPI_MOSI_USART1_TX_GPIO_Port GPIOA
#define PIN_MPI_MISO_USART1_RX_Pin GPIO_PIN_10
#define PIN_MPI_MISO_USART1_RX_GPIO_Port GPIOA
#define PIN_SWDIO_Pin GPIO_PIN_13
#define PIN_SWDIO_GPIO_Port GPIOA
#define PIN_SWCLK_Pin GPIO_PIN_14
#define PIN_SWCLK_GPIO_Port GPIOA
#define PIN_EPS_MOSI_UART_TX_Pin GPIO_PIN_12
#define PIN_EPS_MOSI_UART_TX_GPIO_Port GPIOC
#define PIN_EPS_MISO_UART_RX_Pin GPIO_PIN_2
#define PIN_EPS_MISO_UART_RX_GPIO_Port GPIOD
#define PIN_AX100_MISO_USART2_RX_Pin GPIO_PIN_5
#define PIN_AX100_MISO_USART2_RX_GPIO_Port GPIOD
#define PIN_AX100_MOSI_USART2_TX_Pin GPIO_PIN_6
#define PIN_AX100_MOSI_USART2_TX_GPIO_Port GPIOD
#define PIN_STACK_I2C1_SDA_Pin GPIO_PIN_13
#define PIN_STACK_I2C1_SDA_GPIO_Port GPIOG
#define PIN_STACK_I2C1_SCL_Pin GPIO_PIN_14
#define PIN_STACK_I2C1_SCL_GPIO_Port GPIOG
#define PIN_SWO_Pin GPIO_PIN_3
#define PIN_SWO_GPIO_Port GPIOB
#define PIN_LED_DEVKIT_LD2_Pin GPIO_PIN_7
#define PIN_LED_DEVKIT_LD2_GPIO_Port GPIOB
#define PIN_BOOT0_Pin GPIO_PIN_3
#define PIN_BOOT0_GPIO_Port GPIOH

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
