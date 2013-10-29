#ifndef KEYMATRIX_C
#define KEYMATRIX_C

#include "timer.h"
#include "keymap.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include <util/delay.h>
#include <string.h>
#include "udelay.h"

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
uint8_t currentKeymap = 0;


/* ------------------------------------------------------------------------- */
static uint8_t _isExtraFNDown = 0;
static uint8_t debounceMAX = 5;
static uint8_t debounce = 10;	// debounceMAX 보다 크게 설정하여 플러깅시 all release가 작동되는 것을 방지;



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
	cur = 1;
	for(row=0;row<17;row++) {
		if(currentMatrix[row] > 0){
			cur = 0;
			break;
		}
	}
	return cur;
}

// 키를 누르거나 땔때 FN 및 LED등 을 컨트롤한다.
uint8_t applyFN(uint8_t keyidx, uint8_t isDown) {

	if(keyidx == KEY_FN) return 0;

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
		}else if(keyidx >= KEY_MAX){
			return 0;
		}
	}else{	// isDown : false
		if(keyidx ==  KEY_BEYOND_FN){	// beyond_fn 			 
			 return 0;
		}else if(keyidx == EXTRA_FN){
			_isExtraFNDown = 0;
		}else if(keyidx >= KEY_MAX){
			return 0;
		}
	}

	return 1;
}

// function that determine keymap
// 0 = normal, 1 = fn, 2 = beyond_fn
uint8_t getLayer(void) {
	uint8_t col, row, keyidx, cur;

	// fn이 가장 우선, 다음 fn2

	for(col=0;col<8;col++)
		for(row=0;row<17;row++){			
			//keyidx = pgm_read_byte(&keymap_code[currentKeymap][row][col]);
			keyidx = getCurrentKeycode(currentKeymap, row, col);

			if(keyidx == KEY_FN || (keyidx == KEY_NOR && currentKeymap == 2)) {
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
						//currentKeymap = 1;	// fn 레이어에는 FN키를 검색하지 않는다. 
						return 1;
					}else if(keyidx == KEY_NOR){
						// currentKeymap은 2를 유지하면서 스캔할 레이어만 0으로 반환;
						return 0;
					}
				}
			}
		}
	
	if(isBeyondFN == 1) {
		currentKeymap = 2;
		return 2;
	}
		
	currentKeymap = 0;
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

	return 1;
}

//curmatrix
uint8_t setCurrentMatrix(void){	
	
	startKeyMapping();		

	uint8_t gClearMatrix = getLiveMatrix();

	
	enterFrameForMapper();	
	
	if(isReadyKeyMappingOnBoot()) return 0;

	/*if(debounce != debounceMAX){
		return 0;
	}*/

	// DEBUG_PRINT(("setCurrentMatrix return 1 \n"));

	return gClearMatrix;
}



#endif
