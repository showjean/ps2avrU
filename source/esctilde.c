
#include "esctilde.h"
#include "keydownbuffer.h"
#include "oddebug.h"

//
static bool _isEscTilde;

#ifndef DISABLE_HARDWARE_MENU
#define CMD_TOGGLE_ESC_TILDE 1
#define CMD_EXIT_ESC_TILDE 3
#define CMD_BACK_ESC_TILDE 6

const char str_select_esc_tilde[] PROGMEM =  "esc to ~";

void printMenuEscTilde(void);
void printContentsEscTilde(void);
void putKeyindexEscTilde(uint8_t xCmd, uint8_t xKeyidx, uint8_t xCol, uint8_t xRow, uint8_t xIsDown);


keymapper_driver_t driverKeymapperEscTilde = {
		printMenuEscTilde,
		printContentsEscTilde,
		putKeyindexEscTilde
};

static void printEscTildeState(void){
	printStringFromFlash(str_select_esc_tilde);
	printStringFromFlash(str_space);
	printStringFromFlash(str_colon);
	printStringFromFlash(str_space);
	if(isEscTilde()){
		printStringFromFlash(str_on);
	}else{
		printStringFromFlash(str_off);
	}
}

void printMenuEscTilde(void){
	printEscTildeState();
}

void printContentsEscTilde(void){
	printEscTildeState();
	printEnter();
	printString(toString(CMD_TOGGLE_ESC_TILDE));
	printStringFromFlash(str_colon);
	printStringFromFlashWithEnter(str_toggle);
	printString(toString(CMD_EXIT_ESC_TILDE));
	printStringFromFlash(str_colon);
	printStringFromFlashWithEnter(str_exit);	// exit
	printString(toString(CMD_BACK_ESC_TILDE));
	printStringFromFlash(str_colon);
	printStringFromFlashWithEnter(str_back);
}

void putKeyindexEscTilde(uint8_t xCmd, uint8_t xKeyidx, uint8_t xCol, uint8_t xRow, uint8_t xIsDown){
	if(xCmd == CMD_TOGGLE_ESC_TILDE){
		setStep(STEP_INPUT_COMMAND);
		toggleEscTilde();
		printEscTildeState();
		printEnter();
	}else if(xCmd == CMD_EXIT_ESC_TILDE){
		setStep(STEP_EXIT);
		stopKeyMapping();
	}else if(xCmd == CMD_BACK_ESC_TILDE){
		setStep(STEP_BACK);
	}
}
void toggleEscTilde(void){
    _isEscTilde ^= true;
    setToggleOption(EEPROM_ENABLED_OPTION, TOGGLE_ESC_TO_TILDE, _isEscTilde);
}
#endif

static bool _isEscTildeDown = false;
uint8_t getEscToTilde(uint8_t xKeyidx, bool xIsDown){
    if(xKeyidx == KEY_ESC && isEscTilde()){
    	if(xIsDown){
			uint8_t gModi = getModifierDownBuffer();
//			DBG1(0x33, (uchar *)&gModi, 1);
			if(gModi == 0x02 || gModi == 0x20){
				xKeyidx = KEY_HASH;
				_isEscTildeDown = true;
			}
    	}else{
    		if(_isEscTildeDown){
    			_isEscTildeDown = false;
    			xKeyidx = KEY_HASH;
    		}
    	}
    }
    return xKeyidx;
}

void initEscTilde(void){
	_isEscTilde = getToggleOption(EEPROM_ENABLED_OPTION, TOGGLE_ESC_TO_TILDE);
#ifndef DISABLE_HARDWARE_MENU
	addKeymapperDriver(&driverKeymapperEscTilde);
#endif
}

bool isEscTilde(void){
	return _isEscTilde;
}

void setEscTilde(bool xEnabled)
{
    _isEscTilde = xEnabled;
    setToggleOption(EEPROM_ENABLED_OPTION, TOGGLE_ESC_TO_TILDE, _isEscTilde);
}

