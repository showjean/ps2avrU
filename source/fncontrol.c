
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
#include "dualaction.h"
#include "keymatrix.h"
//#include "keydownbuffer.h"
#include "ps2avru_util.h"
#include "oddebug.h"

static uint8_t _beyondFnLed;	// 0: off, 1:NL, 2:SL
// for KEY_BEYOND_FN;
static uint8_t _beyondFnIndex = 0;     //KEY_BEYOND_FN state
static uint8_t _prevBeyondFnIndex = 0;
static bool _isDownExtraFN = false;
static bool _isDownQuickMacro = false;
static bool _isReadyQuickMacro = false;
static uint8_t _quickMacroIndex = 255;
static uint8_t _isLockKey = LOCK_NOT_SET;
static uint8_t _isLockWin = LOCK_NOT_SET;

static bool _ledOff = false;

bool isDownExtraFn(void)
{
    return _isDownExtraFN;
}

uint8_t getBeyondFN(void){
    return _beyondFnIndex;
}
uint8_t getBeyondFNPrev(void){
    return _prevBeyondFnIndex;
}

uint8_t getBeyondFnLed(void){
    return _beyondFnLed;
}


void initBeyondFn(void){

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
    if(isReleaseAll() && _isReadyQuickMacro){
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

    // dual action key를 기본키 (caps lock 등)으로 반환;
    xKeyidx = getDualActionDefaultKey(xKeyidx);

    if(isFnKey(xKeyidx)) return false;

    if(xIsDown) {

        if((xKeyidx ==  KEY_BEYOND_FN || xKeyidx == KEY_BEYOND_FN3)
        		|| (_isDownExtraFN && xKeyidx == BEYOND_FN_CANCEL_KEY)){ // beyond_fn을 활성화;
            _prevBeyondFnIndex = _beyondFnIndex;
             if( xKeyidx == BEYOND_FN_CANCEL_KEY ) {    // 취소만 가능한 키 
                _beyondFnIndex = LAYER_NORMAL;
             }else{
            	if(_beyondFnIndex == LAYER_NORMAL){
            		if(xKeyidx ==  KEY_BEYOND_FN){
            			_beyondFnIndex = LAYER_FN2;
            		}else{
            			_beyondFnIndex = LAYER_FN3;
            		}
            	}else{
            		if(xKeyidx ==  KEY_BEYOND_FN && !(_beyondFnIndex ==  LAYER_FN2)){
            			_beyondFnIndex = LAYER_FN2;
					}else if(xKeyidx ==  KEY_BEYOND_FN3 && !(_beyondFnIndex ==  LAYER_FN3)){
            			_beyondFnIndex = LAYER_FN3;
					}else{
        				_beyondFnIndex = LAYER_NORMAL;
					}
            	}
             }

             setLEDIndicate();
             _prevBeyondFnIndex = _beyondFnIndex;   // 더이상 변화가 없도록;


             if( xKeyidx == BEYOND_FN_CANCEL_KEY ) {    // 키가 작동하도록 1 리턴;
            	 return 1;
			  }
             return 0;
        }else if(_isDownQuickMacro && isEepromMacroKey(xKeyidx)){
            _quickMacroIndex = xKeyidx - KEY_MAC1; 
            _isReadyQuickMacro = true;
            return 0;
        }else if(xKeyidx == EXTRA_FN){
            _isDownExtraFN = true;
        }else if(xKeyidx == KEY_LED_ON_OFF){
        	_ledOff ^= true;
        	if(_ledOff == false){
        		turnOnLedAll();
        	}else{
        		turnOffLedAll();
        	}
        	return 0;
        }else if(xKeyidx == KEY_QUICK_MACRO){
            if(isQuickMacro()){
                stopQuickMacro();
            }else{
                _isDownQuickMacro = true;
            }
            return 0;
        }else{
        	return delegateFnControl(xKeyidx, _isDownExtraFN);
        }

    }else{  // up 

        if(xKeyidx ==  KEY_BEYOND_FN){  // beyond_fn             
             return 0;
        }else if(xKeyidx == EXTRA_FN){
            _isDownExtraFN = false;
        }else if(xKeyidx == KEY_QUICK_MACRO){
            _isDownQuickMacro = false;
            return 0;
        }
    }

    return 1;
}
