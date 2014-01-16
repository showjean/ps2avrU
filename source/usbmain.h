
#ifndef USBMAIN_H
#define USBMAIN_H

#include "global.h"
#include <avr/io.h>

extern uint8_t makeReportBufferDecorator(uint8_t keyidx, uint8_t xIsDown);
// extern void prepareKeyMappingUsb(void);

extern void usb_main(void);

#endif
