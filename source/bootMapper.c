#ifndef BOOTMAPPER_C
#define BOOTMAPPER_C

#include <stdio.h>
#include <string.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "keymapper.h"
#include "macrobuffer.h"

#ifdef ENABLE_BOOTMAPPER
// bootmapper
static uint8_t _isBootMapper = 0;
void setToBootMapper(void){
	_isBootMapper = 1;
}
uint8_t isBootMapper(void){
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