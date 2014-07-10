
#include "timerinclude.h"
// #include "print.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include <util/delay.h>
#include <string.h>

#include "fncontrol.h"
#include "keymap.h"
#include "keymatrix.h"
#include "ledrender.h"
#include "keymapper.h"
#include "dualaction.h"
#include "keymatrix.h"
#include "keydownbuffer.h"
#include "ps2avru_util.h"

#define CMD_TOGGLE_BEYOND_FN_LED 1
#define CMD_EXIT_BEYOND_FN_LED 3
#define CMD_BACK_BEYOND_FN_LED 6

const char str_select_beyond_fn[] PROGMEM = "fn2 led";

static bool _isBeyondFnLedEnabled;
// for KEY_BEYOND_FN;
static uint8_t _beyondFnIndex = 0;     //KEY_BEYOND_FN state
static bool _isExtraFNDown = false;
static bool _isQuickMacroDown = false;
static bool _isReadyQuickMacro = false;
static uint8_t _quickMacroIndex = 255;
static uint8_t _isLockKey = LOCK_NOT_SET;
static uint8_t _isLockWin = LOCK_NOT_SET;

static bool _ledOff = false;

uint8_t isBeyondFN(void){
    return _beyondFnIndex;
}

bool isBeyondFnLedEnabled(void){
    return _isBeyondFnLedEnabled;
}
static void toggleBeyondFnLedEnabled(void){    
    _isBeyondFnLedEnabled ^= true;
    setToggleOption(EEPROM_ENABLED_OPTION, TOGGLE_BEYOND_FN_LED, _isBeyondFnLedEnabled);
    
    setLEDIndicate();
}


void printMenuBeyondFn(void);
void printContentsBeyondFn(void);
void putKeyindexBeyondFn(uint8_t xCmd, uint8_t xKeyidx, uint8_t xCol, uint8_t xRow, uint8_t xIsDown);

keymapper_driver_t driverKeymapperBeyondFn = {
    printMenuBeyondFn,
    printContentsBeyondFn,
    putKeyindexBeyondFn
};

void initBeyondFn(void){
    _isBeyondFnLedEnabled = getToggleOption(EEPROM_ENABLED_OPTION, TOGGLE_BEYOND_FN_LED);
    addKeymapperDriver(&driverKeymapperBeyondFn);
}

static void printBeyondFnLedState(void){
    printStringFromFlash(str_select_beyond_fn);
    printStringFromFlash(str_space);
    printStringFromFlash(str_colon);
    printStringFromFlash(str_space);
    if(isBeyondFnLedEnabled()){
        printStringFromFlash(str_on);
    }else{
        printStringFromFlash(str_off);
    }
}

void printMenuBeyondFn(void){
    printBeyondFnLedState();
}

void printContentsBeyondFn(void){
    printBeyondFnLedState();
    printEnter();
    printString(toString(CMD_TOGGLE_BEYOND_FN_LED));
    printStringFromFlash(str_colon);
    printStringFromFlash(str_toggle);
    printEnter();
    printString(toString(CMD_EXIT_BEYOND_FN_LED));
    printStringFromFlash(str_colon);
    printStringFromFlash(str_exit); // exit
    printEnter();
    printString(toString(CMD_BACK_BEYOND_FN_LED));
    printStringFromFlash(str_colon);
    printStringFromFlash(str_back);
    printEnter();
}

void putKeyindexBeyondFn(uint8_t xCmd, uint8_t xKeyidx, uint8_t xCol, uint8_t xRow, uint8_t xIsDown){
    if(xCmd == CMD_TOGGLE_BEYOND_FN_LED){
        setStep(STEP_INPUT_COMMAND);
        toggleBeyondFnLedEnabled();
        printBeyondFnLedState();
        printEnter();
    }else if(xCmd == CMD_EXIT_BEYOND_FN_LED){
        setStep(STEP_EXIT);
        stopKeyMapping();
    }else if(xCmd == CMD_BACK_BEYOND_FN_LED){     
        setStep(STEP_BACK);
    }
}

static void applyLock(uint8_t *lock){
    if(*lock == LOCK_WILL_SET){
        *lock = LOCK_IS_SET;
        blinkOnce(50);
        _delay_ms(50);
        blinkOnce(100);
    }else if(*lock == LOCK_WILL_NOT_SET){
        *lock = LOCK_NOT_SET;
        blinkOnce(100);
    }
}
// 
void enterFrameForFnControl(void){
    if(isReleaseAll()){
        applyLock(&_isLockKey);
        applyLock(&_isLockWin);
    }
    if(isReleaseAllPrev() && _isReadyQuickMacro){
        _isReadyQuickMacro = false;
        if(_quickMacroIndex < 255) {
            startQuickMacro(_quickMacroIndex);
            _quickMacroIndex = 255;
        }
    }
}
static void __setKeyEnabled(uint8_t *lock){
    if(*lock == LOCK_IS_SET){
        *lock = LOCK_WILL_NOT_SET;
    }else{
        *lock = LOCK_WILL_SET;              
    }
}
void setKeyEnabled(uint8_t xKeyidx, bool xIsDown){
    if(xIsDown) {   // !xIsDown은 오작동한다.
        if(xKeyidx == KEY_LOCK_ALL){
            __setKeyEnabled(&_isLockKey);
        }else if(xKeyidx == KEY_LOCK_WIN){
            __setKeyEnabled(&_isLockWin);
        }
    }
}

bool isKeyEnabled(uint8_t xKeyidx){

    if(_isLockKey == LOCK_IS_SET){
        return false;
    }else if(_isLockWin == LOCK_IS_SET && (xKeyidx == KEY_LGUI || xKeyidx == KEY_RGUI)){
        return false;
    }
    return true;

}

// 키를 누르거나 땔때 FN 및 LED등 을 컨트롤한다.
bool applyFN(uint8_t xKeyidx, uint8_t xCol, uint8_t xRow, bool xIsDown) {

    // DEBUG_PRINT(("applyFN  : %d xIsDown : %d\n", xKeyidx, xIsDown));
    if(isFnKey(xKeyidx)) return false; 

    applyKeyDownForFullLED(xKeyidx, xCol, xRow, xIsDown);
    setDualAction(xKeyidx, xIsDown);    

    // 듀얼액션 키의 기본 키를 가져온다.
    xKeyidx = getDualActionDefaultKey(xKeyidx);
    if(isFnKey(xKeyidx)) return false; 

    if(xIsDown) {

        if((xKeyidx ==  KEY_BEYOND_FN || xKeyidx == KEY_BEYOND_FN3)
        		|| (_isExtraFNDown && xKeyidx == BEYOND_FN_CANCEL_KEY)){ // beyond_fn을 활성화;
             if( xKeyidx == BEYOND_FN_CANCEL_KEY ) {    // 취소만 가능한 키 
                _beyondFnIndex = false;
             }else{
            	if(_beyondFnIndex == 0){
            		if(xKeyidx ==  KEY_BEYOND_FN){
            			_beyondFnIndex = LAYER_FN2;
            		}else{
            			_beyondFnIndex = LAYER_FN3;
            		}
            	}else{
            		if(xKeyidx ==  KEY_BEYOND_FN){
            			if(_beyondFnIndex ==  LAYER_FN2){
            				_beyondFnIndex = LAYER_NORMAL;
            			}else{
            				_beyondFnIndex = LAYER_FN2;
            			}
					}else if(xKeyidx ==  KEY_BEYOND_FN3){
            			if(_beyondFnIndex ==  LAYER_FN3){
            				_beyondFnIndex = LAYER_NORMAL;
            			}else{
            				_beyondFnIndex = LAYER_FN3;
            			}
					}
            	}
             }

#ifndef DISABLE_FN2_TOGGLE_LED_BLINK 
             if(isBeyondFnLedEnabled() == false){
                 if(_beyondFnIndex == 0){
                    blinkOnce(100);
                 }else{
                    blinkOnce(100);
                    _delay_ms(80);
                    blinkOnce(70);
                 }
             }
#endif

             if(isBeyondFnLedEnabled()){    
                setLed(LED_STATE_NUM, isBeyondFN());
             }

             return 0;
        }else if(_isQuickMacroDown && isEepromMacroKey(xKeyidx)){
            _quickMacroIndex = xKeyidx - KEY_MAC1; 
            _isReadyQuickMacro = true;
            return 0;
        }else if(xKeyidx == EXTRA_FN){
            _isExtraFNDown = true;
        }else if((_isExtraFNDown && xKeyidx == LED_KEY)){
        	uint8_t gModi = getModifierDownBuffer();
			if(gModi == 0x02 || gModi == 0x20){
				changeFullLedState(FULL_LED_MODE2);
			}else{
				changeFullLedState(FULL_LED_MODE1);
			}
            return 0;
        }else if(xKeyidx == KEY_LED){
			changeFullLedState(FULL_LED_MODE1);
			return 0;
        }else if(xKeyidx == KEY_LED2){
			changeFullLedState(FULL_LED_MODE2);
			return 0;
        }else if(xKeyidx == KEY_LED_UP){
        	uint8_t gModi = getModifierDownBuffer();
        	if(gModi == 0x02 || gModi == 0x20){
        		increaseLedBrightness(FULL_LED_MODE2);
        	}else{
        		increaseLedBrightness(FULL_LED_MODE1);
        	}
        	return 0;
        }else if(xKeyidx == KEY_LED_DOWN){
        	uint8_t gModi = getModifierDownBuffer();
        	if(gModi == 0x02 || gModi == 0x20){
        		reduceLedBrightness(FULL_LED_MODE2);
        	}else{
        		reduceLedBrightness(FULL_LED_MODE1);
        	}
            return 0;
        }else if(xKeyidx == KEY_LED_ON_OFF){
        	_ledOff ^= true;
        	if(_ledOff == false){
        		turnOnLedAll();
        	}else{
        		turnOffLedAll();
        	}
        }else if(xKeyidx == KEY_QUICK_MACRO){
            if(isQuickMacro()){
                stopQuickMacro();
            }else{
                _isQuickMacroDown = true;                
            }
            return 0;
        }

    }else{  // up 

        if(xKeyidx ==  KEY_BEYOND_FN){  // beyond_fn             
             return 0;
        }else if(xKeyidx == EXTRA_FN){
            _isExtraFNDown = false;
        }else if(xKeyidx == KEY_QUICK_MACRO){
            _isQuickMacroDown = false;
            return 0;
        }
    }

    return 1;
}
