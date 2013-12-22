
#ifndef VUSB_H
#define VUSB_H

#include "global.h"

/* report id */
#define REPORT_ID_MOUSE     3
#define REPORT_ID_SYSTEM    2
#define REPORT_ID_CONSUMER  1
/* report size */
#define REPORT_SIZE_KEYBOARD    8
#define REPORT_SIZE_EXTRA  3

extern uint8_t reportBuffer[REPORT_SIZE_KEYBOARD]; ///< buffer for HID reports
extern uint8_t reportBufferExtra[REPORT_SIZE_EXTRA];
extern uint8_t idleRate;        ///< in 4ms units

extern void delegateLedUsb(uint8_t xState);
extern void delegateInterfaceReadyUsb(void);
extern void delegateInitInterfaceUsb(void);

#endif
