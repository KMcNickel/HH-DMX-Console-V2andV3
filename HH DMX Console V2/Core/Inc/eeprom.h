/*
 * eeprom.h
 *
 *  Created on: Aug 5, 2020
 *      Author: Kyle
 */

#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_

#define EEPROM_WREN 0b00000110
#define EEPROM_WRDI 0b00000100
#define EEPROM_RDSR 0b00000101
#define EEPROM_WRSR 0b00000001
#define EEPROM_READ 0b00000011
#define EEPROM_WRITE 0b00000010

void EEPROM_Init();
void EEPROM_WriteBlock(uint16_t addr, uint8_t * data, uint16_t length);
void EEPROM_ReadBlock(uint16_t addr, uint8_t * data, uint16_t length);
bool EEPROM_SPICallback();
void EEPROM_QueryBusyFlag();
void EEPROM_SendReadWriteData();
bool EEPROM_IsBusy();


#endif /* INC_EEPROM_H_ */
