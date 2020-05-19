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

extern ADC_HandleTypeDef hadc2;
#define ADCBAT_DIVIDEND 29
#define ADCBAT_MINVOLTAGE 65
#define ADCBAT_RANGEMULTIPLIER 4
#define ADCBAT_AVERAGINGCOUNT 200

uint8_t ADCCount = 0;
uint32_t ADCValue = 0;

void HAL_ADC_ConvCpltCallback (ADC_HandleTypeDef * hadc)
{
	POWER_DisplayBatteryStatus();
}

void POWER_CheckStatus()
{
	if(HAL_GPIO_ReadPin(GPIOA, VSRC_Pin))
	{
		HAL_ADC_Start_IT(&hadc2);
	}
	else
	{
		OLED_DrawPowerSymbolPlug(117, 0);
		OLED_DrawScreen();
		ADCCount = ADCValue = 0;
	}
}

void POWER_DisplayBatteryStatus()
{
	HAL_ADC_Stop_IT(&hadc2);
	ADCCount++;
	ADCValue += HAL_ADC_GetValue(&hadc2)  / ADCBAT_DIVIDEND;

	if(ADCCount == ADCBAT_AVERAGINGCOUNT)
	{
		ADCValue = ((ADCValue  / ADCCount) - ADCBAT_MINVOLTAGE) * ADCBAT_RANGEMULTIPLIER;
		OLED_DrawPowerSymbolBattery(ADCValue, 117, 0);
		OLED_DrawScreen();
		ADCCount = ADCValue = 0;
	}
}
