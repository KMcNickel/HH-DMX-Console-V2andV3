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

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	UI_SPI_Callback();
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	UI_SPI_Callback();
}

void UI_SPI_Callback()
{
    switch(curActiveDevice)
    {
        case UI_DEVICE_KEYPAD:
            Keypad_SPICallback();
            break;
        case UI_DEVICE_OLED:
            OLED_SPICallback();
            break;
        default:
            break;
    }
    curActiveDevice = UI_DEVICE_NONE;

}

void UI_RequestKeypadRead()
{
    KeypadWaiting = true;
}

void UI_RequestOLEDWrite()
{
    OLEDWaiting = true;
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
}

void UI_Init()
{
    OLED_Init();
    Keypad_Init();
}
