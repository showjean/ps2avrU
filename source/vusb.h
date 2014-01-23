/*
Copyright 2011 Jun Wako <wakojun@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef VUSB_H
#define VUSB_H

#include "global.h"

/* report id */
#define REPORT_ID_MOUSE     3
#define REPORT_ID_SYSTEM    2
#define REPORT_ID_CONSUMER  1
/* report size */
#define REPORT_KEYS    6
#define REPORT_SIZE_KEYBOARD  8

typedef struct {
    // uint8_t report_id;
    uint8_t mods;
    uint8_t reserved;
    uint8_t keys[REPORT_KEYS];
    // uint8_t ext;
} __attribute__ ((packed)) report_keyboard_t;

typedef struct {
    uint8_t  report_id;
    uint16_t usage;
} __attribute__ ((packed)) report_extra_t;

typedef struct {
    uint8_t buttons;
    int8_t x;
    int8_t y;
    int8_t v;
    int8_t h;
} __attribute__ ((packed)) report_mouse_t;

typedef struct {
    uint8_t report_id;
    report_mouse_t report;
} __attribute__ ((packed)) vusb_mouse_report_t;


extern report_keyboard_t reportKeyboard; ///< buffer for HID reports
extern uint8_t idleRate;        ///< in 4ms units

extern void delegateLedUsb(uint8_t xState);
extern void delegateInterfaceReadyUsb(void);
extern void delegateInitInterfaceUsb(void);

#endif
