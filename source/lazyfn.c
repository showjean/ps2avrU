#ifndef LAZYFN_C
#define LAZYFN_C

#include "lazyfn.h"
#include <avr/eeprom.h>
#include "eeprominfo.h"

static bool _isLazyFn;

void initLazyFn(void){

	if(eeprom_read_byte((uint8_t *)EEPROM_LAZY_FN) == 1){
		_isLazyFn = true;
	}else{
		_isLazyFn = false;
	}
}
bool isLazyFn(void){
	return _isLazyFn;
}

void toggleLazyFn(void){
	if(_isLazyFn == false){
		_isLazyFn = true;
		eeprom_write_byte((uint8_t *)EEPROM_LAZY_FN, 1);
	}else{
		_isLazyFn = false;		
		eeprom_write_byte((uint8_t *)EEPROM_LAZY_FN, 255);
	}
}

#endif