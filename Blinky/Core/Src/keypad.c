/*
 * keypad.c
 *
 *  Created on: Apr 28, 2020
 *      Author: Kyle
 */

#include "keypad.h"
#include "stdint.h"
#include "stdbool.h"
#include "main.h"
#include "ui.h"
#include "cli.h"

enum ButtonState
{
    KEYPAD_BUTTON_RELEASED,
    KEYPAD_BUTTON_PRESSED,
    KEYPAD_BUTTON_ACTIVATED,
    KEYPAD_BUTTON_PROCESSED
};

enum ButtonState buttonStates[24];
uint8_t buttonCounter[24];
uint8_t rawKeypadData[3];
extern TIM_HandleTypeDef htim16;
extern SPI_HandleTypeDef hspi1;

uint16_t keypadMapping[] =
{
	BtnNext, BtnLast, BtnPlus, BtnMinus, BtnTime, BtnOffset, BtnRecord, BtnPreset,  // 1A - H
	BtnAt, 6, 5, 4, BtnThru, 9, 8, 7,                             					// 2A - H
	BtnEnter, BtnBksp, 0, BtnClear, BtnFull, 3, 2, 1                    			// 3A - H
};

void TMR16Callback ()
{
    HAL_TIM_Base_Stop_IT(&htim16);
    UI_RequestKeypadRead();
}

void Keypad_ReadData()
{
	HAL_GPIO_WritePin(GPIOA, KEYPAD_PL_Pin, GPIO_PIN_SET);
	HAL_SPI_Receive_IT(&hspi1, rawKeypadData, 3);
}

void Keypad_SPICallback ()
{
    uint8_t i;

    HAL_GPIO_WritePin(GPIOA, KEYPAD_PL_Pin, GPIO_PIN_RESET);

    for(uint8_t j = 0; j < 3; j++)
    {
    	for(i = 0; i < 8; i++)
		{
    		uint8_t buttonNum = (j * 8) + i;
			if(((rawKeypadData[j] >> i) & 1) == 0 &&
					buttonStates[buttonNum] != KEYPAD_BUTTON_ACTIVATED &&
					buttonStates[buttonNum] != KEYPAD_BUTTON_PROCESSED)
			{
				buttonStates[buttonNum] = KEYPAD_BUTTON_PRESSED;
				buttonCounter[buttonNum]++;
			}
			else if(((rawKeypadData[j] >> i) & 1) == 1)
			{
				buttonStates[buttonNum] = KEYPAD_BUTTON_RELEASED;
				buttonCounter[buttonNum] = 0;
			}
			if(buttonCounter[buttonNum] == DEBOUNCE_COUNT)
			{
				buttonStates[buttonNum] = KEYPAD_BUTTON_ACTIVATED;
				buttonCounter[buttonNum] = 0;
			}
		}
    }


    HAL_TIM_Base_Start_IT(&htim16);
}

void Keypad_ProcessButtonPress()
{
    uint8_t i;
    for(i = 0; i < 24; i++)
    {
        if(buttonStates[i] == KEYPAD_BUTTON_ACTIVATED)
        {
            CLI_AddToCommand(keypadMapping[i]);
            buttonStates[i] = KEYPAD_BUTTON_PROCESSED;
            break;
        }
    }
}

void Keypad_Init()
{
    uint8_t i;
    for(uint8_t j = 0; j < 3; j++)
	{
		for(i = 0; i < 8; i++)
		{
			buttonStates[(j * 8) + i] = KEYPAD_BUTTON_RELEASED;
			buttonCounter[(j * 8) + i] = 0;
		}
	}

    HAL_GPIO_WritePin(GPIOA, KEYPAD_PL_Pin, GPIO_PIN_RESET);
    HAL_TIM_Base_Start_IT(&htim16);
}
