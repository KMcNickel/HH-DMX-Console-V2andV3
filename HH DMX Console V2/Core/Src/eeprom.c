/*
 * eeprom.c
 *
 *  Created on: Aug 5, 2020
 *      Author: Kyle
 */

#include "ui.h"
#include "eeprom.h"
#include "stdbool.h"
#include "main.h"

enum EEPROM_SPIActivity
{
	EEPROM_SPI_Inactive,
	EEPROM_SPI_WriteData,
	EEPROM_SPI_ReadDataWriteAddress,
	EEPROM_SPI_ReadData,
	EEPROM_SPI_WriteEnable,
	EEPROM_SPI_QueryBusyFlag
};

enum EEPROM_Status
{
	EEPROM_SPI_Idle,
	EEPROM_SPI_InitWaiting,
	EEPROM_SPI_Busy,
	EEPROM_SPI_WriteInProgress

};

uint16_t dataLength;
uint8_t * curActiveData;
uint8_t TXByte[3];
uint8_t RXByte[3];
uint16_t curAddr;
enum EEPROM_SPIActivity curTask;
enum EEPROM_Status status;
extern SPI_HandleTypeDef hspi1;

void EEPROM_WriteData(uint8_t * data, uint8_t len)
{
	HAL_GPIO_WritePin(GPIOA, MEM_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit_IT(&hspi1, data, len);
}

void EEPROM_ReadWriteData(uint8_t * TXData, uint8_t * RXData, uint8_t len)
{
	HAL_GPIO_WritePin(GPIOA, MEM_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive_IT(&hspi1, TXData, RXData, len);
}

void EEPROM_ReadData(uint8_t * data, uint8_t len)
{
	HAL_GPIO_WritePin(GPIOA, MEM_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Receive_IT(&hspi1, data, len);
}

void EEPROM_SPICallback()
{
	HAL_GPIO_WritePin(GPIOA, MEM_CS_Pin, GPIO_PIN_SET);
	switch(curTask)
	{
		case EEPROM_SPI_WriteData:
			status = EEPROM_SPI_WriteInProgress;
			break;
		case EEPROM_SPI_ReadData:
		case EEPROM_SPI_WriteEnable:
			status = EEPROM_SPI_Idle;
			break;
		case EEPROM_SPI_ReadDataWriteAddress:
			curTask = EEPROM_SPI_ReadData;
			UI_RequestEEPROMReadWrite();
			break;
		case EEPROM_SPI_QueryBusyFlag:
			if((RXByte[1] & 1) == 0)
				status = EEPROM_SPI_Idle;
			break;
		default:
			break;
	}
}

void EEPROM_SendReadWriteData()
{
	switch(curTask)
	{
		case EEPROM_SPI_WriteData:
			break;
		case EEPROM_SPI_ReadDataWriteAddress:
			TXByte[0] = EEPROM_WRITE;
			TXByte[1] = curAddr >> 8;
			TXByte[2] = curAddr && 0xFF;
			EEPROM_WriteData(TXByte, 3);
			break;
		case EEPROM_SPI_ReadData:
			EEPROM_ReadData(curActiveData, dataLength);
			break;
		case EEPROM_SPI_WriteEnable:
			EEPROM_WriteData(TXByte, 1);
			break;
		case EEPROM_SPI_QueryBusyFlag:
			EEPROM_ReadWriteData(TXByte, RXByte, 2);
			break;
		default:
			break;
	}
}

void EEPROM_Init()
{
	status = EEPROM_SPI_InitWaiting;
	TXByte[0] = EEPROM_WREN;
	curActiveData = TXByte;
	dataLength = 1;

	curTask = EEPROM_SPI_WriteEnable;

	UI_RequestEEPROMReadWrite();
}

void EEPROM_WriteBlock(uint16_t addr, uint8_t * data, uint16_t length)
{
	if(data == NULL || length == 0 || status != EEPROM_SPI_Idle) return;
	dataLength = length;
	curActiveData = data;
	curAddr = addr;

	status = EEPROM_SPI_Busy;
	curTask = EEPROM_SPI_WriteData;

	UI_RequestEEPROMReadWrite();
}

void EEPROM_ReadBlock(uint16_t addr, uint8_t * data, uint16_t length)
{
	if(data == NULL || length == 0 || status != EEPROM_SPI_Idle) return;
		dataLength = length;
		curActiveData = data;
		curAddr = addr;

		status = EEPROM_SPI_Busy;
		curTask = EEPROM_SPI_ReadDataWriteAddress;

		UI_RequestEEPROMReadWrite();
}

void EEPROM_QueryBusyFlag()
{
	TXByte[0] = EEPROM_RDSR;

	status = EEPROM_SPI_Busy;
	curTask = EEPROM_SPI_QueryBusyFlag;

	UI_RequestEEPROMReadWrite();
}

bool EEPROM_IsBusy()
{
	if(status == EEPROM_SPI_WriteInProgress)
		EEPROM_QueryBusyFlag();
	return status != EEPROM_SPI_Idle;
}


