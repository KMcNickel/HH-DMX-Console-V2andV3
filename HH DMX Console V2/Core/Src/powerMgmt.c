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
#define ADCBAT_DIVIDEND 32		//Min/Max have been adjusted as this will give 0.1V lower than correct
#define ADCBAT_MINVOLTAGE 69
#define ADCBAT_MAXVOLTAGE 84
#define ADCBAT_AVERAGINGCOUNT 100

extern uint8_t BL_ACT;
extern uint32_t BL_Count;

uint16_t ADCCount = 0;
uint32_t ADCValue = 0;

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
	POWER_DisplayBatteryStatus();
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
		else newBatteryLevel = ((newBatteryLevel - ADCBAT_MINVOLTAGE) + 1 ) / 2;
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
	if(HAL_GPIO_ReadPin(GPIOA, VSRC_Pin))
		POWER_UpdateStatus(POWER_STATE_BATTERY, finalBatteryLevel);
	else POWER_UpdateStatus(POWER_STATE_USB, 0);
	HAL_ADC_Start_IT(&hadc2);
}

void POWER_DisplayBatteryStatus()
{
	ADCCount++;
	ADCValue += HAL_ADC_GetValue(&hadc2);

	if(ADCCount >= ADCBAT_AVERAGINGCOUNT)
	{
		ADCValue = (ADCValue / (ADCCount * ADCBAT_DIVIDEND));

		if(finalBatteryLevel != 0) ADCValue = ((finalBatteryLevel * 2) + ADCValue) / 3;
		finalBatteryLevel = ADCValue;
		ADCCount = ADCValue = 0;
	}
}

void POWER_Init()
{
	HAL_OPAMP_Start(&hopamp2);
	HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
	HAL_TIM_Base_Start_IT(&htim15);
}
