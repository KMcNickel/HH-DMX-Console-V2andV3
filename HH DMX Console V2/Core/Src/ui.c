/*
 * ui.c
 *
 *  Created on: Apr 28, 2020
 *      Author: Kyle
 */

#include "ui.h"
#include "oled.h"
#include "keypad.h"
#include "stdbool.h"
#include "main.h"

enum activeDevice curActiveDevice;
bool KeypadWaiting;
bool OLEDWaiting;
bool EEPROMWaiting;
extern TIM_HandleTypeDef htim16;

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	UI_SPI_Callback();
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	UI_SPI_Callback();
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	UI_SPI_Callback();
}

void UI_SPI_Callback()
{
    switch(curActiveDevice)
    {
        case UI_DEVICE_KEYPAD:
            Keypad_SPICallback();
            curActiveDevice = UI_DEVICE_NONE;
            break;
        case UI_DEVICE_OLED:
            OLED_SPICallback();
            curActiveDevice = UI_DEVICE_NONE;
            break;
        case UI_DEVICE_EEPROM:
            if(!EEPROM_SPICallback())	//Function returns true if EEPROM is still using the SPI bus
            	curActiveDevice = UI_DEVICE_NONE;
            break;
        default:
            break;
    }
}

void UI_RequestKeypadRead()
{
    KeypadWaiting = true;
}

void UI_RequestOLEDWrite()
{
    OLEDWaiting = true;
}

void UI_RequestEEPROMReadWrite()
{
    EEPROMWaiting = true;
}

void UI_TimerCallback()
{
	Keypad_TIM_PeriodElapsedCallback();
	if(OLEDResetState == OLED_RESET_WAITING)
		OLEDResetState = OLED_RESET_ACTIVE;
	if(OLEDResetState == OLED_RESET_ACTIVE)
		OLED_InitafterReset();
}

void UI_ProcessQueue()
{
    if(curActiveDevice != UI_DEVICE_NONE)
        return;
    if(KeypadWaiting)
    {
        curActiveDevice = UI_DEVICE_KEYPAD;
        KeypadWaiting = false;
        Keypad_ReadData();
    }
    else if(OLEDWaiting)
    {
        curActiveDevice = UI_DEVICE_OLED;
        OLEDWaiting = false;
        OLED_WriteData();
    }
    else if(EEPROMWaiting)
    {
        curActiveDevice = UI_DEVICE_EEPROM;
        EEPROMWaiting = false;
        EEPROM_SendReadWriteData();
    }
}

void UI_Init()
{
    //HAL_TIM_Base_Start_IT(&htim16);
    //OLED_Init();
    //Keypad_Init();
    EEPROM_Init();
}
