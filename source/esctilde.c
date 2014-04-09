
#include "esctilde.h"

//
#define CMD_TOGGLE_ESC_TILDE 1
#define CMD_EXIT_ESC_TILDE 3
#define CMD_BACK_ESC_TILDE 6

const char str_select_esc_tilde[] PROGMEM =  "esc to ~";

static bool _isEscTilde;

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
	printStringFromFlash(str_toggle);
	printEnter();
	printString(toString(CMD_EXIT_ESC_TILDE));
	printStringFromFlash(str_colon);
	printStringFromFlash(str_exit);	// exit
	printEnter();
	printString(toString(CMD_BACK_ESC_TILDE));
	printStringFromFlash(str_colon);
	printStringFromFlash(str_back);
	printEnter();
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

void initEscTilde(void){
	_isEscTilde = getToggleOption(EEPROM_ENABLED_OPTION, TOGGLE_ESC_TO_TILDE);
}
bool isEscTilde(void){
	return _isEscTilde;
}
void toggleEscTilde(void){
	_isEscTilde ^= true;
	setToggleOption(EEPROM_ENABLED_OPTION, TOGGLE_ESC_TO_TILDE, _isEscTilde);
}

