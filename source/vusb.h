
#ifndef VUSB_H
#define VUSB_H

#include "global.h"

/* report id */
#define REPORT_ID_MOUSE     3
#define REPORT_ID_SYSTEM    2
#define REPORT_ID_CONSUMER  1
/* report size */
#define REPORT_SIZE_KEYBOARD    8
#define REPORT_SIZE_CONSUMER  3

extern uint8_t reportBuffer[REPORT_SIZE_KEYBOARD]; ///< buffer for HID reports
extern uint8_t reportBufferConsumer[REPORT_SIZE_CONSUMER];
extern uint8_t idleRate;        ///< in 4ms units

#endif
