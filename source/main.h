#ifndef KEYMAIN_H
#define KEYMAIN_H

#include <stdio.h>
#include <stdbool.h>
#include "global.h"

// only for test, use makefile -D option on release.bat
// #define GHOST_KEY_PREVENTION	//ghost key prevention
//#define DISABLE_HARDWARE_MENU
//#define DISABLE_HARDWARE_KEYMAPPING

#if defined( DEBUG_LEVEL )
    #if !defined(DISABLE_HARDWARE_KEYMAPPING)
        #define DISABLE_HARDWARE_KEYMAPPING	// disable_hardware_keymapping
    #endif
    #if !defined(SCROLL_LOCK_LED_IS_APART)
        #define SCROLL_LOCK_LED_IS_APART // SCROLL_LOCK pin was connected
    #endif
//    #if !defined(INTERFACE_ONLY_USB)
//        #define INTERFACE_ONLY_USB // usb only
//    #endif
//    #define DEBUG_QUICK_BOOTLOADER  // for debug
#endif

#define ENABLE_BOOTMAPPER	// use bootmapper

//#define DEBUG_SLEEP_H
//#define DEBUG_KEYMAPPER_H
//#define DEBUG_PRINT_H
//#define DEBUG_QUICKSWAP_H

/* --------------------------- interface --------------------------------*/
#define INTERFACE_PS2		0
#define INTERFACE_USB		1
//#define INTERFACE_USB_USER	2
//#define INTERFACE_PS2_USER	3
//#define INTERFACE_CLEAR		4

typedef struct {
	bool (*hasUpdate)(void);
} interface_update_t;

int interfaceCount;
bool interfaceReady;
uint8_t INTERFACE;		// ps/2 : 0, usb : 1, user usb : 2, user ps/2 : 3, clear user interface : 4 
extern uint8_t delegateGetBootmapperStatus(uint8_t xCol, uint8_t xRow);

int setDelay(int xDelay);
void initAfterInterfaceMount(void);
void setUpdateDriver(interface_update_t *driver);
bool hasUpdate(void);

#endif
