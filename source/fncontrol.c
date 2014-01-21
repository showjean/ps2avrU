
#include "timer.h"
#include "print.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdbool.h>

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

bool isBeyondFN(void){
	return _isBeyondFN;
}

// 키를 누르거나 땔때 FN 및 LED등 을 컨트롤한다.
bool applyFN(uint8_t keyidx, uint8_t col, uint8_t row, bool isDown) {

    // DEBUG_PRINT(("applyFN  : %d isDown : %d\n", keyidx, isDown));
    if(keyidx == KEY_NONE) return 0; 

	applyKeyDownForFullLED(keyidx, col, row, isDown);
    setDualAction(keyidx, isDown);	
	// if(keyidx == KEY_FN) return 0;

    // 듀얼액션 키의 캔슬 시 사용되는 기본 키를 가져온다.
	keyidx = getDualActionDefaultKeyIndexWhenIsCancel(keyidx);

	if(isDown) {

		if((keyidx ==  KEY_BEYOND_FN) || (_isExtraFNDown && keyidx == BEYOND_FN_CANCEL_KEY)){	// beyond_fn을 활성화;
			 if( keyidx == BEYOND_FN_CANCEL_KEY ) {	// 취소만 가능한 키 
				_isBeyondFN = 0;
			 }else{
				_isBeyondFN ^= 1;
			 }
			 if(_isBeyondFN == 0){
				setLEDIndicate();
			 }
			 return 0;
		}else if(keyidx == EXTRA_FN){
			_isExtraFNDown = 1;
		}else if((_isExtraFNDown && keyidx == LED_KEY) || keyidx == KEY_LED){
			
			changeFullLedState();
			return 0;
		}else if(keyidx == KEY_LED_UP){
			increaseLedBrightness();
			return 0;
		}else if(keyidx == KEY_LED_DOWN){
			reduceLedBrightness();
			return 0;
		}
	}else{	// up 

		if(keyidx ==  KEY_BEYOND_FN){	// beyond_fn 			 
			 return 0;
		}else if(keyidx == EXTRA_FN){
			_isExtraFNDown = 0;
		}else if(keyidx == KEY_P2U){
			showP2UMenu();
			return 0;
		}
	}

	return 1;
}
