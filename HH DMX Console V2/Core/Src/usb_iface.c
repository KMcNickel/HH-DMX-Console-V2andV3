/*
 * usb_iface.c
 *
 *  Created on: May 21, 2020
 *      Author: Kyle
 */

#include "main.h"
#include "cli.h"
#include "stm32g4xx_hal_rcc.h"
#include "usbd_core.h"
#include "usbd_def.h"

void (*SysMemBootJump) (void);
extern USBD_HandleTypeDef hUsbDeviceFS;
uint8_t switchToBootloader __attribute__ ((section (".noinit")));

void USB_BootloaderInit()
{
	switchToBootloader = 0x00;	//Reset the variable to prevent being stuck in the bootloader (since a device reset wont change it)
	volatile uint32_t addr = 0x1FFF0000;	//The STM32G431KB system memory start address
	SysMemBootJump = (void (*)(void)) (*((uint32_t *)(addr + 4)));	//Point the PC to the System Memory reset vector

	HAL_RCC_DeInit();		//Reset the system clock
	SysTick->CTRL = 0;		//Reset the  SysTick Timer
	SysTick->LOAD = 0;
	SysTick->VAL  = 0;

	__set_MSP(*(uint32_t *)addr);	//Set the Main Stack Pointer

	SysMemBootJump();				//Run our virtual function defined above that sets the PC

	while(1);
}

void USB_TriggerBootloader()
{
	SYSCFG->MEMRMP &= 0xFFFFFFF9;		//Remap the memory (may not be necessary)
	SYSCFG->MEMRMP |= 1;
	switchToBootloader = 0x11;			//Set the noinit variable to be read by startup code
	NVIC_SystemReset();					//Reset the system
}

void USB_CheckRXData(uint8_t* Buf, uint32_t *Len)
{
	switch(*Buf)
	{
		case 0x30 ... 0x39:		//0 - 9
			CLI_AddToCommand(*Buf - 0x30);
			break;
		case 0x54:				//T
		case 0x74:				//t
			CLI_AddToCommand(BtnThru);
			break;
		case 0x41:				//A
		case 0x61:				//a
			CLI_AddToCommand(BtnAt);
			break;
		case 0x46:				//F
		case 0x66:				//f
			CLI_AddToCommand(BtnFull);
			break;
		case 0x4C:				//L
		case 0x6C:				//l
			CLI_AddToCommand(BtnLast);
			break;
		case 0x4E:				//N
		case 0x6E:				//n
			CLI_AddToCommand(BtnNext);
			break;
		case 0x2D:				//-
			CLI_AddToCommand(BtnMinus);
			break;
		case 0x2B:				//+
			CLI_AddToCommand(BtnPlus);
			break;
		case 0x50:				//P
		case 0x70:				//p
			CLI_AddToCommand(BtnPreset);
			break;
		case 0x52:				//R
		case 0x72:				//r
			CLI_AddToCommand(BtnRecord);
			break;
		case 0x4F:				//O
		case 0x6F:				//o
			CLI_AddToCommand(BtnOffset);
			break;
		case 0x4D:				//T
		case 0x6D:				//t
			CLI_AddToCommand(BtnTime);
			break;
		case 0xD:				//Return
			CLI_AddToCommand(BtnEnter);
			break;
		case 0x7F:				//Backspace
			CLI_AddToCommand(BtnBksp);
			break;
		case 0x43:				//C
		case 0x63:				//c
			CLI_AddToCommand(BtnClear);
			break;
		case 0x21:				//!
			USB_TriggerBootloader();
			break;
		default:
			break;
	}

}

