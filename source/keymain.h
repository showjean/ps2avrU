#ifndef KEYMAIN_H
#define KEYMAIN_H

#include <stdio.h>
#include "global.h"

// #define DEBUG_KEYMAIN	// if you want to debug mode

// only for test, use makefile -D option on release.bat
// #define GHOST_KEY_PREVENTION	//ghost key prevention
// #define SCROLL_LOCK_LED_IS_APART	// SCROLL_LOCK pin was connect

#ifdef DEBUG_KEYMAIN
	#define DEBUG_KEYMAPPER_H
	#define DEBUG_SLEEP_H
	#define DEBUG_PRINT_H
#endif

#define ENABLE_BOOTMAPPER	// use bootmapper	

/* --------------------------- interface --------------------------------*/
#define INTERFACE_PS2		0
#define INTERFACE_PS2_USER	3
#define INTERFACE_USB		1
#define INTERFACE_USB_USER	2
#define INTERFACE_CLEAR		4

extern int interfaceCount;
extern uint8_t interfaceReady;
extern uint8_t INTERFACE;		// ps/2 : 0, usb : 1, user usb : 2, user ps/2 : 3, clear user interface : 4 

#endif