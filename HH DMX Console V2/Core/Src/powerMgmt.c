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
#define ADCBAT_DIVIDEND 29
#define ADCBAT_MINVOLTAGE 65
#define ADCBAT_RANGEMULTIPLIER 4
#define ADCBAT_AVERAGINGCOUNT 200

extern uint8_t BL_ACT;
extern uint32_t BL_Count;

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
		HAL_ADC_Start_IT(&hadc1);
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
	HAL_ADC_Stop_IT(&hadc1);
	ADCCount++;
	ADCValue += HAL_ADC_GetValue(&hadc1)  / ADCBAT_DIVIDEND;

	if(ADCCount == ADCBAT_AVERAGINGCOUNT)
	{
		ADCValue = ((ADCValue  / ADCCount) - ADCBAT_MINVOLTAGE) * ADCBAT_RANGEMULTIPLIER;
		OLED_DrawPowerSymbolBattery(ADCValue, 117, 0);
		OLED_DrawScreen();
		ADCCount = ADCValue = 0;
	}
}
