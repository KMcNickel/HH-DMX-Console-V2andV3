/*
 * powerMgmt.c
 *
 *  Created on: May 14, 2020
 *      Author: Kyle
 */

#include "stdint.h"
#include "stdbool.h"
#include "main.h"
#include "oled.h"
#include "powerMgmt.h"
#include "usb_iface.h"

extern ADC_HandleTypeDef hadc2;
extern TIM_HandleTypeDef htim15;
extern OPAMP_HandleTypeDef hopamp2;
								//Note that voltage will be lower while loaded down
#define ADCBAT_DIVIDEND 125
#define ADCBAT_MINVOLTAGE 21	//2.00V or less = 0 bars
#define ADCBAT_MAXVOLTAGE 29	//3.00V or more = 8 bars

uint32_t ADCValue = 0;
bool PwrWasReleased = 0;

enum powerStates
{
	POWER_STATE_NONE,
	POWER_STATE_USB,
	POWER_STATE_BATTERY
} curPowerState;
uint8_t curBatteryLevel;
uint8_t finalBatteryLevel;
uint8_t initCount;

void HAL_ADC_ConvCpltCallback (ADC_HandleTypeDef * hadc)
{
	HAL_ADC_Stop_IT(&hadc2);

	ADCValue = HAL_ADC_GetValue(&hadc2) / ADCBAT_DIVIDEND;

	if(finalBatteryLevel != 0) ADCValue = ((finalBatteryLevel * 2) + ADCValue) / 3;
	finalBatteryLevel = ADCValue;

	HAL_TIM_Base_Start_IT(&htim15);
}

void POWER_UpdateStatus(enum powerStates newPowerState, uint8_t newBatteryLevel)
{
	if(newPowerState == POWER_STATE_USB && curPowerState != POWER_STATE_USB)
	{
		OLED_DrawPowerSymbolPlug(117, 0);
		OLED_DrawScreen();
		curPowerState = newPowerState;
	}
	else
	{
		if(newBatteryLevel <= ADCBAT_MINVOLTAGE) newBatteryLevel = 0;
		else if(newBatteryLevel >= ADCBAT_MAXVOLTAGE) newBatteryLevel = 8;
		else newBatteryLevel = (newBatteryLevel - ADCBAT_MINVOLTAGE);
		if(newPowerState == POWER_STATE_BATTERY && (curPowerState != POWER_STATE_BATTERY
				|| newBatteryLevel != curBatteryLevel))
		{
			OLED_DrawPowerSymbolBattery(newBatteryLevel, 117, 0);
			OLED_DrawScreen();
			curBatteryLevel = newBatteryLevel;
			curPowerState = newPowerState;
		}
	}
}

void POWER_CheckStatus()
{
	HAL_TIM_Base_Stop_IT(&htim15);
	if(HAL_GPIO_ReadPin(GPIOA, USB_VCC_DETECT_Pin))
		POWER_UpdateStatus(POWER_STATE_USB, 0);
	else POWER_UpdateStatus(POWER_STATE_BATTERY, finalBatteryLevel);
	HAL_ADC_Start_IT(&hadc2);
}

void POWER_Init()
{
	HAL_OPAMP_Start(&hopamp2);
	HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
	HAL_TIM_Base_Start_IT(&htim15);
}

void POWER_Shutdown()
{
	HAL_GPIO_WritePin(GPIOA, PWRON_Pin, GPIO_PIN_RESET);
}

void POWER_CheckPowerButton()
{
	if(PwrWasReleased)
	{
		if(!HAL_GPIO_ReadPin(GPIOA, PBSTAT_Pin))
			POWER_Shutdown();
	}
	else
		if(HAL_GPIO_ReadPin(GPIOA, PBSTAT_Pin))
			PwrWasReleased = 1;
}
