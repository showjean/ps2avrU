#ifndef KEYMAIN_H
#define KEYMAIN_H

#define DEBUG_KEYMAIN	// if you want to debug mode

#ifdef DEBUG_KEYMAIN
	#define DEBUG_KEYMAPPER_H
	#define DEBUG_SLEEP_H
	#define DEBUG_PRINT_H
#endif

#define ENABLE_BOOTMAPPER	// use bootmapper

// only for test, use makefile -D option on release.bat
// #define GHOST_KEY_PREVENTION	//ghost key prevention
// #define SCROLL_LOCK_LED_IS_APART	// pd6 pin was connect

#endif