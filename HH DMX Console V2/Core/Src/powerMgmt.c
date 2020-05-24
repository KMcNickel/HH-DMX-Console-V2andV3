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

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim15;
#define ADCBAT_DIVIDEND 17
#define ADCBAT_MINVOLTAGE 65
#define ADCBAT_RANGEMULTIPLIER 5
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

void HAL_ADC_ConvCpltCallback (ADC_HandleTypeDef * hadc)
{
	POWER_DisplayBatteryStatus();
	HAL_TIM_Base_Start_IT(&htim15);
}

void POWER_UpdateStatus(enum powerStates newPowerState, uint8_t newBatteryLevel)
{
	if(newPowerState != curPowerState || newBatteryLevel / 10 != curBatteryLevel / 10)
	{
		if(newPowerState == POWER_STATE_USB)
			OLED_DrawPowerSymbolPlug(117, 0);
		else OLED_DrawPowerSymbolBattery(newBatteryLevel, 117, 0);
		OLED_DrawScreen();
		curPowerState = newPowerState;
		curBatteryLevel = newBatteryLevel;
	}
}

void POWER_CheckStatus()
{
	HAL_TIM_Base_Stop_IT(&htim15);
	if(HAL_GPIO_ReadPin(GPIOA, VSRC_Pin))
		POWER_UpdateStatus(POWER_STATE_BATTERY, finalBatteryLevel);
	else POWER_UpdateStatus(POWER_STATE_USB, curBatteryLevel);
	HAL_ADC_Start_IT(&hadc1);
}

void POWER_DisplayBatteryStatus()
{
	HAL_ADC_Stop_IT(&hadc1);
	ADCCount++;
	ADCValue += HAL_ADC_GetValue(&hadc1)  / ADCBAT_DIVIDEND;

	if(ADCCount >= ADCBAT_AVERAGINGCOUNT)
	{
		ADCValue = ((ADCValue  / ADCCount) - ADCBAT_MINVOLTAGE) * ADCBAT_RANGEMULTIPLIER;
		if(curBatteryLevel != 0) ADCValue = ((curBatteryLevel * 2) + ADCValue) / 3;
		finalBatteryLevel = ADCValue;
		ADCCount = ADCValue = 0;
	}
}

void POWER_Init()
{
	HAL_TIM_Base_Start_IT(&htim15);
}
