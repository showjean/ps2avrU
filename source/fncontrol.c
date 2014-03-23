
#include "timerinclude.h"
#include "print.h"

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
#include "ps2avru_util.h"

#define CMD_TOGGLE_BEYOND_FN_LED 1
#define CMD_EXIT_BEYOND_FN_LED 3
#define CMD_BACK_BEYOND_FN_LED 6

const char str_select_beyond_fn[] PROGMEM = "enable fn2 led";

static bool _isBeyondFnLedEnabled;
// for KEY_BEYOND_FN;
static bool _isBeyondFN = false;     //KEY_BEYOND_FN state
static bool _isExtraFNDown = false;
static uint8_t _isLockKey = LOCK_NOT_SET;
static uint8_t _isLockWin = LOCK_NOT_SET;

void initBeyondFn(void){
    _isBeyondFnLedEnabled = getToggleOption(EEPROM_ENABLED_OPTION, TOGGLE_BEYOND_FN_LED);
}

bool isBeyondFN(void){
    return _isBeyondFN;
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
}
static void __setKeyEnabled(uint8_t *lock){
    if(*lock == LOCK_IS_SET){
        *lock = LOCK_WILL_NOT_SET;
    }else{
        *lock = LOCK_WILL_SET;              
    }
}
void setKeyEnabled(uint8_t xKeyidx, bool xIsDown){
    /*
        현재 ps2에서 눌려진 키가 복귀 되지 않는 상태로 lock이 걸린다.
    */
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

        if((xKeyidx ==  KEY_BEYOND_FN) || (_isExtraFNDown && xKeyidx == BEYOND_FN_CANCEL_KEY)){ // beyond_fn을 활성화;
             if( xKeyidx == BEYOND_FN_CANCEL_KEY ) {    // 취소만 가능한 키 
                _isBeyondFN = false;
             }else{
                _isBeyondFN ^= true;
             }

#ifndef DISABLE_FN2_TOGGLE_LED_BLINK 
             if(isBeyondFnLedEnabled() == false){
                 if(_isBeyondFN == false){                
                    blinkOnce(100);
                 }else{
                    blinkOnce(100);
                    _delay_ms(80);
                    blinkOnce(70);
                 }
             }
#endif
             delegateSetBeyondFnLed(_isBeyondFN);

             if(isBeyondFnLedEnabled()){    
                setLed(LED_STATE_NUM, isBeyondFN());
             }

             return 0;
        }else if(xKeyidx == EXTRA_FN){
            _isExtraFNDown = 1;
        }else if((_isExtraFNDown && xKeyidx == LED_KEY) || xKeyidx == KEY_LED){
            
            changeFullLedState();
            return 0;
        }else if(xKeyidx == KEY_LED_UP){
            increaseLedBrightness();
            return 0;
        }else if(xKeyidx == KEY_LED_DOWN){
            reduceLedBrightness();
            return 0;
        }
    }else{  // up 

        if(xKeyidx ==  KEY_BEYOND_FN){  // beyond_fn             
             return 0;
        }else if(xKeyidx == EXTRA_FN){
            _isExtraFNDown = 0;
        }else if(xKeyidx == KEY_P2U){
            showP2UMenu();
            return 0;
        }
    }

    return 1;
}
