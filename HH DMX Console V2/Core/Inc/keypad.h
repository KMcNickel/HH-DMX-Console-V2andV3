/*
 * keypad.h
 *
 *  Created on: Apr 28, 2020
 *      Author: Kyle
 */

#ifndef INC_KEYPAD_H_
#define INC_KEYPAD_H_

#include "main.h"
#include "stdbool.h"

#define DEBOUNCE_COUNT 11

void Keypad_Init();
void TMR16Callback ();
void Keypad_ProcessButtonPress();
void Keypad_SPICallback ();
void Keypad_ReadData();
void Keypad_TIM_PeriodElapsedCallback ();

#endif /* INC_KEYPAD_H_ */
