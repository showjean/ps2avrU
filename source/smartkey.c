
#include "smartkey.h"
#include "keymap.h"
#include <avr/eeprom.h>
#include "eeprominfo.h"
// #include "print.h"
#include "oddebug.h"
#include "ps2avru_util.h"

// smart key
#define CMD_TOGGLE_SMART_KEY 1
#define CMD_EXIT_SMART_KEY 3
#define CMD_BACK_SMART_KEY 6

const char str_select_smart_key[] PROGMEM =  "smart key";

static bool _isOsx = false;
static bool _enabled = false;
static bool _fixed = false;


void printMenuSmartKey(void);
void printContentsSmartKey(void);
void putKeyindexSmartKey(uint8_t xCmd, uint8_t xKeyidx, uint8_t xCol, uint8_t xRow, uint8_t xIsDown);

keymapper_driver_t driverKeymapperSmartKey = {
   	printMenuSmartKey,
    printContentsSmartKey,
    putKeyindexSmartKey
};


static void printSmartKeyState(void){	
	printStringFromFlash(str_select_smart_key);
	printStringFromFlash(str_space);
	printStringFromFlash(str_colon);
	printStringFromFlash(str_space);
	if(isSmartKeyEnabled()){
		printStringFromFlash(str_on);
	}else{
		printStringFromFlash(str_off);
	}
}

void printMenuSmartKey(void){
	printSmartKeyState();
}

void printContentsSmartKey(void){
	printSmartKeyState();
	printEnter();
	printString(toString(CMD_TOGGLE_SMART_KEY));
	printStringFromFlash(str_colon);
	printStringFromFlashWithEnter(str_toggle);
	printString(toString(CMD_EXIT_SMART_KEY));
	printStringFromFlash(str_colon);
	printStringFromFlashWithEnter(str_exit);	// exit
	printString(toString(CMD_BACK_SMART_KEY));
	printStringFromFlash(str_colon);
	printStringFromFlashWithEnter(str_back);

}

void putKeyindexSmartKey(uint8_t xCmd, uint8_t xKeyidx, uint8_t xCol, uint8_t xRow, uint8_t xIsDown){
	if(xCmd == CMD_TOGGLE_SMART_KEY){
		setStep(STEP_INPUT_COMMAND);
		toggleSmartKeyEnabled();
		printSmartKeyState();
		printEnter();
	}else if(xCmd == CMD_EXIT_SMART_KEY){
		setStep(STEP_EXIT);
		stopKeyMapping();						
	}else if(xCmd == CMD_BACK_SMART_KEY){		
		setStep(STEP_BACK);
	}
}

void initSmartKey(void){
//	_enabled = getToggleOption(EEPROM_ENABLED_OPTION, TOGGLE_SMART_KEY);
//	addKeymapperDriver(&driverKeymapperSmartKey);
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
	if(!_fixed) {
		_isOsx = xIsOsx;
		_fixed = true;
	}
}

void toggleSmartKeyEnabled(void){	
	_enabled ^= true;	
    setToggleOption(EEPROM_ENABLED_OPTION, TOGGLE_SMART_KEY, _enabled);
}
