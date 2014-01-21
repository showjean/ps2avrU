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

#include "keymatrix.h"
#include "keymap.h"
#include "fncontrol.h"
#include "lazyfn.h"
#include "keymapper.h"
#include "keyindex.h"
#include "dualaction.h"

// 17*8 bit matrix
static uint8_t prevMatrix[ROWS];
static uint8_t currentMatrix[ROWS];  ///< contains current state of the keyboard

#ifdef GHOST_KEY_PREVENTION
	uint8_t *ghostFilterMatrixPointer;
	uint8_t findGhostKey(void);
#endif

uint8_t _currentLayer = 0;


/* ------------------------------------------------------------------------- */
static uint8_t debounceMAX = 5;
static uint8_t debounce = 10;	// debounceMAX 보다 크게 설정하여 플러깅시 all release가 작동되는 것을 방지;
static bool _isAllKeyRelease = true;



/* ------------------------------------------------------------------------- */
/* -----------------------------    Function  global ----------------------------- */
/* ------------------------------------------------------------------------- */
void initMatrix(void){
	
	initMatrixDevice();

	clearMatrix();
}

void clearMatrix(void){
	uint8_t row;
	for(row=0;row<ROWS;++row) {
		prevMatrix[row] = 0;
		currentMatrix[row] = 0;
	}

	// debounceMAX = 5;
	// debounce = 10;

#ifdef GHOST_KEY_PREVENTION	
	ghostFilterMatrixPointer = currentMatrix;
#endif

}

void clearPrevMatrix(void){
	uint8_t row;
	for(row=0;row<ROWS;++row) {
		prevMatrix[row] = 0;
	}
}

bool isAllKeyRelease(void)
{
	uint8_t row;
	_isAllKeyRelease = true;
	for(row=0;row<ROWS;row++) {
		// currentMatrix로 비교하면 ps/2연결시 마지막 키의 up 판단 전에 매트릭스상 모든 키가 릴리즈 상태여서 마지막 키가 리포트 되지 않는다.
		if(prevMatrix[row] > 0){
			_isAllKeyRelease = false;
			break;
		}
	}
	return _isAllKeyRelease;
}


// function that determine keymap
// 0 = normal, 1 = fn, 2 = beyond_fn
uint8_t getLayer(void) {
	uint8_t col, row, keyidx, cur;
	static uint8_t _currentLazyLayer = 0;

	// fn이 가장 우선, 다음 fn2

	for(col=0;col<COLUMNS;col++)
		for(row=0;row<ROWS;row++){		
			keyidx = getCurrentKeyindex(_currentLayer, row, col);

		    keyidx = getDualActionMaskDown(keyidx); 

			if(keyidx == KEY_FN || keyidx == KEY_FN2 || keyidx == KEY_FN3 
				|| (keyidx == KEY_NOR && _currentLayer == 2)) {
				
				cur = 0;
				// Col -> set only one port as input and all others as output low
				setCellStatus(col);

				cur = getCellStatus(row);

				if(cur){
					// DEBUG_PRINT(("col= %d, row= %d keymap\n", col, row));
					if(keyidx == KEY_FN){
						//_currentLayer = 1;	// fn 레이어에는 FN키를 검색하지 않는다. 
						if(_isAllKeyRelease && isLazyFn()){
							_currentLazyLayer = 1;
						}
						return 1;
					}else if(keyidx == KEY_FN2){
						if(_isAllKeyRelease && isLazyFn()){
							_currentLazyLayer = 2;
						}
						return 2;	
					}else if(keyidx == KEY_FN3){
						if(_isAllKeyRelease && isLazyFn()){
							_currentLazyLayer = 3;
						}
						return 3;					
					}else if(keyidx == KEY_NOR){
						// _currentLayer은 2를 유지하면서 스캔할 레이어만 0으로 반환;
						return 0;
					}
				}
			}
		}

	if(isAllKeyRelease()){
		_currentLazyLayer = 0;
	}

	if(_currentLazyLayer > 0) return _currentLazyLayer;
	
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
		setCellStatus(col);
		
		// scan each rows
		for(row=0;row<ROWS;row++)
		{
			cur = getCellStatus(row);

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

void setPrevMatrix(void){
	uint8_t row;
	uint8_t *gMatrix = getCurrentMatrix();
	for(row=0;row<ROWS;++row)
		prevMatrix[row] = gMatrix[row];
}

uint8_t *getPrevMatrix(void){
	return prevMatrix;
}

//curmatrix
uint8_t setCurrentMatrix(void){	
			

	uint8_t gClearMatrix = getLiveMatrix();

	return gClearMatrix;
}



#endif
