/*
 * oled.c
 *
 *  Created on: Apr 28, 2020
 *      Author: Kyle
 */

#include "oled.h"
#include "stdint.h"
#include "stdbool.h"
#include "main.h"
#include "ui.h"
#include "font_table.h"


enum OLED_States
{
    OLED_STATE_Idle,
    OLED_STATE_InitList,
    OLED_STATE_DrawScreen,
    OLED_STATE_DispOn,
    OLED_STATE_Waiting
};

enum OLED_DrawStates
{
    OLED_STATE_DrawIdle,
    OLED_STATE_SetParams,
    OLED_STATE_Data,
    OLED_STATE_DrawWaiting
};

enum OLED_ScreenUpdateTypes
{
	OLED_UPDATE_EntireScreen,
	OLED_UPDATE_Page,
	OLED_UPDATE_Area
};

#define OLED_PixelCount 128*4
unsigned char OLED_Buffer[OLED_PixelCount];
enum OLED_States OLED_Sys_State;
enum OLED_DrawStates OLED_Draw_State;
bool initialized;

uint8_t startPage;
uint8_t startCol;
uint8_t colCount;
enum OLED_ScreenUpdateTypes updateCommand;


extern SPI_HandleTypeDef hspi1;


void OLED_CommandArray_Write(unsigned char * cmd, uint16_t size)
{
	HAL_GPIO_WritePin(GPIOA, OLED_DC_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, OLED_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit_DMA(&hspi1, cmd, size);
}

void OLED_Command_Write(unsigned char cmd)
{
    OLED_CommandArray_Write(&cmd, 1);
}

void OLED_DataArray_Write(unsigned char * data, uint16_t size)
{
	HAL_GPIO_WritePin(GPIOA, OLED_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, OLED_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit_DMA(&hspi1, data, size);
}

void OLED_Data_Write(unsigned char data)
{
    OLED_DataArray_Write(&data, 1);
}

unsigned char initList[] =
{
    SSD1305_DISPLAYOFF,
    SSD1305_SETDISPLAYCLOCKDIV,
    0x10,
    SSD1305_SETMULTIPLEX,
    0x1F,
    SSD1305_SETDISPLAYOFFSET,
    0x00,
    SSD1305_SETSTARTLINE | 0x0,
    SSD1305_MASTERCONFIG,
    0x8E,
    SSD1305_SETAREACOLOR,
    0x05,
    SSD1305_MEMORYMODE,
    0x00,						//Horizontal Addressing Mode
    SSD1305_SEGREMAP,
    SSD1305_COMSCANINC,
    SSD1305_SETCOMPINS,
    0x12,
    SSD1305_SETLUT,
    0x3F,
    0x3F,
    0x3F,
    0x3F,
    SSD1305_SETCONTRAST,
    0xBF,
    SSD1305_SETBRIGHTNESS,
    0xBF,
    SSD1305_SETPRECHARGE,
    0xD2,
    SSD1305_SETVCOMLEVEL,
    0x08,
    SSD1305_DISPLAYALLON_RESUME,
    SSD1305_NORMALDISPLAY
};

unsigned char drawList[] =
{
    SSD1305_SETPAGEADDR,
	0x00,
	0x03,
    SSD1305_SETCOLADDR,
	0x00,
	0x7F
};

void OLED_Checkerboard()
{
    uint16_t i;
    for(i = 0; i < OLED_PixelCount; i++)
    {
        if(i%2)
            OLED_Buffer[i] = 0x55;
        else
            OLED_Buffer[i] = 0xAA;
    }
}

unsigned char* GetCharFromASCII(unsigned char ascii)
{
    if(ascii >= 0x20 && ascii <= 0x7D)
        return font_table[ascii - 0x20];
    else return no_char;
}

void OLED_Char(unsigned char* character, unsigned char column, unsigned char page)
{
    unsigned char i;
    for(i = 0; i < 5; i++)
        OLED_Buffer[i + (column + (page * 128))] = *(character + i);
    OLED_Buffer[5 + (column + (page * 128))] = 0x00;
}

void OLED_CharASCII(char character, unsigned char column, unsigned char page)
{
    OLED_Char(GetCharFromASCII(character), column, page);
}

void OLED_String(char* str, unsigned char len, unsigned char column, unsigned char page)
{
    char* follower = str;
    while(follower != str + len)
    {
        OLED_CharASCII(*follower, column, page);
        column += 6;
        if(column > 122)
        {
            page++;
            column = 0;
            if(*(follower + 1) == ' ')
                follower++;
        }
        follower++;
    }
}

void OLED_StringAutoLine(char* str, unsigned char len, unsigned char column, unsigned char page,  unsigned char maxLines)
{
    char* strFollower = str;
    char charCount = 0, colCount = column;
    uint8_t curLine = 1;
    while(strFollower != str + len)
    {
    	if(!(*strFollower == ' ' && charCount == 0))
    	{
    		OLED_CharASCII(*strFollower, colCount, page);
			colCount += 6;
			charCount++;
    	}
    	strFollower++;
    	if(charCount >= 21)
    	{
    		for(uint8_t i = 0; i < 8; i++)
    		{
    			if(*(strFollower - i) == ' ')
				{
    				charCount -= i;
    				colCount -= i * 6;
    				for(; charCount < 21; charCount++)
    				{
    					OLED_CharASCII(' ', colCount, page);
    					colCount += 6;
    				}
    				strFollower -= i;
    				break;
				}
    		}
    		curLine++;
    		page++;
    		charCount = 0;
    		colCount = column;
    		if(curLine > maxLines)
    			return;
    	}
    }
}

void OLED_DrawPowerSymbolPlug(unsigned char column, unsigned char page)
{
	unsigned char i;
	for(i = 0; i < 10; i++)
		OLED_Buffer[i + (column + (page * 128))] = plug[i];
}

void OLED_DrawPowerSymbolBattery(uint8_t bars, unsigned char column, unsigned char page)
{
	unsigned char i;
	if(bars > 8) bars = 8;
	for(i = 0; i < 10; i++)
		OLED_Buffer[i + (column + (page * 128))] = empty_battery[i];
	for(i = 0; i <= bars; i++)
	{
		OLED_Buffer[i + (column + (page * 128))] = battery_bar;
	}
}

void OLED_Blank()
{
    uint16_t i;
    for(i = 0; i < OLED_PixelCount; i++)
        OLED_Buffer[i] = 0;
}

void OLED_ClearLine(unsigned char line)
{
    uint16_t i, lineStart = line * 128;
    for(i = lineStart; i < lineStart + 128; i++)
        OLED_Buffer[i] = 0;
}

void OLED_Fill()
{
    uint16_t i;
    for(i = 0; i < OLED_PixelCount; i++)
        OLED_Buffer[i] = 0xFF;
}

bool OLED_UpdateScreen()
{
    switch(OLED_Draw_State)
    {
        case OLED_STATE_DrawIdle:
            OLED_Draw_State = OLED_STATE_SetParams;
        case OLED_STATE_SetParams:
            UI_RequestOLEDWrite();
            break;
        case OLED_STATE_Data:
        	UI_RequestOLEDWrite();
        	break;
        case OLED_STATE_DrawWaiting:
        	OLED_Draw_State = OLED_STATE_DrawIdle;
			return true;
    }
    return false;
}

bool OLED_DrawScreen()
{
    if(OLED_Sys_State != OLED_STATE_Idle)
        return false;
    OLED_Sys_State = OLED_STATE_DrawScreen;
    updateCommand = OLED_UPDATE_EntireScreen;
    OLED_UpdateScreen();
    return true;
}

bool OLED_DrawPage(uint8_t page)
{
    if(OLED_Sys_State != OLED_STATE_Idle || page <  0 || page > 3)
        return false;
    OLED_Sys_State = OLED_STATE_DrawScreen;
    startPage = page;
    updateCommand = OLED_UPDATE_Page;
    OLED_UpdateScreen();
    return true;
}

bool OLED_DrawArea(uint8_t page, uint8_t column, uint8_t count)
{
    if(OLED_Sys_State != OLED_STATE_Idle || page <  0 || page > 3 ||
    		column < 0 || column > 127 || count + column > 128)
        return false;
    OLED_Sys_State = OLED_STATE_DrawScreen;
    startPage = page;
    startCol = column;
    colCount = count;
    updateCommand = OLED_UPDATE_Area;
    OLED_UpdateScreen();
    return true;
}

void OLED_SPICallback()
{
	HAL_GPIO_WritePin(GPIOA, OLED_CS_Pin, GPIO_PIN_SET);
    switch(OLED_Sys_State)
    {
        case OLED_STATE_Idle:
            break;
        case OLED_STATE_InitList:
            OLED_Blank();
            OLED_Sys_State = OLED_STATE_DrawScreen;
        case OLED_STATE_DrawScreen:
            if(OLED_UpdateScreen())
            {
                if(initialized)
                    OLED_Sys_State = OLED_STATE_Idle;
                else
                {
                    OLED_Sys_State = OLED_STATE_DispOn;
                    UI_RequestOLEDWrite();
                }
            }
            break;
        case OLED_STATE_DispOn:
            UI_RequestOLEDWrite();
            break;
        case OLED_STATE_Waiting:
            initialized = true;
            OLED_Sys_State = OLED_STATE_Idle;
    }
}

void OLED_WriteUpdateScreen()
{
    switch(OLED_Draw_State)
    {
        case OLED_STATE_SetParams:
            OLED_Draw_State = OLED_STATE_Data;
            if(updateCommand == OLED_UPDATE_EntireScreen)
            {
            	drawList[1] = drawList[4] = 0;		//Start Page and Column
            	drawList[2] = 0x03;					//End Page
            	drawList[5] = 0x7F;					//End Column
            }
            else if(updateCommand == OLED_UPDATE_Page)
            {
            	drawList[1] = drawList[2] = startPage;					//Start and End Page
            	drawList[4] = 0;										//Start Column
            	drawList[5] = 0x7F;										//End Column
            }
            else if(updateCommand == OLED_UPDATE_Area)
            {
            	drawList[1] = drawList[2] = startPage;	//Start and End Page
            	drawList[4] = startCol;					//Start Column
				drawList[5] = startCol + colCount;		//End Column
            }
            OLED_CommandArray_Write(drawList, sizeof(drawList));
            break;
        case OLED_STATE_Data:
            OLED_Draw_State = OLED_STATE_DrawWaiting;
            if(updateCommand == OLED_UPDATE_EntireScreen)
			{
            	OLED_DataArray_Write(OLED_Buffer, OLED_PixelCount);
			}
			else if(updateCommand == OLED_UPDATE_Page)
			{
				OLED_DataArray_Write(OLED_Buffer + (startPage * 128), 128);
			}
			else if(updateCommand == OLED_UPDATE_Area)
			{
				OLED_DataArray_Write(OLED_Buffer + (startPage * 128) + startCol, colCount);
			}
        default:
            break;
    }
}

void OLED_WriteData()
{
    switch(OLED_Sys_State)
    {
        case OLED_STATE_InitList:
             OLED_CommandArray_Write(initList, sizeof(initList));
             break;
        case OLED_STATE_DrawScreen:
            OLED_WriteUpdateScreen();
            break;
        case OLED_STATE_DispOn:
            OLED_Sys_State = OLED_STATE_Waiting;
            OLED_Command_Write(SSD1305_DISPLAYON);
            break;
        default:
            break;
    }
}

void OLED_Init()
{
	HAL_GPIO_WritePin(GPIOA, OLED_RST_Pin, GPIO_PIN_RESET);
	OLEDResetState = OLED_RESET_WAITING;
}

void OLED_InitafterReset()
{
	OLEDResetState = OLED_RESET_DONE;
	HAL_GPIO_WritePin(GPIOA, OLED_RST_Pin, GPIO_PIN_SET);

    OLED_Sys_State = OLED_STATE_InitList;

    UI_RequestOLEDWrite();
}

bool OLED_IsReady()
{
    return initialized & (OLED_Sys_State == OLED_STATE_Idle);
}

bool OLED_IsInitialized()
{
    return initialized;
}
