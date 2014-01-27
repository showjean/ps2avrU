#ifndef CUSTOMMACRO_H
#define CUSTOMMACRO_H

#include <stdio.h>
#include <stdbool.h>
#include "global.h"

/*
	2048 flash space
	12 macros
	42 keycodes per macro
	1 keycode use 4byte
	down 2byte up 2byte
	2byte : 1byte = keyindex, 1byte = 8bit(1bit = up/down flag, 7bit = delay * 100ms);
*/

#define CUSTOM_MACRO_SIZE_MAX 			168 // = 42 * 4

extern bool hasCustomMacroAt(uint8_t xMacroIndex);

extern void readCustomMacroAt(uint8_t xMacroIndex);

extern bool isActiveCustomMacro(void);

extern void enterFrameForCustomMacro(void);

#endif