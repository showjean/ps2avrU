
#include "timer.h"
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

/* ------------------------------------------------------------------------- */
/* -----------------------------    variable  global ----------------------------- */
/* ------------------------------------------------------------------------- */

// for KEY_BEYOND_FN;
static bool _isBeyondFN = false;	 //KEY_BEYOND_FN state
static bool _isExtraFNDown = false;
static uint8_t _isLockKey = LOCK_NOT_SET;
static uint8_t _isLockWin = LOCK_NOT_SET;

bool isBeyondFN(void){
	return _isBeyondFN;
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
	if(xIsDown) {	// !xIsDown은 오작동한다.
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
    if(xKeyidx == KEY_FN || xKeyidx == KEY_FN2 || xKeyidx == KEY_FN3) return false; 

	applyKeyDownForFullLED(xKeyidx, xCol, xRow, xIsDown);
    setDualAction(xKeyidx, xIsDown);	

    // 듀얼액션 키의 캔슬 시 사용되는 기본 키를 가져온다.
	xKeyidx = getDualActionDefaultKeyIndexWhenIsCancel(xKeyidx);

	if(xIsDown) {

		if((xKeyidx ==  KEY_BEYOND_FN) || (_isExtraFNDown && xKeyidx == BEYOND_FN_CANCEL_KEY)){	// beyond_fn을 활성화;
			 if( xKeyidx == BEYOND_FN_CANCEL_KEY ) {	// 취소만 가능한 키 
				_isBeyondFN = false;
			 }else{
				_isBeyondFN ^= true;
			 }
			 if(_isBeyondFN == false){
				blinkOnce(100);
			 }else{
				blinkOnce(100);
				_delay_ms(80);
				blinkOnce(70);
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
	}else{	// up 

		if(xKeyidx ==  KEY_BEYOND_FN){	// beyond_fn 			 
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
