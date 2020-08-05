/*
 * oled.h
 *
 *  Created on: Apr 28, 2020
 *      Author: Kyle
 */

#ifndef INC_OLED_H_
#define INC_OLED_H_

#include "main.h"
#include "stdbool.h"

#define SSD1305_ADDRESS 0x3C

#define SSD1305_SETLOWCOLUMN 0x00
#define SSD1305_SETHIGHCOLUMN 0x10
#define SSD1305_MEMORYMODE 0x20
#define SSD1305_SETCOLADDR 0x21
#define SSD1305_SETPAGEADDR 0x22
#define SSD1305_SETSTARTLINE 0x40

#define SSD1305_SETCONTRAST 0x81
#define SSD1305_SETBRIGHTNESS 0x82

#define SSD1305_SETLUT 0x91

#define SSD1305_SEGREMAP 0xA0
#define SSD1305_DISPLAYALLON_RESUME 0xA4
#define SSD1305_DISPLAYALLON 0xA5
#define SSD1305_NORMALDISPLAY 0xA6
#define SSD1305_INVERTDISPLAY 0xA7
#define SSD1305_SETMULTIPLEX 0xA8
#define SSD1305_DISPLAYDIM 0xAC
#define SSD1305_MASTERCONFIG 0xAD
#define SSD1305_DISPLAYOFF 0xAE
#define SSD1305_DISPLAYON 0xAF

#define SSD1305_SETPAGESTART 0xB0

#define SSD1305_COMSCANINC 0xC0
#define SSD1305_COMSCANDEC 0xC8
#define SSD1305_SETDISPLAYOFFSET 0xD3
#define SSD1305_SETDISPLAYCLOCKDIV 0xD5
#define SSD1305_SETAREACOLOR 0xD8
#define SSD1305_SETPRECHARGE 0xD9
#define SSD1305_SETCOMPINS 0xDA
#define SSD1305_SETVCOMLEVEL 0xDB


void OLED_Init();
void OLED_InitafterReset();
bool OLED_DrawScreen();
bool OLED_DrawPage(uint8_t page);
bool OLED_DrawArea(uint8_t page, uint8_t column, uint8_t count);
void OLED_Checkerboard();
void OLED_Blank();
void OLED_Fill();
void OLED_Char(unsigned char* character, unsigned char column, unsigned char page);
void OLED_CharASCII(char character, unsigned char column, unsigned char page);
void OLED_String(char* str, unsigned char len, unsigned char column, unsigned char page);
void OLED_StringAutoLine(char* str, unsigned char len, unsigned char column, unsigned char page,  unsigned char maxLines);
bool OLED_IsReady();
void OLED_ClearLine(unsigned char line);
void OLED_SPICallback();
void OLED_WriteData();
void OLED_DrawPowerSymbolPlug(unsigned char column, unsigned char page);
void OLED_DrawPowerSymbolBattery(uint8_t bars, unsigned char column, unsigned char page);


#endif /* INC_OLED_H_ */
