#ifndef BOOTMAPPER_C
#define BOOTMAPPER_C

#include <stdio.h>

#include "keymapper.h"
#include "macrobuffer.h"
#include "bootmapper.h"

#ifdef ENABLE_BOOTMAPPER
// bootmapper
static bool _isBootMapper = false;
void setToBootMapper(void){
	_isBootMapper = true;
}
bool isBootMapper(void){
	return _isBootMapper;
}
void trace(uint8_t xRow, uint8_t xCol){
	// DEBUG_PRINT(("trace : row= %d, col= %d \n", xRow, xCol));
	printString("-");
	printString(toString(xCol));
	printString(",");
	printString(toString(xRow));	
	printString("=");
}
#endif

#endif