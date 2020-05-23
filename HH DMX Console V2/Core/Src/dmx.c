/*
 * dmx.c
 *
 *  Created on: Apr 28, 2020
 *      Author: Kyle
 */

#include "dmx.h"
#include "main.h"
#include "stdbool.h"

uint8_t __attribute__ ((aligned(32))) TXArray[513];

extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;

void SetTXPin(bool brk)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = brk ? GPIO_MODE_OUTPUT_PP : GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void HAL_UART_TxCpltCallback (UART_HandleTypeDef * huart)
{
	SetTXPin(true);
	HAL_TIM_Base_Start_IT(&htim6);
}

void DMX_TIM_PeriodElapsedCallback (TIM_HandleTypeDef * htim)
{
	if(htim == &htim6)
	{
		SetTXPin(false);
		HAL_TIM_Base_Start_IT(&htim7);
		HAL_TIM_Base_Stop_IT(&htim6);
	}
	if(htim == &htim7)
	{
		HAL_UART_Transmit_DMA(&huart1, TXArray, 513);
		HAL_TIM_Base_Stop_IT(&htim7);
	}
}

void DMX_TransitionToMAB()
{
	SetTXPin(false);
	HAL_TIM_Base_Start_IT(&htim7);
	HAL_TIM_Base_Stop_IT(&htim6);
}

void DMX_TransitionToData()
{
	HAL_UART_Transmit_DMA(&huart1, TXArray, 4);
	HAL_TIM_Base_Stop_IT(&htim7);
}

void DMX_Init()
{
	  for(int i = 1; i < 513; i++)
	  {
		  TXArray[i] = 0x00;
	  }

	  HAL_GPIO_WritePin(GPIOB, DMX_DIR_Pin, GPIO_PIN_SET);
	  HAL_TIM_Base_Start_IT(&htim7);
}
