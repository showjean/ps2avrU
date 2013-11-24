#ifndef KEYMATRIX_C
#define KEYMATRIX_C

#include "timer.h"
#include "print.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include <util/delay.h>
#include <string.h>
#include "udelay.h"

#include "keymap.h"
#include "keymatrix.h"
#include "hardwareinfo.h"
#include "ledrender.h"
#include "keymapper.h"


/* ------------------------------------------------------------------------- */
/* -----------------------------    variable  global ----------------------------- */
/* ------------------------------------------------------------------------- */

// for KEY_BEYOND_FN;
uint8_t isBeyondFN = 0;	 //KEY_BEYOND_FN state
// 17*8 bit matrix
uint8_t prevMatrix[17];
uint8_t currentMatrix[17];  ///< contains current state of the keyboard

#ifdef GHOST_KEY_PREVENTION
	uint8_t ghostFilterMatrix[17];
#endif

uint8_t _currentLayer = 0;


/* ------------------------------------------------------------------------- */
static uint8_t _isExtraFNDown = 0;
static uint8_t debounceMAX = 5;
static uint8_t debounce = 10;	// debounceMAX 보다 크게 설정하여 플러깅시 all release가 작동되는 것을 방지;
static uint8_t _isAllKeyRelease = 0;



/* Originally used as a mask for the modifier bits, but now also
   used for other x -> 2^x conversions (lookup table). */
const unsigned short int modmask[16] = {
	0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
	0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000
};

/* ------------------------------------------------------------------------- */
/* -----------------------------    Function  global ----------------------------- */
/* ------------------------------------------------------------------------- */
void clearMatrix(void){
	uint8_t row;
	for(row=0;row<17;++row) {
		prevMatrix[row] = 0;
		currentMatrix[row] = 0;
	}

	debounceMAX = 10;
}

uint8_t isAllKeyRelease(void)
{
	uint8_t cur, row;
	_isAllKeyRelease = 1;
	for(row=0;row<17;row++) {
		if(currentMatrix[row] > 0){
			_isAllKeyRelease = 0;
			break;
		}
	}
	return _isAllKeyRelease;
}

// 키를 누르거나 땔때 FN 및 LED등 을 컨트롤한다.
uint8_t applyFN(uint8_t keyidx, uint8_t isDown) {

	if(keyidx == KEY_FN) return 0;
    setDualAction(keyidx, isDown);	

	if(isDown) {
		applyKeyDownForFullLED();

		if((keyidx ==  KEY_BEYOND_FN) || (_isExtraFNDown && keyidx == BEYOND_FN_CANCEL_KEY)){	// beyond_fn을 활성화;
			 if( keyidx == BEYOND_FN_CANCEL_KEY ) {	// 취소만 가능한 키 
				isBeyondFN = 0;
			 }else{
				isBeyondFN ^= 1;
			 }
			 if(isBeyondFN == 0){
				if(getLEDState() & LED_STATE_NUM) { 
					turnOnLED(LEDNUM); 
				}else{ 
					turnOffLED(LEDNUM); 
				}
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
		}/*else if(keyidx >= KEY_MAX){
			return 0;
		}*/
	}else{	// up 

		if(keyidx ==  KEY_BEYOND_FN){	// beyond_fn 			 
			 return 0;
		}else if(keyidx == EXTRA_FN){
			_isExtraFNDown = 0;
		}/*else if(keyidx >= KEY_MAX){
			return 0;
		}*/
	}

	return 1;
}

// function that determine keymap
// 0 = normal, 1 = fn, 2 = beyond_fn
uint8_t getLayer(void) {
	uint8_t col, row, keyidx, cur;
	static uint8_t isLazyLayer = 0;

	// fn이 가장 우선, 다음 fn2

	for(col=0;col<8;col++)
		for(row=0;row<17;row++){			
			//keyidx = pgm_read_byte(&keymap_code[_currentLayer][row][col]);
			keyidx = getCurrentKeycode(_currentLayer, row, col);

			if (keyidx > KEY_dualAction && keyidx < KEY_dualAction_end) { 
		        keyidx = dualActionMaskDown[keyidx - (KEY_dualAction + 1)];
		    }

			if(keyidx == KEY_LAZY_FN || keyidx == KEY_LAZY_FN2 
				|| keyidx == KEY_FN || keyidx == KEY_FN2 
				|| (keyidx == KEY_NOR && _currentLayer == 2)) {
				cur = 0;
				DDRCOLUMNS  = BV(col);		// 해당 col을 출력으로 설정, 나머지 입력
				PORTCOLUMNS = ~BV(col);	// 해당 col output low, 나머지 컬럼을 풀업 저항
				/*
				DDR을 1로 설정하면 출력, 0이면 입력
				입력중, PORT가 1이면 풀업(풀업 상태는  high 상태);

				출력 상태의 PORT가 0이면 output low(0v);

				스위치를 on하면 0, off하면 1이 PIN에 저장;
				row는 내부 풀업 저항 상태 이기 때문에 1값이 기본값
				*/
				
				_delay_us_m(1);

				if(row<8)	{				// for 0..7, PORTA 0 -> 7
					cur = (~PINROWS1)&BV(row);
				}
				else if(row>=8 && row<16) {	// for 8..15, PORTC 7 -> 0
					cur = (~PINROWS2)&BV(15-row);
				}
				else {						// for 16, PORTD 7 
					cur = (~PIND)&BV(23-row);
				}

				if(cur){
					// DEBUG_PRINT(("col= %d, row= %d keymap\n", col, row));
					if(keyidx == KEY_FN){
						//_currentLayer = 1;	// fn 레이어에는 FN키를 검색하지 않는다. 
						return 1;
					}else if(keyidx == KEY_FN2){
						return 2;	
					}else if(keyidx == KEY_LAZY_FN){
						if(_isAllKeyRelease){
							isLazyLayer = 1;
							return 1;	
						}
					}else if(keyidx == KEY_LAZY_FN2){
						if(_isAllKeyRelease){
							isLazyLayer = 2;
							return 2;			
						}		
					}else if(keyidx == KEY_NOR){
						// _currentLayer은 2를 유지하면서 스캔할 레이어만 0으로 반환;
						return 0;
					}
				}
			}
		}

	if(isAllKeyRelease()){
		isLazyLayer = 0;
	}

	if(isLazyLayer > 0) return isLazyLayer;
	
	if(isBeyondFN == 1) {
		_currentLayer = 2;
		return 2;
	}
		
	_currentLayer = 0;
	return 0;
}

uint8_t getLiveMatrix(void){
	uint8_t col, row;
	uint8_t prev, cur;
	
	uint8_t isModified = 0;

	for(col=0;col<8;col++)
	{
		// Col -> set only one port as input and all others as output low
		DDRCOLUMNS  = BV(col);
		PORTCOLUMNS = ~BV(col);

		_delay_us_m(1);
		
		// scan each rows
		for(row=0;row<17;row++)
		{
			if(row<8)	{				// for 0..7, PORTA 0 -> 7
				cur = (~PINROWS1) & BV(row);
			}
			else if(row>=8 && row<16) {	// for 8..15, PORTC 7 -> 0
				cur = (~PINROWS2) & BV(15-row);
			}
			else {						// for 16, PORTD 7
				cur = (~PIND) & BV(23-row);
			}

			prev = currentMatrix[row] & BV(col);

			if(!(prev && cur) && !(!prev && !cur)) {
				if(cur)
					currentMatrix[row] |= BV(col);
				else
					currentMatrix[row] &=~ BV(col);

				isModified = 1;
			}
		}

	}

	if(isModified){
		debounce=0;
	}else if(debounce<100){	// to prevent going over limit of int
		// 키 입력에 변화가 없다면 99에서 멈춰서 0을 계속 반환하게 된다. 때문에, 키 변화없을때는 키코드 갱신없음;
		debounce++;
	}

	if(debounce != debounceMAX){
		return 0;
	}

#ifdef GHOST_KEY_PREVENTION
	// ghost-key prevention
	// col에 2개 이상의 입력이 있을 경우, 입력이 있는 row에는 더이상 입력을 허용하지 않는다.
	uint8_t ghost = 0;
	uint8_t needDetection = 0;
	uint8_t i;
	for(row=0;row<17;row++)
	{
		if(needDetection > 1){
			ghostFilterMatrix[row] &= currentMatrix[row] ^ ghost;	// r1:g1 = 0, r1:g0 = 1, r0:g1 = 0, r0:g0 = 0; 다른 것만 1로 만들 후 & 연산;
		}else{
			ghostFilterMatrix[row] = currentMatrix[row];
		}

		ghost |= ghostFilterMatrix[row];
		// DEBUG_PRINT(("currentMatrix[%d]= %02X, ghost=%02X, needDetection=%d  \n", row, ghostFilterMatrix[row], ghost, needDetection));
		// 1비트(컬럼)만 입력이 있을 경우 외에 2비트 이상 입력이 있다면 표시;
		needDetection = 0;

		for (i = 0; i < 8; ++i)
		{
			if(ghost & BV(i)){
				++needDetection;
			}
		}
	}
#endif

	return 1;
}

uint8_t *getCurrentMatrix(void){

#ifdef GHOST_KEY_PREVENTION
	return ghostFilterMatrix;
#else
	return currentMatrix;
#endif

}

//curmatrix
uint8_t setCurrentMatrix(void){	
	
	startKeyMapping();		

	uint8_t gClearMatrix = getLiveMatrix();

	
	enterFrameForMapper();	
	
	if(isReadyKeyMappingOnBoot()) return 0;

	// DEBUG_PRINT(("setCurrentMatrix return 1 \n"));

	return gClearMatrix;
}



#endif
