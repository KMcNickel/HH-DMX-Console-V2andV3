/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32g4xx_hal.h"
#include "stm32g4xx_ll_pwr.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
#define OLED_RST_Pin GPIO_PIN_1
#define OLED_RST_GPIO_Port GPIOA
#define OLED_DC_Pin GPIO_PIN_2
#define OLED_DC_GPIO_Port GPIOA
#define OLED_CS_Pin GPIO_PIN_3
#define OLED_CS_GPIO_Port GPIOA
#define KEYPAD_PL_Pin GPIO_PIN_4
#define KEYPAD_PL_GPIO_Port GPIOA
#define UI_SCK_Pin GPIO_PIN_5
#define UI_SCK_GPIO_Port GPIOA
#define UI_MISO_Pin GPIO_PIN_6
#define UI_MISO_GPIO_Port GPIOA
#define UI_MOSI_Pin GPIO_PIN_7
#define UI_MOSI_GPIO_Port GPIOA
#define BAT_DIV_Pin GPIO_PIN_0
#define BAT_DIV_GPIO_Port GPIOB
#define VSRC_Pin GPIO_PIN_8
#define VSRC_GPIO_Port GPIOA
#define USB_VCC_DETECT_Pin GPIO_PIN_9
#define USB_VCC_DETECT_GPIO_Port GPIOA
#define T_SWDIO_Pin GPIO_PIN_13
#define T_SWDIO_GPIO_Port GPIOA
#define T_SWCLK_Pin GPIO_PIN_14
#define T_SWCLK_GPIO_Port GPIOA
#define DMX_DIR_Pin GPIO_PIN_5
#define DMX_DIR_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
