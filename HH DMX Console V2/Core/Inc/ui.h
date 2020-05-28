/*
 * ui.h
 *
 *  Created on: Apr 28, 2020
 *      Author: Kyle
 */

#ifndef INC_UI_H_
#define INC_UI_H_

#include "main.h"
#include "stdbool.h"

enum activeDevice
{
    UI_DEVICE_NONE,
    UI_DEVICE_KEYPAD,
    UI_DEVICE_OLED
};
enum
{
	OLED_RESET_INACTIVE,
	OLED_RESET_WAITING,
	OLED_RESET_ACTIVE,
	OLED_RESET_DONE
} OLEDResetState;

void UI_Init();
bool UI_SetActiveDevice(enum activeDevice device);
void UI_RequestKeypadRead();
void UI_ProcessQueue();
void UI_RequestOLEDWrite();
void UI_SPI_Callback();
void UI_TimerCallback();

#endif /* INC_UI_H_ */
