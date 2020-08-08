/*
 * cli.h
 *
 *  Created on: Apr 28, 2020
 *      Author: Kyle
 */

#ifndef INC_CLI_H_
#define INC_CLI_H_

#define CLI_MAX_ITEMS 33
#define CLI_COMMAND_START 600
#define CLI_PRESET_COUNT 10
#define CLI_TICKS_PER_SECOND 128

#define SWITCH_VALID_CHANNELS 1 ... 512
#define SWITCH_VALID_RAW_VALUES 0 ... 255
#define SWITCH_VALID_PERCENT_VALUES 0 ... 100
#define SWITCH_VALID_PRESETS 1 ... CLI_PRESET_COUNT
#define SWITCH_VALID_TIME 0 ... 60

#define BtnThru 601
#define BtnAt 602
#define BtnFull 603
#define BtnEnter 604
#define BtnBksp 605
#define BtnClear 606
#define BtnPlus 607
#define BtnMinus 608
#define BtnLast 609
#define BtnNext 610
#define BtnRecord 611
#define BtnPreset 612
#define BtnOffset 613
#define BtnTime 614

void CLI_Init(uint8_t* dmxBuf);
void CLI_AddToCommand(uint16_t function);
void CLI_Timer_Callback();

#endif /* INC_CLI_H_ */
