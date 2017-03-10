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

#define KFLA_EXTEND         0x01
#define KFLA_SPECIAL        0x02
#define KFLA_MAKEONLY       0x04
#define KFLA_MAKE_BREAK     0x08
#define KFLA_PROC_SHIFT     0x10
#define KFLA_WAIT_UNTIL_RX  0x20

#define STA_NORMAL          0
#define STA_RXCHAR          1
#define STA_WAIT_SCAN_SET   2
#define STA_WAIT_SCAN_REPLY 3
#define STA_WAIT_ID         4
#define STA_WAIT_ID1        5
#define STA_WAIT_LEDS       6
#define STA_WAIT_AUTOREP    7
#define STA_WAIT_RESET      8
#define STA_DISABLED        9
#define STA_DELAY           11
#define STA_REPEAT          12

#define START_MAKE  0xFF
#define END_MAKE    0xFE
#define NO_REPEAT   0xFD
#define SPLIT       0xFC
#define WAIT_RX     0xFB

#define PS2_REPEAT_SPEED_NONE	0
#define PS2_REPEAT_SPEED_HIGH	1
#define PS2_REPEAT_SPEED_MIDD	5
#define PS2_REPEAT_SPEED_LOW	10
#define PS2_REPEAT_SPEED_SET_HIGH	1
#define PS2_REPEAT_SPEED_SET_MIDD	2
#define PS2_REPEAT_SPEED_SET_LOW	3


uint8_t ps2_repeat_speed;
void ps2_main(void);

#endif
