/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
extern SPI_HandleTypeDef hspi1;
extern ADC_HandleTypeDef hadc3;
extern SDRAM_HandleTypeDef hsdram1;  // For NAND Flash FMC switching
extern NAND_HandleTypeDef hnand1;    // For NAND Flash operations

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RADAR_SPI_SCLK_Pin GPIO_PIN_2
#define RADAR_SPI_SCLK_GPIO_Port GPIOE
#define RADAR_SS_Pin GPIO_PIN_3
#define RADAR_SS_GPIO_Port GPIOE
#define RADAR_SS_EXTI_IRQn EXTI3_IRQn
#define RADAR_INTG_CLK_Pin GPIO_PIN_4
#define RADAR_INTG_CLK_GPIO_Port GPIOE
#define RADAR_INTG_CLK_EXTI_IRQn EXTI4_IRQn
#define RADAR_SPI_MISO_Pin GPIO_PIN_5
#define RADAR_SPI_MISO_GPIO_Port GPIOE
#define RADAR_SPI_MOSI_Pin GPIO_PIN_6
#define RADAR_SPI_MOSI_GPIO_Port GPIOE
#define FMC_nWP_Pin GPIO_PIN_13
#define FMC_nWP_GPIO_Port GPIOC
#define SPI_CLK_ESP32_Pin GPIO_PIN_5
#define SPI_CLK_ESP32_GPIO_Port GPIOA
#define SPI_MISO_ESP32_Pin GPIO_PIN_6
#define SPI_MISO_ESP32_GPIO_Port GPIOA
#define SPI_MOSI_ESP32_Pin GPIO_PIN_7
#define SPI_MOSI_ESP32_GPIO_Port GPIOA
#define SPI1_CS_Pin GPIO_PIN_8
#define SPI1_CS_GPIO_Port GPIOA
#define USB_OTG_DM_Pin GPIO_PIN_11
#define USB_OTG_DM_GPIO_Port GPIOA
#define USB_OTG_DP_Pin GPIO_PIN_12
#define USB_OTG_DP_GPIO_Port GPIOA
#define RADAR_SPI_nCS_Pin GPIO_PIN_5
#define RADAR_SPI_nCS_GPIO_Port GPIOB
#define RADAR_LED_Pin GPIO_PIN_6
#define RADAR_LED_GPIO_Port GPIOB
#define RADAR_SPI_RST_Pin GPIO_PIN_7
#define RADAR_SPI_RST_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
