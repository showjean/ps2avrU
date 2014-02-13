#ifndef CUSTOMMACRO_H
#define CUSTOMMACRO_H

#include <stdio.h>
#include <stdbool.h>
#include "global.h"

#define BYTE_PER_KEYCODE	4

/*
	1 keycode use 4byte
	down 2byte up 2byte
	2byte : 1byte = keyindex, 1byte = 8bit(1bit = up/down flag, 7bit = delay * 100ms);
*/
extern void initCustomMacro(void);

extern bool hasCustomMacroAt(uint8_t xMacroIndex);

extern void readCustomMacroAt(uint8_t xMacroIndex);

extern bool isActiveCustomMacro(void);

extern void enterFrameForCustomMacro(void);

#endif