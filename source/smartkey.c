
#include "smartkey.h"
#include "keymap.h"
#include <avr/eeprom.h>
#include "eeprominfo.h"
#include "print.h"

static bool _isOsx = false;
static bool _enabled = false;
void initSmartKey(void){

	uint8_t gOption = eeprom_read_byte((uint8_t *)EEPROM_ENABLED_OPTION);
	DEBUG_PRINT(("initSmartKey : %d %d \n", gOption, gOption&(1<<TOGGLE_SMART_KEY)));	
	if(((gOption >> TOGGLE_SMART_KEY) & 0x01) == OPTION_OFF){
		_enabled = false;
	}else{
		_enabled = true;
	}
}
bool isSmartKeyEnabled(void){
	return _enabled;
}

uint8_t getSmartKeyIndex(uint8_t xKeyidx){
    if(isSmartKeyEnabled() && _isOsx){
    	switch(xKeyidx) {
			case KEY_LALT:
				xKeyidx = KEY_LGUI;
				break;
			case KEY_LGUI:
				xKeyidx = KEY_LALT;
				break;
			case KEY_RALT:
				xKeyidx = KEY_RGUI;
				break;
			case KEY_RGUI:
				xKeyidx = KEY_RALT;
				break;
		}
    }
    return xKeyidx;
}

void setCurrentOS(bool xIsOsx){
	_isOsx = xIsOsx;
}

void toggleSmartKeyEnabled(void){	
	uint8_t gOption = eeprom_read_byte((uint8_t *)EEPROM_ENABLED_OPTION);
	if(_enabled == false){
		_enabled = true;
		gOption &= ~(1<<TOGGLE_SMART_KEY);	
	}else{
		_enabled = false;	
		gOption |= (1<<TOGGLE_SMART_KEY);
	}
	eeprom_write_byte((uint8_t *)EEPROM_ENABLED_OPTION, gOption);
}