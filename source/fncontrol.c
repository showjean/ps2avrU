
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

/* ------------------------------------------------------------------------- */
/* -----------------------------    variable  global ----------------------------- */
/* ------------------------------------------------------------------------- */

// for KEY_BEYOND_FN;
static bool _isBeyondFN = false;	 //KEY_BEYOND_FN state
static bool _isExtraFNDown = false;
// static bool _isLockKey = false;
// static bool _isLockWin = false;

bool isBeyondFN(void){
	return _isBeyondFN;
}

// bool isKeyEnabled(uint8_t xKeyidx, bool xIsDown){

// 	if(xIsDown) {
// 		if(xKeyidx == KEY_LOCK_KEY){
// 			_isLockKey ^= true;
// 		}else if(xKeyidx == KEY_LOCK_WIN){
// 			_isLockWin ^= true;
// 		}
// 	}

// 	if(_isLockKey == true){
// 		return false;
// 	}else if(_isLockWin == true && (xKeyidx == KEY_LGUI || xKeyidx == KEY_RGUI)){
// 		return false;
// 	}
// 	return true;

// }

// 키를 누르거나 땔때 FN 및 LED등 을 컨트롤한다.
bool applyFN(uint8_t xKeyidx, uint8_t col, uint8_t row, bool xIsDown) {

    // DEBUG_PRINT(("applyFN  : %d xIsDown : %d\n", xKeyidx, xIsDown));

    if(xKeyidx == KEY_FN || xKeyidx == KEY_FN2 || xKeyidx == KEY_FN3) return false; 

    setDualAction(xKeyidx, xIsDown);
    // 듀얼액션 키의 캔슬 시 사용되는 기본 키를 가져온다.
	xKeyidx = getDualActionDefaultKeyIndexWhenIsCancel(xKeyidx);

	applyKeyDownForFullLED(xKeyidx, col, row, xIsDown);
    
	if(xIsDown) {

		if((xKeyidx ==  KEY_BEYOND_FN) || (_isExtraFNDown && xKeyidx == BEYOND_FN_CANCEL_KEY)){	// beyond_fn을 활성화;
			 if( xKeyidx == BEYOND_FN_CANCEL_KEY ) {	// 취소만 가능한 키 
				_isBeyondFN = false;
			 }else{
				_isBeyondFN ^= true;
			 }
			 if(_isBeyondFN == false){
				setLEDIndicate();
			 }
			 return false;
		}else if(xKeyidx == EXTRA_FN){
			_isExtraFNDown = true;
		}else if((_isExtraFNDown && xKeyidx == LED_KEY) || xKeyidx == KEY_LED){
			
			changeFullLedState();
			return false;
		}else if(xKeyidx == KEY_LED_UP){
			increaseLedBrightness();
			return false;
		}else if(xKeyidx == KEY_LED_DOWN){
			reduceLedBrightness();
			return false;
		}
	}else{	// up 

		if(xKeyidx ==  KEY_BEYOND_FN){	// beyond_fn 			 
			 return false;
		}else if(xKeyidx == EXTRA_FN){
			_isExtraFNDown = false;
		}else if(xKeyidx == KEY_P2U){
			showP2UMenu();
			return false;
		}
	}

	return true;
}
