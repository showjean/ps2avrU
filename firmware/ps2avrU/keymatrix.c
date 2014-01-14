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
// #include "udelay.h"

#include "keymap.h"
#include "keymatrix.h"
#include "keymapper.h"
#include "fncontrol.h"


/* ----------------------- hardware I/O abstraction ------------------------ */
#define PORTCOLUMNS PORTB  ///< port on which we read the state of the columns
#define PINCOLUMNS  PINB   ///< port on which we read the state of the columns
#define DDRCOLUMNS  DDRB   ///< port on which we read the state of the columns
#define PORTROWS1   PORTA  ///< first port connected to the matrix rows
#define PINROWS1    PINA   ///< first port connected to the matrix rows
#define DDRROWS1    DDRA   ///< first port connected to the matrix rows
#define PORTROWS2   PORTC  ///< second port connected to the matrix rows
#define PINROWS2    PINC   ///< second port connected to the matrix rows
#define DDRROWS2    DDRC   ///< second port connected to the matrix rows


/* ------------------------------------------------------------------------- */
/* -----------------------------    variable  global ----------------------------- */
/* ------------------------------------------------------------------------- */

// 17*8 bit matrix
uint8_t prevMatrix[ROWS];
uint8_t currentMatrix[ROWS];  ///< contains current state of the keyboard

#ifdef GHOST_KEY_PREVENTION
	uint8_t *ghostFilterMatrixPointer;
	uint8_t findGhostKey(void);
#endif

uint8_t _currentLayer = 0;


/* ------------------------------------------------------------------------- */
static uint8_t debounceMAX = 5;
static uint8_t debounce = 10;	// debounceMAX 보다 크게 설정하여 플러깅시 all release가 작동되는 것을 방지;
static uint8_t _isAllKeyRelease = 0;



/* ------------------------------------------------------------------------- */
/* -----------------------------    Function  global ----------------------------- */
/* ------------------------------------------------------------------------- */
void initMatrix(void){
	// initialize matrix ports - cols, rows
	// PB0-PB7 : col0 .. col7
	// PA0-PA7 : row0 .. row7
	// PC7-PC0 : row8 .. row15
	
	// PD0 : NUM
    // PD1 : CAPS
    // PD2 : D+ / Clock
    // PD3 : D- / Data
    // PD4 : FULL LED
    // PD5 : 3.6V switch TR
	// PD6 : SCRL
    // PD7 : row17
	

	// signal direction : col -> row

	DDRCOLUMNS 	= 0xFF;	// all outputs for cols
	PORTCOLUMNS	= 0xFF;	// high
	DDRROWS1	= 0x00;	// all inputs for rows
	DDRROWS2	= 0x00;
	PORTROWS1	= 0xFF;	// all rows pull-up.
	PORTROWS2	= 0xFF;	
  
	DDRD        &= ~(1<<PIND7); // input row 17
	PORTD 		|= (1<<PIND7);// pull up row 17

	clearMatrix();
}
void clearMatrix(void){
	uint8_t row;
	for(row=0;row<ROWS;++row) {
		prevMatrix[row] = 0;
		currentMatrix[row] = 0;
	}

	debounceMAX = 10;

#ifdef GHOST_KEY_PREVENTION	
	ghostFilterMatrixPointer = currentMatrix;
#endif

}

uint8_t isAllKeyRelease(void)
{
	uint8_t row;
	_isAllKeyRelease = 1;
	for(row=0;row<ROWS;row++) {
		if(currentMatrix[row] > 0){
			_isAllKeyRelease = 0;
			break;
		}
	}
	return _isAllKeyRelease;
}


// function that determine keymap
// 0 = normal, 1 = fn, 2 = beyond_fn
uint8_t getLayer(void) {
	uint8_t col, row, keyidx, cur;
	static uint8_t isLazyLayer = 0;

	// fn이 가장 우선, 다음 fn2

	for(col=0;col<COLUMNS;col++)
		for(row=0;row<ROWS;row++){		
			keyidx = getCurrentKeyindex(_currentLayer, row, col);

		    keyidx = getDualActionMaskDown(keyidx); 

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
				
				// _delay_us_m(1);
				_delay_us(5);


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
	
	if(isBeyondFN() == 1) {
		_currentLayer = 2;
		return 2;
	}
		
	_currentLayer = 0;
	return 0;
}

#ifdef GHOST_KEY_PREVENTION
uint8_t findGhostKey(void){
	uint8_t col, row, gCol, gRow;
	for(gRow=0;gRow<ROWS;gRow++)
	{		
		if(currentMatrix[gRow] == 0) continue;

		for (gCol = 0; gCol < COLUMNS; ++gCol)
		{			
			if(currentMatrix[gRow] & BV(gCol)){
				// matrix가 set 되었다.
				// 현재 row보다 낮은 위치에 또 다른 set이 있는지 확인;
				for(col = gCol+1; col < COLUMNS; ++col)
				{
					if(currentMatrix[gRow] & BV(col)){
						//또다른 col에 set이 있다.
						// 그렇다면, 같은 col의 하위 row에 또 다른 set이 있는지 확인;
						for(row = gRow+1; row < ROWS; ++row)
						{						
							if((currentMatrix[row] & (1<<col))> 0) {
								// DEBUG_PRINT(("GHOST_KEY_PREVENTION \n"));
								// 고스트 키 확인;
								return 1;
							}
						}
					}
				}
			}
			
		}
	}
	return 0;
}
#endif

uint8_t getLiveMatrix(void){
	uint8_t col, row;
	uint8_t prev, cur;
	
	uint8_t isModified = 0;

	for(col=0;col<COLUMNS;col++)
	{
		// Col -> set only one port as input and all others as output low
		DDRCOLUMNS  = BV(col);
		PORTCOLUMNS = ~BV(col);

		// _delay_us_m(1);
		_delay_us(5);
		
		// scan each rows
		for(row=0;row<ROWS;row++)
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
	uint8_t hasGhostKey = 0;
	hasGhostKey = findGhostKey();
	
	if(hasGhostKey > 0){
		ghostFilterMatrixPointer = prevMatrix;
	}else{
		ghostFilterMatrixPointer = currentMatrix;
	}

	DEBUG_PRINT(("GHOST_KEY_PREVENTION  p : %d, prevMatrix : %d, currentMatrix : %d \n", ghostFilterMatrixPointer, prevMatrix, currentMatrix));
#endif

	return 1;
}

uint8_t *getCurrentMatrix(void){

#ifdef GHOST_KEY_PREVENTION
	return ghostFilterMatrixPointer;
#else
	return currentMatrix;
#endif

}

//curmatrix
uint8_t setCurrentMatrix(void){	
			

	uint8_t gClearMatrix = getLiveMatrix();
	
	// if(isReadyKeyMappingOnBoot()) return 0;

	return gClearMatrix;
}



#endif
