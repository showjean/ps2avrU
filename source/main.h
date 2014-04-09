#ifndef KEYMAIN_H
#define KEYMAIN_H

#include <stdio.h>
#include <stdbool.h>
#include "global.h"

// for oddebug
// #define  DEBUG_LEVEL 		1

// #define DEBUG_KEYMAIN	// if you want to debug mode

// only for test, use makefile -D option on release.bat
// #define GHOST_KEY_PREVENTION	//ghost key prevention
// #define SCROLL_LOCK_LED_IS_APART	// SCROLL_LOCK pin was connect
// #define INTERFACE_ONLY_USB	// usb only
// #define DISABLE_HARDWARE_KEYMAPPING	// disable_hardware_keymapping

//#define DEBUG_QUICK_BOOTLOADER  // for debug

#ifdef DEBUG_KEYMAIN
    #define DEBUG_KEYMAPPER_H
	#define DEBUG_SLEEP_H
	#define DEBUG_PRINT_H
	#define DEBUG_QUICKSWAP_H
#endif

#define ENABLE_BOOTMAPPER	// use bootmapper	

/* --------------------------- interface --------------------------------*/
#define INTERFACE_PS2		0
#define INTERFACE_PS2_USER	3
#define INTERFACE_USB		1
#define INTERFACE_USB_USER	2
#define INTERFACE_CLEAR		4

int interfaceCount;
bool interfaceReady;
uint8_t INTERFACE;		// ps/2 : 0, usb : 1, user usb : 2, user ps/2 : 3, clear user interface : 4 
extern uint8_t delegateGetBootmapperStatus(uint8_t xCol, uint8_t xRow);

int setDelay(int xDelay);
void initAfterInterfaceMount(void);

#endif
