
#ifndef USBMAIN_H
#define USBMAIN_H

#include "global.h"
#include <avr/io.h>

extern uint8_t makeReportBuffer(uint8_t keyidx, uint8_t xIsDown);
extern void prepareKeyMappingUsb(void);
extern void initInterfaceUsb(void);
extern void setLedUsb(uint8_t xState);

extern void usb_main(void);

#endif
