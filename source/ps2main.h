/* Copyright Jamie Honan, 2001.  Distributed under the GPL.
   This program comes with ABSOLUTELY NO WARRANTY.
   See the file COPYING for license details.
   */
   
#ifndef PS2MAIN_H
#define PS2MAIN_H

#include "global.h"
#include <avr/io.h>

/* This controls what to do when a character has been recieved from the
   host, and when we want to send a character to the host. Note that
   replies are driven by a state machine.
   */

#define STA_NORMAL		0
#define STA_RXCHAR		1
#define STA_WAIT_SCAN_SET	2
#define STA_WAIT_SCAN_REPLY	3
#define STA_WAIT_ID		4
#define STA_WAIT_ID1		5
#define STA_WAIT_LEDS		6
#define STA_WAIT_AUTOREP	7
#define STA_WAIT_RESET		8
#define STA_DISABLED		9
#define STA_DELAY           11
#define STA_REPEAT        12

#define START_MAKE 0xFF
#define END_MAKE   0xFE
#define NO_REPEAT  0xFD
#define SPLIT      0xFC

extern uint8_t pushKeyCodeDecorator(uint8_t keyidx, bool isDown);
// extern void prepareKeyMappingPs2(void);
extern void ps2_main(void);
extern void initInterfacePs2(void);

#endif