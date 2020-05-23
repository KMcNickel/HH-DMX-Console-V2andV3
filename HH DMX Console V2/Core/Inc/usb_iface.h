/*
 * usb_iface.h
 *
 *  Created on: May 21, 2020
 *      Author: Kyle
 */

#ifndef INC_USB_IFACE_H_
#define INC_USB_IFACE_H_



void USB_BootloaderInit();
void USB_TriggerBootloader();
void USB_CheckRXData(uint8_t* Buf, uint32_t *Len);

#endif /* INC_USB_IFACE_H_ */
