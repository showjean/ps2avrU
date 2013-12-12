#ifndef QUICKSWAP_H
#define QUICKSWAP_H

#include <stdio.h>

#include "keymain.h"

#ifdef DEBUG_QUICKSWAP_H
	#define QUICKSWAP_COUNT_MAX 500
#else
	#define QUICKSWAP_COUNT_MAX 5000
#endif

#define COUNT_TYPE_QUICKSWAP_ALT_GUI 	2
#define QUICKSWAP_ALT_GUI				(1 << 0) 	// bit index for eeprom write
#define QUICKSWAP_CAPS_LCTRL			(1 << 1) 	// bit index for eeprom write
/**
	전달되는 키인덱스를 현재 퀵스왑 형식에 맞게 변환;
*/
extern uint8_t getQuickSwapKeyindex(uint8_t xKeyidx);
extern void initQuickSwap(void);

extern void enterFrameForQuickSwap(void);

#endif