#ifndef LAZYFN_C
#define LAZYFN_C

#include "lazyfn.h"
#include <avr/eeprom.h>
#include "eeprominfo.h"
#include "print.h"

static bool _isLazyFn;

void initLazyFn(void){

	uint8_t gOption = eeprom_read_byte((uint8_t *)EEPROM_ENABLED_OPTION);
	DEBUG_PRINT(("initLazyFn : %d %d \n", gOption, gOption&(1<<TOGGLE_SMART_KEY)));	
	if(((gOption >> TOGGLE_LAZY_FN) & 0x01) == OPTION_OFF){
		_isLazyFn = false;
	}else{
		_isLazyFn = true;
	}
}
bool isLazyFn(void){
	return _isLazyFn;
}

void toggleLazyFn(void){
	uint8_t gOption = eeprom_read_byte((uint8_t *)EEPROM_ENABLED_OPTION);
	if(_isLazyFn == false){
		_isLazyFn = true;
		gOption &= ~(1<<TOGGLE_LAZY_FN);	
	}else{
		_isLazyFn = false;	
		gOption |= (1<<TOGGLE_LAZY_FN);
	}
	eeprom_write_byte((uint8_t *)EEPROM_ENABLED_OPTION, gOption);
}

#endif